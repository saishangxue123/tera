// Copyright (c) 2015, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef STORAGE_LEVELDB_INCLUDE_COMPACT_STRATEGY_H_
#define STORAGE_LEVELDB_INCLUDE_COMPACT_STRATEGY_H_

#include <stdint.h>
#include <string>
#include "leveldb/iterator.h"
#include "leveldb/comparator.h"

namespace leveldb {

class Slice;
class Iterator;
class InternalKeyComparator;

// the class privides the adjustment functions to
// determine whether user records are drop during
// compaction.
class CompactStrategy {
 public:
  virtual ~CompactStrategy() {}

  virtual const Comparator* RowKeyComparator() = 0;

  virtual void ExtractRowKey(const Slice& tera_key, std::string* row_key) = 0;

  virtual bool Drop(const Slice& k, uint64_t n, const std::string& lower_bound = "") = 0;

  // tera-specific, based on all-level iterators.
  // used in LowLevelScan
  virtual bool ScanDrop(const Slice& k, uint64_t n) = 0;

  virtual bool ScanMergedValue(Iterator* it, std::string* merged_value,
                               int64_t* merged_num = NULL) = 0;

  virtual bool MergeAtomicOPs(Iterator* it, std::string* merged_value, std::string* merged_key) = 0;

  // Set snapshot for CompactStrategy so that tera will not drop data entries
  // which
  // are protected by snpashot
  virtual void SetSnapshot(uint64_t snapshot) = 0;

  virtual bool CheckTag(const Slice& tera_key, bool* del_tag, int64_t* ttl_tag) = 0;

  virtual const char* Name() const = 0;
};

class DummyCompactStrategy : public CompactStrategy {
 public:
  virtual ~DummyCompactStrategy() {}

  virtual const Comparator* RowKeyComparator() { return NULL; }

  virtual void ExtractRowKey(const Slice& tera_key, std::string* row_key) {
    *row_key = tera_key.ToString();
  }

  virtual bool Drop(const Slice& k, uint64_t n, const std::string& lower_bound) { return false; }

  virtual bool ScanDrop(const Slice& k, uint64_t n) { return false; }

  virtual const char* Name() const { return "leveldb.DummyCompactStrategy"; }

  virtual void SetSnapshot(uint64_t snapshot) {
    // snapshot is taken care of by leveldb
  }

  virtual bool MergeAtomicOPs(Iterator* it, std::string* merged_value, std::string* merged_key) {
    return false;
  }

  virtual bool ScanMergedValue(Iterator* it, std::string* merged_value, int64_t* merged_num) {
    return false;
  }

  virtual bool CheckTag(const Slice& tera_key, bool* del_tag, int64_t* ttl_tag) {
    *del_tag = false;
    *ttl_tag = -1;
    return true;
  }
};

// each strategy object has its own inner status or context,
// so create anew one when needed.

class CompactStrategyFactory {
 public:
  virtual ~CompactStrategyFactory() {}
  virtual CompactStrategy* NewInstance() = 0;
  virtual const char* Name() const = 0;
  virtual void SetArg(const void* arg) = 0;
};

class DummyCompactStrategyFactory : public CompactStrategyFactory {
 public:
  virtual CompactStrategy* NewInstance() { return new DummyCompactStrategy(); }
  virtual const char* Name() const { return "leveldb.DummyCompactStrategyFactory"; }
  virtual void SetArg(const void* arg) {}
};

}  // namespace leveldb

#endif  // STORAGE_LEVELDB_INCLUDE_COMPACT_STRATEGY_H_
