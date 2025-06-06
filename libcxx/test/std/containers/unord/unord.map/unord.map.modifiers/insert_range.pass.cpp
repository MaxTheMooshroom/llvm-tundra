//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++03, c++11, c++14, c++17, c++20
// Some fields in the test case variables are deliberately not explicitly initialized, this silences a warning on GCC.
// ADDITIONAL_COMPILE_FLAGS(gcc-style-warnings): -Wno-missing-field-initializers

// <unordered_map>

// template<container-compatible-range<value_type> R>
//   void insert_range(R&& rg); // C++23

#include <unordered_map>

#include "../../../insert_range_maps_sets.h"
#include "test_macros.h"

int main(int, char**) {
  // Note: we want to use a pair with non-const elements for input (an assignable type is a lot more convenient) but
  // have to use the exact `value_type` of the map (that is, `pair<const K, V>`) for the allocator.
  using Pair      = std::pair<int, char>;
  using ConstPair = std::pair<const int, char>;
  for_all_iterators_and_allocators<ConstPair, const Pair*>([]<class Iter, class Sent, class Alloc>() {
    test_map_set_insert_range<std::unordered_map<int, char, test_hash<int>, test_equal_to<int>, Alloc>,
                              Pair,
                              Iter,
                              Sent>();
  });

  static_assert(test_map_constraints_insert_range<std::unordered_map, int, int, char, double>());

  test_map_insert_range_move_only<std::unordered_map>();

  test_map_insert_range_exception_safety_throwing_copy<std::unordered_map>();
  test_unord_map_insert_range_exception_safety_throwing_allocator<std::unordered_map, int, int>();

  return 0;
}
