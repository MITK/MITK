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
#include "vecmath.h"


/*
 * test of instanciation
 */
//template class Tuple2<double>;
//template class Tuple2<float>;
//template class Vector2<double>;
//template class Vector2<float>;
//template class Point2<double>;

#ifdef VM_INCLUDE_NAMESPACE
using namespace kh_vecmath;
#endif


/**
 * test for Vector2, Tuple2, Point2
 */
#ifdef TESTALL
int test_1() {
#else
int main(int, char**) {
#endif
    // creation, ==
    Tuple2<double> d(1,2);
    d += Tuple2<double>(2,3);
    assert(d == Tuple2<double>(3,5));

    // size_type, hashCode
    Tuple2<float> f(1,2);
    Tuple2f::size_type hashf = f.hashCode();
    Point2f p1(1,2);
    Point2f p2(1,2.0001f);
    Point2f::size_type hashp1 = p1.hashCode();
    Point2f::size_type hashp2 = p2.hashCode();
    assert(hashf == hashp1);
    assert(hashp1 != hashp2); // fails very very rarely.

    // set
    f.set(1, 2);
    f *= 3;
    float farray[] = { 3, 6};
    assert(f == Tuple2<float>(farray));
#ifdef VM_INCLUDE_TOSTRING
    assert(f.toString() == "(3,6)");
#endif

    // get
    Tuple2f::value_type farray2[2];
    f.get(farray2);
    assert(farray2[0] == 3 && farray2[1] == 6);
    Tuple2f ff2;
    f.get(&ff2);
    assert(ff2 == Tuple2f(3,6));

    return 0;
}
