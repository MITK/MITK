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
#include "Matrix4.h"

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
    Matrix4<T> m1(1,2,3,4,
                 5,6,7,8,
                 9,10,11,12,
                 13,14,15,16);
                 
    T array[] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
    Matrix4<T> m2(array);

#ifdef VM_INCLUDE_CONVERSION_FROM_2DARRAY
    T array2[4][4] = {{1,2,3,4},{5,6,7,8},{9,10,11,12},{13,14,15,16}};
    Matrix4<T> m3(array2);
#else
    Matrix4<T> m3(m2);
#endif

    int k = 0;
    for (unsigned i = 0; i < Matrix4<T>::DIMENSION; i++) {
        for (unsigned j = 0; j < Matrix4<T>::DIMENSION; j++) {
            k++;
#ifdef VM_INCLUDE_SUBSCRIPTION_OPERATOR
            assert(m1(i,j) == k);
            assert(m2(i,j) == k);
            assert(m3(i,j) == k);
#endif
            assert(m1.getElement(i,j) == k);
            assert(m2.getElement(i,j) == k);
            assert(m3.getElement(i,j) == k);
        }
    }
    assert(m1 == m2);
    assert(m1 == m3);

    /*
     * set/get row/column
     */
    m1.setIdentity();
    m1.setScale(2);
    assert(m1 == Matrix4<T>(2,0,0,0,
                            0,2,0,0,
                            0,0,2,0,
                            0,0,0,1));

#ifdef VM_INCLUDE_SUBSCRIPTION_OPERATOR
    for (unsigned i = 0; i < 4; i++)
        for (unsigned j = 0; j < 4; j++)
            m1(i,j) = 10*i + j;
    for (unsigned ii = 0; ii < 4; ii++)
        for (unsigned j = 0; j < 4; j++)
            assert(10*ii + j == m1(ii,j));
#endif
    for (unsigned i2 = 0; i2 < 4; i2++)
        for (unsigned j = 0; j < 4; j++)
            m1.setElement(i2,j,10*i2 + j);
    for (unsigned i3 = 0; i3 < 4; i3++)
        for (unsigned j = 0; j < 4; j++)
            assert(10*i3 + j == m1.getElement(i3,j));

    for (unsigned i4 = 0; i4 < 4; i4++) {
        Vector4<T> v1(10*i4,6*i4,i4,17*i4);
        Vector4<T> v2;
        m1.setRow(i4, v1);
        m1.getRow(i4, &v2);
        assert(v1 == v2);
    }

    for (unsigned i5 = 0; i5 < 4; i5++) {
        const T t[] = { 1, 2, 3, 4 };
        T s[4];
        m1.setRow(i5, t);
        m1.getRow(i5, s);
        for (unsigned j = 0; j < 4; j ++)
            assert(t[j] == s[j]);
    }

    for (unsigned i6 = 0; i6 < 4; i6++) {
        Vector4<T> v1(7*i6,5*i6,i6,13*(-i6));
        Vector4<T> v2;
        m1.setColumn(i6, v1);
        m1.getColumn(i6, &v2);
        assert(v1 == v2);
    }

    for (unsigned i7 = 0; i7 < 4; i7++) {
        const T t[] = { 1, 2, 3, 4};
        T s[4];
        m1.setColumn(i7, t);
        m1.getColumn(i7, s);
        for (unsigned j = 0; j < 4; j ++)
            assert(t[j] == s[j]);
    }

    /*
     * scales
     */
     m1.setIdentity();
    for (unsigned i8 = 1; i8 < 10; i8++) {
        m1.setScale(i8);
        T s = m1.getScale();
        assert(equals(s, i8));
    }

    /*
     * add/sub
     */
    m1.set(1, 2, 3, 4,
           5, 6, 7, 8,
           9, 10,11,12,
           13,14,15,16);
    m2.add(10, m1);
    m2.sub(10);
    assert(m1 == m2);

    /**
     * negate
     */
    m1.set(1, 2, 3, 10,
           4, 5, 6, 11,
           2, 5, 1, 100,
           9, 3, 4, 12);
    m2.negate(m1);
    m3.add(m1, m2);
    assert(m3 == Matrix4<T>(0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0));

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
    assert(equals(m2(0,3),0));
    assert(equals(m2(1,0),0));
    assert(equals(m2(1,1),1));
    assert(equals(m2(1,2),0));
    assert(equals(m2(1,3),0));
    assert(equals(m2(2,0),0));
    assert(equals(m2(2,1),0));
    assert(equals(m2(2,2),1));
    assert(equals(m2(2,3),0));
    assert(equals(m2(3,0),0));
    assert(equals(m2(3,1),0));
    assert(equals(m2(3,2),0));
    assert(equals(m2(3,3),1));
#endif
    assert(equals(m2.getElement(0,0),1));
    assert(equals(m2.getElement(0,1),0));
    assert(equals(m2.getElement(0,2),0));
    assert(equals(m2.getElement(0,3),0));
    assert(equals(m2.getElement(1,0),0));
    assert(equals(m2.getElement(1,1),1));
    assert(equals(m2.getElement(1,2),0));
    assert(equals(m2.getElement(1,3),0));
    assert(equals(m2.getElement(2,0),0));
    assert(equals(m2.getElement(2,1),0));
    assert(equals(m2.getElement(2,2),1));
    assert(equals(m2.getElement(2,3),0));
    assert(equals(m2.getElement(3,0),0));
    assert(equals(m2.getElement(3,1),0));
    assert(equals(m2.getElement(3,2),0));
    assert(equals(m2.getElement(3,3),1));

}

/**
 * test for Matrix3
 */
#ifdef TESTALL
int test_9() {
#else
int main(int, char**) {
#endif
    f(1.0);
    f(1.0f);
    return 0;
}

