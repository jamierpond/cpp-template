// Returns diff + the newly-built tree + its deterministic path list.
// Minimal, readable; assumes get_content_hash(path) exists.
#include <filesystem>
#include <vector>
#include <string>
#include <unordered_map>
#include <set>
#include <algorithm>
#include <functional>
#include <utility>
#include "include/merklecpp/merklecpp.h"
#include <iostream>
#include <fstream>

using Path = std::filesystem::path;
using Hash = merkle::Hash;
using Tree = merkle::Tree;

struct DiffResult {
  std::vector<std::string> added;
  std::vector<std::string> deleted;
  std::vector<std::pair<std::string, std::string>> moved;
};

struct DiffAndTree {
  DiffResult diff;
  Tree tree;
  std::vector<std::string> paths; // deterministic, relative to root
};

static std::vector<std::string> collect_files_sorted(const Path& root) {
  std::vector<std::string> r;
  for (auto const& e : std::filesystem::recursive_directory_iterator(root))
    if (e.is_regular_file())
      r.push_back(std::filesystem::relative(e.path(), root).generic_string());
  std::sort(r.begin(), r.end());
  return r;
}

static Tree build_tree_from_directory(
  const Path& root,
  const std::function<Hash(const Path&)>& get_content_hash,
  std::vector<std::string>& out_paths)
{
  out_paths = collect_files_sorted(root);
  std::vector<Hash> leaves;
  leaves.reserve(out_paths.size());
  for (auto &rel : out_paths) leaves.push_back(get_content_hash(root / rel));
  Tree t;
  t.insert(leaves);
  return t;
}

// prevTree + prev_paths (same ordering used to create prevTree)
// -> DiffAndTree { diff, newTree, newPaths }.
static DiffAndTree detect_changed_file_paths(
  const Tree& prevTree,
  const std::vector<std::string>& prev_paths,
  const Path& rootDir,
  const std::function<Hash(const Path&)>& get_content_hash)
{
  std::vector<std::string> curr_paths;
  Tree currTree = build_tree_from_directory(rootDir, get_content_hash, curr_paths);

  auto build_map = [](const Tree& tree, const std::vector<std::string>& paths) {
    std::unordered_map<std::string, std::vector<std::string>> m;
    for (size_t i = 0; i < paths.size(); ++i) m[ tree.leaf(i).to_string() ].push_back(paths[i]);
    return m;
  };

  auto prev_map = build_map(prevTree, prev_paths);
  auto curr_map = build_map(currTree, curr_paths);

  DiffResult res;
  for (auto &kv : prev_map) {
    auto it = curr_map.find(kv.first);
    if (it == curr_map.end()) {
      for (auto &p : kv.second) res.deleted.push_back(p);
      continue;
    }
    std::multiset<std::string> prevs(kv.second.begin(), kv.second.end());
    std::multiset<std::string> curs(it->second.begin(), it->second.end());
    for (auto itp = prevs.begin(); itp != prevs.end();) {
      auto itc = curs.find(*itp);
      if (itc != curs.end()) { curs.erase(itc); itp = prevs.erase(itp); }
      else ++itp;
    }
    if (!prevs.empty() && !curs.empty() && prevs.size() == curs.size()) {
      auto itp = prevs.begin(), itc = curs.begin();
      for (; itp != prevs.end(); ++itp, ++itc) res.moved.emplace_back(*itp, *itc);
    } else {
      for (auto &p : prevs) res.deleted.push_back(p);
      for (auto &p : curs) res.added.push_back(p);
    }
    curr_map.erase(it);
  }
  for (auto &kv : curr_map) for (auto &p : kv.second) res.added.push_back(p);

  return DiffAndTree{ std::move(res), std::move(currTree), std::move(curr_paths) };
}

// main.cpp
// Build initial tree, then mutate the FS and run diff.
// Requires detect_changed_file_paths / DiffAndTree from earlier.

#include <iostream>
#include <filesystem>

Hash get_content_hash(const std::filesystem::path& p) {
  // temp fake hash: just hash the file size + name base64 trimmed to 16 chars
  std::ifstream f(p, std::ios::binary | std::ios::ate);
  auto size = f.tellg();
  auto to_hash = p.filename().string() + std::to_string(size);
  // trim to 32
  if (to_hash.size() > 64) to_hash = to_hash.substr(0, 64);
  if (to_hash.size() < 64) to_hash.append(64 - to_hash.size(), '0');
  return Hash{to_hash};
}

DiffAndTree detect_changed_file_paths(
  const Tree& prevTree,
  const std::vector<std::string>& prevPaths,
  const std::filesystem::path& root,
  const std::function<Hash(const std::filesystem::path&)>& get_content_hash);

int main() {
  namespace fs = std::filesystem;
  const fs::path root = "/Users/jamiepond/Downloads/drluke";
  fs::current_path(root);

  // Setup: create temp dirs and files
  fs::create_directories("temp");

  // Create initial test file that will be moved later
  std::ofstream("temp/file1.txt") << "test content";

  // Build initial tree
  std::vector<std::string> prevPaths;
  Tree prevTree = [&] {
    std::vector<std::string> p;
    Tree t;
    p = {}; // will be populated inside detect fn, so use build helper instead
    std::vector<std::string> dummy;
    t = build_tree_from_directory(root, get_content_hash, prevPaths);
    return t;
  }();

  // Mutate FS: add new file, move one existing deeper
  fs::create_directories("temp/onedeeper/here");
  fs::rename("temp/file1.txt", "temp/onedeeper/here/file1_moved.txt");
  std::ofstream("temp/newFile.txt") << "new stuff";

  // Detect diff + get new tree
  DiffAndTree r = detect_changed_file_paths(
    prevTree,
    prevPaths,
    root,
    get_content_hash
  );

  // Print results
  std::cout << "Added:\n";
  for (auto& p : r.diff.added) std::cout << "  + " << p << "\n";
  std::cout << "Deleted:\n";
  for (auto& p : r.diff.deleted) std::cout << "  - " << p << "\n";
  std::cout << "Moved:\n";
  for (auto& [from, to] : r.diff.moved)
    std::cout << "  " << from << " -> " << to << "\n";

  std::cout << "New root hash: " << r.tree.root().to_string() << "\n";
}

