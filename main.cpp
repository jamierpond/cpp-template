#include <map>
#include <vector>

class Solution {
public:
  std::string convert(std::string s, int n_rows) {
        if (n_rows <= 1) { return s; }
        auto diag_len = n_rows - 2;

        auto get_row = [&](auto pi) {
            if (pi < n_rows) { return pi; }
            auto index_in_diagonal = pi - diag_len;
            return n_rows - index_in_diagonal;
        };

        auto pattern_period = n_rows + diag_len;
        auto pi_last_char = (s.size() - 1) % pattern_period;
        auto row_last_char = get_row(pi_last_char);
        auto num_full_patterns = std::floor(float(s.size()) / float(pattern_period));
        auto row_starts = std::vector<int>(n_rows);

/*
P A H N
A P L S I I G
Y I R                                               */
        for (int r = 0; r < n_rows - 1; r++) {
            auto found_end = r >= row_last_char;
            auto extra = found_end ? 1 : 0;
            auto is_single = r == (n_rows - 1) || r == 0;
            auto len = is_single ? (num_full_patterns + 1) : ((num_full_patterns * 2) + 1);
            auto res = len + row_starts[r];
            row_starts[r + 1] += res;
        }

        auto str = std::string(s.size(), '*');
        auto map = std::map<int, int>{};

/*

P I N
A L S I G
Y A H R
P I

*/
        for (int i = 0; i < s.size(); i++) {
            auto index_in_pattern = i % pattern_period;
            auto row = get_row(index_in_pattern);
            auto row_start = row_starts[row];
            auto row_index = map[row]++;
            auto new_index = row_start + row_index;
            str[new_index] = s[i];
        }

        return str;
    }
};


