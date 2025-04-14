#include <iostream>
#include "ArtistGraph.hpp"

auto main() -> int {
  auto a = 1;
  auto b = 2;
  auto c = 3;
  auto d = 4;

  auto x = 55;
  auto y = 66;

  ArtistGraph graph{};
  graph.setFollowing(a, b);
  graph.getNumberOfChains();
  graph.setFollowing(b, c);
  graph.getNumberOfChains();//  -> 1
  graph.setFollowing(c, d);
  graph.getNumberOfChains(); // -> 1

  auto n = graph.getNumberOfChains();
  std::cout << n << "\n";

  graph.setFollowing(x, y);

  n = graph.getNumberOfChains();
  std::cout << n << "\n";
}


