#include <unordered_map>
#include <set>

struct ArtistGraph {
  std::unordered_map<int, std::set<int>> data{};

  void setFollowing(int follower, int followee) {
    data[follower].insert(followee);

    // touch the followee!
    if (data[followee].empty()) {
      data[followee] = {};
    }
  }

  int getNumberOfChains() {
    auto num_graphs = 0;
    for (auto& [k, follows] : data) {
      if (follows.empty()) {
        num_graphs++;
      }
    }
    return num_graphs;
  }
};


