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
#include "Matrix3.h"

template<class T, class S>
bool equals(T a, S b) {
    return fabs(a - b) < 1.0e-5;
}

#ifdef VM_INCLUDE_NAMESPACE
using namespace kh_vecmath;
#endif

template<class T>
void f(T) {
    /*
     * constructors
     */
    Matrix3<T> m1(1,2,3,
                 4,5,6,
                 7,8,9);

    T array[] = {1,2,3,4,5,6,7,8,9};
    // const T array2[3][3] = {{1,2,3},{4,5,6},{7,8,9}};
    //    Matrix3<T> m3(array2);
    Matrix3<T> m2(array);
    Matrix3<T> m3(m1);
    

#ifdef VM_INCLUDE_SUBSCRIPTION_OPERATOR
    assert(m1(0,0) == 1);
    assert(m1(0,1) == 2);
    assert(m1(0,2) == 3);
    assert(m1(1,0) == 4);
    assert(m1(1,1) == 5);
    assert(m1(1,2) == 6);
    assert(m1(2,0) == 7);
    assert(m1(2,1) == 8);
    assert(m1(2,2) == 9);
#endif
    assert(m1.getElement(0,0) == 1);
    assert(m1.getElement(0,1) == 2);
    assert(m1.getElement(0,2) == 3);
    assert(m1.getElement(1,0) == 4);
    assert(m1.getElement(1,1) == 5);
    assert(m1.getElement(1,2) == 6);
    assert(m1.getElement(2,0) == 7);
    assert(m1.getElement(2,1) == 8);
    assert(m1.getElement(2,2) == 9);
    assert(m1 == m2);
    assert(m1 == m3);

    /*
     * set/get row/column
     */
    m1.setIdentity();
    m1.setScale(2);
    assert(m1 == Matrix3<T>(2,0,0,0,2,0,0,0,2));
#ifdef VM_INCLUDE_SUBSCRIPTION_OPERATOR
    for (unsigned i = 0; i < 3; i++)
        for (unsigned j = 0; j < 3; j++)
            m1(i,j) = 10*i + j;
    for (unsigned i2 = 0; i2 < 3; i2++)
        for (unsigned j = 0; j < 3; j++)
            assert(m1(i2,j) == 10*i2 + j);
#endif

    for (unsigned ii = 0; ii< 3; ii++)
        for (unsigned j = 0; j < 3; j++)
            m1.setElement(ii,j, 10*ii + j);
    for (unsigned iii = 0; iii < 3; iii++)
        for (unsigned j = 0; j < 3; j++)
            assert(m1.getElement(iii,j) == 10*iii + j);


    for (unsigned j = 0; j < 3; j++) {
        Vector3<T> v1(10*j,6*j,j);
        Vector3<T> v2;
        m1.setRow(j, v1);
        m1.getRow(j, &v2);
        assert(v1 == v2);
    }

    for (unsigned i3 = 0; i3 < 3; i3++) {
        T t[] = { 1, 2, 3 };
        T s[3];
        m1.setRow(i3, t);
        m1.getRow(i3, s);
        for (unsigned j = 0; j < 3; j ++)
            assert(t[j] == s[j]);
    }

    for (unsigned i4 = 0; i4 < 3; i4++) {
        Vector3<T> v1(7*i4,6*i4,i4);
        Vector3<T> v2;
        m1.setColumn(i4, v1);
        m1.getColumn(i4, &v2);
        assert(v1 == v2);
    }

    for (unsigned i5 = 0; i5 < 3; i5++) {
        T t[] = { 1, 2, 3 };
        T s[3];
        m1.setColumn(i5, t);
        m1.getColumn(i5, s);
        for (unsigned j = 0; j < 3; j ++)
            assert(t[j] == s[j]);
    }

    /*
     * scales
     */
     m1.setIdentity();
    for (unsigned i6 = 1; i6 < 10; i6++) {
        m1.setScale(i6);
        T s = m1.getScale();
        assert(equals(s, i6));
    }

    /*
     * add/sub
     */
    m1.set(1, 2, 3,
           4, 5, 6,
           9, 3, 4);
    m2.add(10, m1);
    m2.sub(10);
    assert(m1 == m2);

    /**
     * negate
     */
    m1.set(1, 2, 3,
           4, 5, 6,
           9, 3, 4);
    m2.negate(m1);
    m3.add(m1, m2);
    assert(m3 == Matrix3<T>(0,0,0,0,0,0,0,0,0));

    /*
     * transpose
     */
    m2.transpose(m1);
    m2.transpose();
    assert(m1 == m2);

    /*
     * invert
     */
    m2.invert(m1);
    m2 *= m1;
#ifdef VM_INCLUDE_SUBSCRIPTION_OPERATOR
    assert(equals(m2(0,0),1));
    assert(equals(m2(0,1),0));
    assert(equals(m2(0,2),0));
    assert(equals(m2(1,0),0));
    assert(equals(m2(1,1),1));
    assert(equals(m2(1,2),0));
    assert(equals(m2(2,0),0));
    assert(equals(m2(2,1),0));
    assert(equals(m2(2,2),1));
#endif
    assert(equals(m2.getElement(0,0),1));
    assert(equals(m2.getElement(0,1),0));
    assert(equals(m2.getElement(0,2),0));
    assert(equals(m2.getElement(1,0),0));
    assert(equals(m2.getElement(1,1),1));
    assert(equals(m2.getElement(1,2),0));
    assert(equals(m2.getElement(2,0),0));
    assert(equals(m2.getElement(2,1),0));
    assert(equals(m2.getElement(2,2),1));

}

/**
 * test for Matrix3
 */
#ifdef TESTALL
int test_8() {
#else
int main(int, char**) {
#endif
    f(1.0);
    f(1.0f);
    return 0;
}

