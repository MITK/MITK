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
#include "Point3.h"
#include "Vector3.h"

#ifdef VM_INCLUDE_NAMESPACE
using namespace kh_vecmath;
#endif

template<class T, class S>
bool equals(T a, S b) {
    return fabs(a - b) < 1.0e-8;
}

/**
 * test for Point3
 */
#ifdef TESTALL
int test_4() {
#else
int main(int, char**) {
#endif
    Point3d p1(1,2,3);
    Point3d p2(4,6,1);

    // distance
    Point3d::value_type d = p1.distance(p2);
    Vector3d dv = p2 - p1;
    assert(equals(d, dv.length()));

    // distance L1
    d = p1.distanceL1(p2);
    assert(d == 9);

    // distance Linf
    d = p1.distanceLinf(p2);
    assert(d == 4);

    // project
    p1.project(Point4d(6, 4, 2, 2));
    assert(p1 == Point3d(3, 2, 1));

    return 0;
}
