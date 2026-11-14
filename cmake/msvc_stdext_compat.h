// Qt 5.15.2's qcompilerdetection.h expands QT_MAKE_CHECKED_ARRAY_ITERATOR and
// QT_MAKE_UNCHECKED_ARRAY_ITERATOR to stdext::make_(un)checked_array_iterator
// on MSVC. Recent MSVC STL releases removed those helpers, so Qt 5.15.2 headers
// no longer compile. Plain pointers are what the helpers wrapped, so hand those
// back. Force-included on MSVC builds from CMakeLists.txt.
#pragma once
#if defined(_MSC_VER) && defined(__cplusplus)
#include <cstddef>
#include <iterator>
namespace stdext {
template <class T>
inline T *make_checked_array_iterator(T *ptr, std::size_t, std::size_t = 0) { return ptr; }
template <class T>
inline T *make_unchecked_array_iterator(T *ptr) { return ptr; }
}
#endif
