/*
 *  Copyright (c) 2017, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */
#include "ServerLoad.h"

#include <cassert>

namespace facebook {
namespace memcache {

namespace {

// Constant used to convert from raw load to percent load.
constexpr uint32_t kPercentLoadNormalizer = 10e4; // 10,000
constexpr uint32_t kMaxRawLoad = kPercentLoadNormalizer * 100;

} // anonymous namespace

ServerLoad::ServerLoad(uint32_t rawLoad) noexcept
    : load_(rawLoad > kMaxRawLoad ? 0 : rawLoad) {}

/* static */
ServerLoad ServerLoad::fromPercentLoad(double percentLoad) noexcept {
  constexpr double kEpsilon = 1e-6;

  assert(percentLoad > (0.0 - kEpsilon));
  assert(percentLoad < (100.0 + kEpsilon));

  uint32_t rawLoad;
  if (percentLoad < (0.0 + kEpsilon)) {
    rawLoad = 0;
  } else if (percentLoad > (100.0 - kEpsilon)) {
    rawLoad = kMaxRawLoad;
  } else {
    rawLoad = static_cast<uint32_t>(percentLoad * kPercentLoadNormalizer);
  }

  return ServerLoad(rawLoad);
}

/* static */
const ServerLoad ServerLoad::zero() noexcept {
  static const ServerLoad emptyServerLoad(0);
  return emptyServerLoad;
}

double ServerLoad::percentLoad() const noexcept {
  return static_cast<double>(load_) / kPercentLoadNormalizer;
}

} // memcache
} // facebook
