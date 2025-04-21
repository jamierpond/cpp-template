#include <unordered_map>
#include <set>
#include <vector>

namespace sets {

bool has_intersection(const auto& a, const auto& b) {
  for (auto& e : a) {
    if (b.contains(e)) { return true; }
  }
  return false;
}

auto set_union(const auto& a, const auto& b) {
  auto result = a; // copy
  result.insert(b.begin(), b.end());
  return result;
}

}

struct ArtistGraph {
  std::unordered_map<int, std::set<int>> data{};

  void setFollowing(int follower, int followee) {
    data[follower].insert(followee);
    if (data[followee].empty()) {
      data[followee] = {};
    }
  }

  int get_num_chains() {
    if (data.size() == 1) { return 1; }

    auto uniques = std::vector<std::set<int>>{};

    for (auto& [follower, followee] : data) {
    }

    return 0;
  }
};


