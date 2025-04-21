#define CATCH_CONFIG_MAIN
#include <catch2/catch_all.hpp>

#include "../convert.hpp"

/*
*
0 1 2 3
P A H N

*
4 5 6 7 8 9 10
A P L S I I G

*
11 12 34
Y  I  R
*/


/*
53 total
0 1 2 3 4 5 6 7 8
5 5 5 5 5 5 5 5 5

9 10111213141516171819202122232425
5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5

5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5
5 5 5 5 5 5 5 5 5
*/


/*
0 1 2
P I N

3 4 5 6 7
A L S I G

8 9 10 11
Y A H R

1213
P I


P     I    N
A   L S  I G
Y A   H R
P     I

*/
TEST_CASE("PLAN") {
  constexpr auto pp = std::string_view{"PAYPALISHIRING"};

  auto test = [] (auto n_rows, auto expected, auto str_len) {
    auto plan = create_zig_zag_plan(n_rows, str_len);
    REQUIRE(plan == expected);
  };

  test(3, std::vector<int>{{0, 4, 11}}, pp.size());
  test(4, std::vector<int>{{0, 3, 8, 12}}, pp.size());

  test(4, std::vector<int>{{0, 9, 26, 43}}, 53);
}
