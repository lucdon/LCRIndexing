#pragma once

#include <locale>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <streambuf>

#ifdef __GNUC__
    #if __GNUC_PREREQ(8, 0)
        #include <filesystem>
        #define FILESYSTEM_ENABLED 1
    #else
        #define FILESYSTEM_ENABLED 0
    #endif
#else
    #include <filesystem>

    #define FILESYSTEM_ENABLED 1
#endif

#include <regex>
#include <atomic>
#include <algorithm>
#include <tuple>
#include <any>
#include <optional>
#include <variant>
#include <functional>
#include <memory>
#include <thread>
#include <mutex>
#include <shared_mutex>
#include <future>
#include <condition_variable>
#include <utility>
#include <string>
#include <random>
#include <numeric>
#include <ratio>
#include <limits>
#include <stack>
#include <queue>
#include <list>
#include <forward_list>
#include <deque>
#include <array>
#include <vector>
#include <iterator>
#include <bitset>
#include <set>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <stdexcept>
#include <cmath>
#include <string_view>

#include "utility/rss.hpp"

#include "boost/dynamic_bitset.hpp"

namespace std {
    template<class _Elem, class _Traits>
    basic_ostream<_Elem, _Traits> &fatal(basic_ostream<_Elem, _Traits> &_Ostr) {
        _Ostr.put(_Ostr.widen('\n'));
        _Ostr.flush();

        exit(1);
    }
}