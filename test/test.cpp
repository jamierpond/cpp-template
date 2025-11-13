#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_all.hpp>
#include "../scanner.hpp"
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

class TempTestDir {
public:
  TempTestDir() {
    path = fs::temp_directory_path() / ("merkle_test_" + std::to_string(std::rand()));
    fs::create_directories(path);
  }

  ~TempTestDir() {
    if (fs::exists(path)) {
      fs::remove_all(path);
    }
  }

  fs::path get() const { return path; }

  void create_file(const std::string& relative_path, const std::string& content) {
    fs::path full_path = path / relative_path;
    fs::create_directories(full_path.parent_path());
    std::ofstream file(full_path);
    file << content;
  }

private:
  fs::path path;
};

TEST_CASE("collect_files_sorted returns files in sorted order") {
  TempTestDir temp;

  temp.create_file("file_c.txt", "content c");
  temp.create_file("file_a.txt", "content a");
  temp.create_file("file_b.txt", "content b");
  temp.create_file("subdir/file_z.txt", "content z");
  temp.create_file("subdir/file_x.txt", "content x");

  auto files = collect_files_sorted(temp.get());

  REQUIRE(files.size() == 5);
  REQUIRE(files[0] == "file_a.txt");
  REQUIRE(files[1] == "file_b.txt");
  REQUIRE(files[2] == "file_c.txt");
  REQUIRE(files[3] == "subdir/file_x.txt");
  REQUIRE(files[4] == "subdir/file_z.txt");
}

TEST_CASE("build_tree_from_directory creates tree with correct number of leaves") {
  TempTestDir temp;

  temp.create_file("file1.txt", "content 1");
  temp.create_file("file2.txt", "content 2");
  temp.create_file("dir/file3.txt", "content 3");

  std::vector<std::string> paths;
  Tree tree = build_tree_from_directory(temp.get(), get_content_hash, paths);

  REQUIRE(paths.size() == 3);
  REQUIRE(tree.num_leaves() == 3);
}

TEST_CASE("detect_changed_file_paths detects added files") {
  TempTestDir temp;

  temp.create_file("file1.txt", "content 1");

  std::vector<std::string> prev_paths;
  Tree prev_tree = build_tree_from_directory(temp.get(), get_content_hash, prev_paths);

  temp.create_file("file2.txt", "content 2");

  auto result = detect_changed_file_paths(prev_tree, prev_paths, temp.get(), get_content_hash);

  REQUIRE(result.diff.added.size() == 1);
  REQUIRE(result.diff.added[0] == "file2.txt");
  REQUIRE(result.diff.deleted.empty());
  REQUIRE(result.diff.moved.empty());
  REQUIRE(result.paths.size() == 2);
}

TEST_CASE("detect_changed_file_paths detects deleted files") {
  TempTestDir temp;

  temp.create_file("file1.txt", "content 1");
  temp.create_file("file2.txt", "content 2");

  std::vector<std::string> prev_paths;
  Tree prev_tree = build_tree_from_directory(temp.get(), get_content_hash, prev_paths);

  fs::remove(temp.get() / "file2.txt");

  auto result = detect_changed_file_paths(prev_tree, prev_paths, temp.get(), get_content_hash);

  REQUIRE(result.diff.deleted.size() == 1);
  REQUIRE(result.diff.deleted[0] == "file2.txt");
  REQUIRE(result.diff.added.empty());
  REQUIRE(result.diff.moved.empty());
  REQUIRE(result.paths.size() == 1);
}

TEST_CASE("detect_changed_file_paths detects moved files") {
  TempTestDir temp;

  temp.create_file("file1.txt", "content 1");

  std::vector<std::string> prev_paths;
  Tree prev_tree = build_tree_from_directory(temp.get(), get_content_hash, prev_paths);

  fs::create_directories(temp.get() / "subdir");
  fs::rename(temp.get() / "file1.txt", temp.get() / "subdir" / "file1_moved.txt");

  auto result = detect_changed_file_paths(prev_tree, prev_paths, temp.get(), get_content_hash);

  REQUIRE(result.diff.moved.size() == 1);
  REQUIRE(result.diff.moved[0].first == "file1.txt");
  REQUIRE(result.diff.moved[0].second == "subdir/file1_moved.txt");
  REQUIRE(result.diff.added.empty());
  REQUIRE(result.diff.deleted.empty());
  REQUIRE(result.paths.size() == 1);
}

TEST_CASE("detect_changed_file_paths detects multiple operations") {
  TempTestDir temp;

  temp.create_file("file1.txt", "content 1");
  temp.create_file("file2.txt", "content 2");
  temp.create_file("file3.txt", "content 3");

  std::vector<std::string> prev_paths;
  Tree prev_tree = build_tree_from_directory(temp.get(), get_content_hash, prev_paths);

  fs::remove(temp.get() / "file2.txt");
  temp.create_file("file4.txt", "content 4");
  fs::create_directories(temp.get() / "moved");
  fs::rename(temp.get() / "file3.txt", temp.get() / "moved" / "file3.txt");

  auto result = detect_changed_file_paths(prev_tree, prev_paths, temp.get(), get_content_hash);

  REQUIRE(result.diff.added.size() == 1);
  REQUIRE(result.diff.deleted.size() == 1);
  REQUIRE(result.diff.moved.size() == 1);
  REQUIRE(result.paths.size() == 3);
}

TEST_CASE("detect_changed_file_paths handles unchanged files correctly") {
  TempTestDir temp;

  temp.create_file("file1.txt", "content 1");
  temp.create_file("file2.txt", "content 2");

  std::vector<std::string> prev_paths;
  Tree prev_tree = build_tree_from_directory(temp.get(), get_content_hash, prev_paths);

  auto result = detect_changed_file_paths(prev_tree, prev_paths, temp.get(), get_content_hash);

  REQUIRE(result.diff.added.empty());
  REQUIRE(result.diff.deleted.empty());
  REQUIRE(result.diff.moved.empty());
  REQUIRE(result.paths.size() == 2);
  REQUIRE(result.tree.root().to_string() == prev_tree.root().to_string());
}

TEST_CASE("get_content_hash generates consistent hashes") {
  TempTestDir temp;

  temp.create_file("test.txt", "test content");

  Hash hash1 = get_content_hash(temp.get() / "test.txt");
  Hash hash2 = get_content_hash(temp.get() / "test.txt");

  REQUIRE(hash1.to_string() == hash2.to_string());
  REQUIRE(hash1.to_string().length() == 64);
}

TEST_CASE("get_content_hash generates different hashes for different content") {
  TempTestDir temp;

  temp.create_file("file1.txt", "content 1");
  temp.create_file("file2.txt", "content 2");

  Hash hash1 = get_content_hash(temp.get() / "file1.txt");
  Hash hash2 = get_content_hash(temp.get() / "file2.txt");

  REQUIRE(hash1.to_string() != hash2.to_string());
}

TEST_CASE("detect_changed_file_paths detects file content changes as delete+add") {
  TempTestDir temp;

  temp.create_file("file1.txt", "original content");

  std::vector<std::string> prev_paths;
  Tree prev_tree = build_tree_from_directory(temp.get(), get_content_hash, prev_paths);

  temp.create_file("file1.txt", "modified content");

  auto result = detect_changed_file_paths(prev_tree, prev_paths, temp.get(), get_content_hash);

  REQUIRE(result.diff.deleted.size() == 1);
  REQUIRE(result.diff.added.size() == 1);
  REQUIRE(result.diff.moved.empty());
}
