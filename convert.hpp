#include <string>
#include <cmath>
#include <vector>

constexpr auto get_row = [](auto pattern_index, auto num_rows, auto diag_len) -> int {
    if (pattern_index < num_rows) { return pattern_index; }
    auto index_in_diagonal = pattern_index - diag_len;
    return num_rows - index_in_diagonal;
};

constexpr auto diag_length = [](auto n_rows) {
    return n_rows - 2;
};

constexpr auto pattern_period = [](auto n_rows) {
    return n_rows + diag_len(n_rows);
};

constexpr auto create_zig_zag_plan(auto n_rows, auto str_len) {
    auto diag_len = diag_length(n_rows);

    auto pattern_period = n_rows + diag_len;
    auto pi_last_char = (str_len - 1) % pattern_period;
    auto row_last_char = get_row(pi_last_char, n_rows, diag_len);
    auto num_full_patterns = std::floor(float(str_len) / float(pattern_period));
    auto row_starts = std::vector<int>(n_rows);

    for (int r = 0; r < n_rows - 1; r++) {
        auto is_single = r == (n_rows - 1) || r == 0;
        auto len = is_single ? (num_full_patterns + 1) : ((num_full_patterns * 2) + 1);
        auto res = len + row_starts[r];
        row_starts[r + 1] += res;
    }
    return std::move(row_starts);
}

constexpr std::string convert(std::string s, int n_rows) {
    if (n_rows <= 1) { return s; }
    auto diag_len = n_rows - 2;
    auto pattern_period = n_rows + diag_len;

    auto row_starts = create_zig_zag_plan(n_rows, s.size());
    auto res = std::string(s.size(), '*');
    auto map = std::vector<int>(n_rows, 0);

    // now just random access fill the string
    for (int i = 0; i < s.size(); i++) {
        auto index_in_pattern = i % pattern_period;
        auto row = get_row(index_in_pattern, n_rows, diag_len);
        auto row_start = row_starts[row];
        auto row_index = map[row]++;
        auto new_index = row_start + row_index;
        res[new_index] = s[i];
    }

    return res;
}

