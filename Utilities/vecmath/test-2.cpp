/*
   Copyright (C) 1997,1998,1999
   Kenji Hiranabe, Eiwa System Management, Inc.

   This program is free software.
   Implemented by Kenji Hiranabe(hiranabe@esm.co.jp),
   conforming to the Java(TM) 3D API specification by Sun Microsystems.

   Permission to use, copy, modify, distribute and sell this software
   and its documentation for any purpose is hereby granted without fee,
   provided that the above copyright notice appear in all copies and
   that both that copyright notice and this permission notice appear
   in supporting documentation. Kenji Hiranabe and Eiwa System Management,Inc.
   makes no representations about the suitability of this software for any
   purpose.  It is provided "AS IS" with NO WARRANTY.
*/
#include "Vector2.h"
#include "Tuple2.h"
#include "Point2.h"

#ifdef VM_INCLUDE_NAMESPACE
using namespace kh_vecmath;
#endif

/**
 * test-2 for Vector2
 */
#ifdef TESTALL
int test_2() {
#else
int main(int, char**) {
#endif
    Point2d p1(1,2);
    Point2d p2(4,6);

    // distance
    Point2d::value_type d = p1.distance(p2);
    assert(fabs(d - 5) < 1.0e-5);

    // distance L1
    d = p1.distanceL1(p2);
    assert(d == 7);

    // distance Linf
    d = p1.distanceLinf(p2);
    assert(d == 4);

    return 0;
}
