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

#ifndef M_PI
#define M_PI        3.14159265358979323846
#endif

#ifdef VM_INCLUDE_NAMESPACE
using namespace kh_vecmath;
#endif

template<class T, class S>
bool equals(T a, S b) {
    return fabs(a - b) < 1.0e-8;
}

/**
 * test-3 for Vector2
 */
#ifdef TESTALL
int test_3() {
#else
int main(int, char**) {
#endif
    Vector2d p1(1,2);
    Vector2d p2(4,6);

    // dot product
    Vector2d::value_type d = p1.dot(p2);
    assert(equals(d, 16));

    // length
    d = p1.length();
    assert(equals(d, sqrt(p1.dot(p1))));

    // normalize
    p1.normalize();
    assert(equals(p1.x,1/d));
    assert(equals(p1.y,2/d));

    p1.set(-1,3);
    d = p1.length();
    p2.normalize(p1);
    assert(equals(p2.x,-1/d));
    assert(equals(p2.y, 3/d));
    
    // angle
    p1.set(3, 4);
    p2.set(-4, 3);
    d = p1.angle(p2);
    assert(equals(d, M_PI/2));
    d = p2.angle(p1);
    assert(equals(d, M_PI/2));
    d = p1.angle(p1);
    assert(equals(d, 0));
    d = p2.angle(p2);
    assert(equals(d, 0));

    return 0;
}
