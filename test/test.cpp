#define CATCH_CONFIG_MAIN
#include <catch2/catch_all.hpp>
#include "../ArtistGraph.hpp"



TEST_CASE("Hello, World!") {
  ArtistGraph graph{};

  auto require_num_graphs  = [&] (auto num_expected) {
    auto n = graph.get_num_chains();
    REQUIRE(n == num_expected);
  };

  require_num_graphs(0);

  graph.set_following(1, 2);
  graph.get_num_chains();
  graph.set_following(2, 3);
  graph.get_num_chains();//  -> 1
  graph.set_following(3, 4);
  graph.get_num_chains(); // -> 1


  require_num_graphs(1);
  graph.set_following(55, 56);
  require_num_graphs(2);

  graph.set_following(2, 56);
  require_num_graphs(1);
}
