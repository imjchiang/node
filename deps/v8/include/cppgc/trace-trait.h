// Copyright 2020 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef INCLUDE_CPPGC_TRACE_TRAIT_H_
#define INCLUDE_CPPGC_TRACE_TRAIT_H_

#include <type_traits>
#include "cppgc/type-traits.h"

namespace cppgc {

class Visitor;

namespace internal {

// Implementation of the default TraceTrait handling GarbageCollected and
// GarbageCollectedMixin.
template <typename T,
          bool =
              IsGarbageCollectedMixinTypeV<typename std::remove_const<T>::type>>
struct TraceTraitImpl;

}  // namespace internal

/**
 * Callback for invoking tracing on a given object.
 *
 * \param visitor The visitor to dispatch to.
 * \param object The object to invoke tracing on.
 */
using TraceCallback = void (*)(Visitor* visitor, const void* object);

/**
 * Describes how to trace an object, i.e., how to visit all Oilpan-relevant
 * fields of an object.
 */
struct TraceDescriptor {
  /**
   * Adjusted base pointer, i.e., the pointer to the class inheriting directly
   * from GarbageCollected, of the object that is being traced.
   */
  const void* base_object_payload;
  /**
   * Callback for tracing the object.
   */
  TraceCallback callback;
};

/**
 * Trait specifying how the garbage collector processes an object of type T.
 *
 * Advanced users may override handling by creating a specialization for their
 * type.
 */
template <typename T>
struct TraceTrait {
  static_assert(internal::IsTraceableV<T>, "T must have a Trace() method");

  /**
   * Accessor for retrieving a TraceDescriptor to process an object of type T.
   *
   * \param self The object to be processed.
   * \returns a TraceDescriptor to process the object.
   */
  static TraceDescriptor GetTraceDescriptor(const void* self) {
    return internal::TraceTraitImpl<T>::GetTraceDescriptor(
        static_cast<const T*>(self));
  }

  /**
   * Function invoking the tracing for an object of type T.
   *
   * \param visitor The visitor to dispatch to.
   * \param self The object to invoke tracing on.
   */
  static void Trace(Visitor* visitor, const void* self) {
    static_cast<const T*>(self)->Trace(visitor);
  }
};

namespace internal {

template <typename T>
struct TraceTraitImpl<T, false> {
  static TraceDescriptor GetTraceDescriptor(const void* self) {
    return {self, TraceTrait<T>::Trace};
  }
};

template <typename T>
struct TraceTraitImpl<T, true> {
  static TraceDescriptor GetTraceDescriptor(const void* self) {
    return static_cast<const T*>(self)->GetTraceDescriptor();
  }
};

}  // namespace internal
}  // namespace cppgc

#endif  // INCLUDE_CPPGC_TRACE_TRAIT_H_
