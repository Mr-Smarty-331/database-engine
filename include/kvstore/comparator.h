// include/kvstore/comparator.h

#pragma once

#include "kvstore/slice.h"
#include <string>

namespace kvstore {

class Comparator {
public:
  virtual ~Comparator() = default;

  // Three-way comparison. Returns value:
  //   < 0 if "a" < "b"
  //   = 0 if "a" == "b"
  //   > 0 if "a" > "b"
  virtual int Compare(const Slice &a, const Slice &b) const = 0;

  // The name of the comparator. Used for debugging and logging.
  virtual const char *Name() const = 0;
};

// The default comparator for our key-value store.
// It performs a simple bytewise comparison of keys.
class BytewiseComparator : public Comparator {
public:
  BytewiseComparator() = default;

  const char *Name() const override { return "kvstore.BytewiseComparator"; }

  int Compare(const Slice &a, const Slice &b) const override {
    return a.compare(b);
  }
};

} // namespace kvstore