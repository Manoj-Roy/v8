// Copyright 2017 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef V8_INVALIDATED_SLOTS_INL_H
#define V8_INVALIDATED_SLOTS_INL_H

#include <map>

#include "src/allocation.h"
#include "src/heap/invalidated-slots.h"
#include "src/heap/spaces.h"
#include "src/objects-body-descriptors-inl.h"
#include "src/objects-body-descriptors.h"
#include "src/objects.h"

namespace v8 {
namespace internal {

bool InvalidatedSlotsFilter::IsValid(Address slot) {
#ifdef DEBUG
  DCHECK_LT(slot, sentinel_);
  // Slots must come in non-decreasing order.
  DCHECK_LE(last_slot_, slot);
  last_slot_ = slot;
#endif
  while (slot >= invalidated_end_) {
    ++iterator_;
    if (iterator_ != iterator_end_) {
      // Invalidated ranges must not overlap.
      DCHECK_LE(invalidated_end_, iterator_->first->address());
      invalidated_start_ = iterator_->first->address();
      invalidated_end_ = invalidated_start_ + iterator_->second;
    } else {
      invalidated_start_ = sentinel_;
      invalidated_end_ = sentinel_;
    }
  }
  // Now the invalidated region ends after the slot.
  if (slot < invalidated_start_) {
    // The invalidated region starts after the slot.
    return true;
  }
  // The invalidated region includes the slot.
  // Ask the object if the slot is valid.
  if (invalidated_object_ == nullptr) {
    invalidated_object_ = HeapObject::FromAddress(invalidated_start_);
    invalidated_object_size_ =
        invalidated_object_->SizeFromMap(invalidated_object_->map());
  }
  int offset = static_cast<int>(slot - invalidated_start_);
  DCHECK_GT(offset, 0);

  return offset < invalidated_object_size_ &&
         invalidated_object_->IsValidSlot(offset);
}

}  // namespace internal
}  // namespace v8

#endif  // V8_INVALIDATED_SLOTS_INL_H
