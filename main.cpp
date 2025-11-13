// Returns diff + the newly-built tree + its deterministic path list.
// Minimal, readable; assumes get_content_hash(path) exists.
#include "scanner.hpp"

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

