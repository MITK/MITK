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
#ifndef M_PI
#define M_PI        3.14159265358979323846
#endif



#include <vecmath.h>

#ifdef VM_INCLUDE_NAMESPACE
using namespace kh_vecmath;
#endif

#ifdef VM_USE_STD_NAMESPACE
using namespace std;
#endif

/**
 * test for toString()
 */
#ifdef TESTALL
int test_12() {
#else
int main(int, char**) {
#endif
#ifdef VM_INCLUDE_TOSTRING
    AxisAngle4d aa(1,2,3,4);
    cout << aa.toString() << endl;
    assert(aa.toString() == "(1,2,3,4)");

    Color3d c3(1,2,3);
    cout << c3.toString() << endl;
    assert(c3.toString() == "(1,2,3)");

    Color4f c4(1,2,3,4);
    cout << c4.toString() << endl;
    assert(c4.toString() == "(1,2,3,4)");

    Matrix3d m3d(1,2,3,4,5,6,7,8,9);
    cout << m3d.toString() << endl;
    assert(m3d.toString() ==
           "[ [1,2,3]\n"
           "  [4,5,6]\n"
           "  [7,8,9] ]");

    Point2f p2f(3,4);
    cout << p2f.toString() << endl;
    assert(p2f.toString() == "(3,4)");

    Quat4d q4d(3,4,1,2);
    cout << q4d.toString() << endl;
    assert(q4d.toString() == "(3,4,1,2)");

    Vector4d v4d(3,4,1,2);
    cout << v4d.toString() << endl;
    assert(v4d.toString() == "(3,4,1,2)");
#endif

    return 0;
}
