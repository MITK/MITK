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
#ifndef MATRIX3_H
#define MATRIX3_H

#include "VmUtil.h"
#include "Matrix3_.h"
#include "AxisAngle4_.h"
#include "Quat4_.h"

VM_BEGIN_NS

template<class T>
Matrix3<T>::Matrix3(T n00, T n01, T n02,
                         T n10, T n11, T n12,
                         T n20, T n21, T n22):
    m00(n00), m01(n01), m02(n02),
    m10(n10), m11(n11), m12(n12),
    m20(n20), m21(n21), m22(n22) { }

template<class T>
Matrix3<T>::Matrix3(const T v[]): 
    m00(v[ 0]), m01(v[ 1]), m02(v[ 2]),
    m10(v[ 3]), m11(v[ 4]), m12(v[ 5]),
    m20(v[ 6]), m21(v[ 7]), m22(v[ 8]) { }

#ifdef VM_INCLUDE_CONVERSION_FROM_2DARRAY
template<class T>
Matrix3<T>::Matrix3(const T v[][3]): 
    m00(v[0][0]), m01(v[0][1]), m02(v[0][2]),
    m10(v[1][0]), m11(v[1][1]), m12(v[1][2]),
    m20(v[2][0]), m21(v[2][1]), m22(v[2][2]) { }
#endif

template<class T>
Matrix3<T>::Matrix3():
        m00(0), m01(0), m02(0),
        m10(0), m11(0), m12(0),
        m20(0), m21(0), m22(0) { }

template<class T>
void Matrix3<T>::set(T m00, T m01, T m02,
                     T m10, T m11, T m12,
                     T m20, T m21, T m22) {
    this->m00 = m00; this->m01 = m01; this->m02 = m02;
    this->m10 = m10; this->m11 = m11; this->m12 = m12;
    this->m20 = m20; this->m21 = m21; this->m22 = m22;
}

template<class T>
void Matrix3<T>::set(const Matrix3& m1) {
    m00 = m1.m00; m01 = m1.m01; m02 = m1.m02;
    m10 = m1.m10; m11 = m1.m11; m12 = m1.m12;
    m20 = m1.m20; m21 = m1.m21; m22 = m1.m22;
}

template<class T>
void Matrix3<T>::set(const T v[]) {
    assert(v != 0);
    m00 = v[ 0]; m01 = v[ 1]; m02 = v[ 2];
    m10 = v[ 3]; m11 = v[ 4]; m12 = v[ 5];
    m20 = v[ 6]; m21 = v[ 7]; m22 = v[ 8];
}

#ifdef VM_INCLUDE_CONVERSION_FROM_2DARRAY
template<class T>
void Matrix3<T>::set(const T m[][3]) {
    assert(m != 0);
    m00 = m[0][0]; m01 = m[0][1]; m02 = m[0][2];
    m10 = m[1][0]; m11 = m[1][1]; m12 = m[1][2];
    m20 = m[2][0]; m21 = m[2][1]; m22 = m[2][2];
}
#endif

template<class T>
void Matrix3<T>::setIdentity() {
    m00 = 1.0; m01 = 0.0; m02 = 0.0;
    m10 = 0.0; m11 = 1.0; m12 = 0.0;
    m20 = 0.0; m21 = 0.0; m22 = 1.0;
}

template<class T>
void Matrix3<T>::setScale(T scale) {
    SVD(this);
    m00 *= scale;
    m11 *= scale;
    m22 *= scale;
}


template<class T>
void Matrix3<T>::set(const Quat4<T>& q1) {
    setFromQuat(q1.x, q1.y, q1.z, q1.w);
}

template<class T>
void Matrix3<T>::set(const AxisAngle4<T>& a1) {
    setFromAxisAngle(a1.x, a1.y, a1.z, a1.angle);
}

template<class T>
void Matrix3<T>::setElement(size_t row, size_t column, T value) {
    assert(row < 3);
    assert(column < 3);
    if (row == 0)
        if (column == 0)
            m00 = value;
        else if (column == 1)
            m01 = value;
        else if (column == 2)
            m02 = value;
        else
            VM_OUT_OF_RANGE;
    else if (row == 1)
        if (column == 0)
            m10 = value;
        else if (column == 1)
            m11 = value;
        else if (column == 2)
            m12 = value;
        else
            VM_OUT_OF_RANGE;
    else if (row == 2)
        if (column == 0)
            m20 = value;
        else if (column == 1)
            m21 = value;
        else if (column == 2)
            m22 = value;
        else
            VM_OUT_OF_RANGE;
    else 
        VM_OUT_OF_RANGE;
}

template<class T>
T Matrix3<T>::getElement(size_t row, size_t column) const {
    assert(row < 3);
    assert(column < 3);
    if (row == 0)
        if (column == 0)
            return m00;
        else if (column == 1)
            return m01;
        else if (column == 2)
            return m02;
        else {
            VM_OUT_OF_RANGE;
        }
    else if (row == 1)
        if (column == 0)
            return m10;
        else if (column == 1)
            return m11;
        else if (column == 2)
            return m12;
        else {
            VM_OUT_OF_RANGE;
        }

    else if (row == 2)
        if (column == 0)
            return m20;
        else if (column == 1)
            return m21;
        else if (column == 2)
            return m22;
        else {
            VM_OUT_OF_RANGE;
        }
    else {
        VM_OUT_OF_RANGE;
    }
    return 0;
}

template<class T>
T& Matrix3<T>::getElementReference(size_t row, size_t column) {
    assert(row < 3);
    assert(column < 3);
    if (row == 0)
        if (column == 0)
            return m00;
        else if (column == 1)
            return m01;
        else if (column == 2)
            return m02;
        else {
            VM_OUT_OF_RANGE;
        }
    else if (row == 1)
        if (column == 0)
            return m10;
        else if (column == 1)
            return m11;
        else if (column == 2)
            return m12;
        else {
            VM_OUT_OF_RANGE;
        }

    else if (row == 2)
        if (column == 0)
            return m20;
        else if (column == 1)
            return m21;
        else if (column == 2)
            return m22;
        else {
            VM_OUT_OF_RANGE;
        }
    else {
        VM_OUT_OF_RANGE;
    }

    // error anyway
    static T dummy;
    return dummy;
}

template<class T>
void Matrix3<T>::setRow(size_t row, T x, T y, T z) {
    assert(row < 3);
    if (row == 0) {
        m00 = x;
        m01 = y;
        m02 = z;
    } else if (row == 1) {
        m10 = x;
        m11 = y;
        m12 = z;
    } else if (row == 2) {
        m20 = x;
        m21 = y;
        m22 = z;
    } else {
        VM_OUT_OF_RANGE;
    }
}

template<class T>
void Matrix3<T>::setRow(size_t row, const Vector3<T>& v) {
    assert(row < 3);
    if (row == 0) {
        m00 = v.x;
        m01 = v.y;
        m02 = v.z;
    } else if (row == 1) {
        m10 = v.x;
        m11 = v.y;
        m12 = v.z;
    } else if (row == 2) {
        m20 = v.x;
        m21 = v.y;
        m22 = v.z;
    } else {
        VM_OUT_OF_RANGE;
    }
}

template<class T>
void Matrix3<T>::setRow(size_t row, const T v[]) {
    assert(v != 0);
    assert(row < 3);
    if (row == 0) {
        m00 = v[0];
        m01 = v[1];
        m02 = v[2];
    } else if (row == 1) {
        m10 = v[0];
        m11 = v[1];
        m12 = v[2];
    } else if (row == 2) {
        m20 = v[0];
        m21 = v[1];
        m22 = v[2];
    } else {
        VM_OUT_OF_RANGE;
    }
}

template<class T>
void Matrix3<T>::getRow(size_t row, T v[]) const {
    assert(v != 0);
    assert(row < 3);
    if (row == 0) {
        v[0] = m00;
        v[1] = m01;
        v[2] = m02;
    } else if (row == 1) {
        v[0] = m10;
        v[1] = m11;
        v[2] = m12;
    } else if (row == 2) {
        v[0] = m20;
        v[1] = m21;
        v[2] = m22;
    } else {
        VM_OUT_OF_RANGE;
    }
}

template<class T>
void Matrix3<T>::getRow(size_t row, Vector3<T>* v) const {
    assert(v != 0);
    assert(row < 3);
    if (row == 0) {
        v->x = m00;
        v->y = m01;
        v->z = m02;
    } else if (row == 1) {
        v->x = m10;
        v->y = m11;
        v->z = m12;
    } else if (row == 2) {
        v->x = m20;
        v->y = m21;
        v->z = m22;
    } else {
        VM_OUT_OF_RANGE;
    }
}

template<class T>
void Matrix3<T>::setColumn(size_t column, T x, T y, T z) {
    assert(column < 3);
    if (column == 0) {
        m00 = x;
        m10 = y;
        m20 = z;
    }  else if (column == 1) {
        m01 = x;
        m11 = y;
        m21 = z;
    } else if (column == 2) {
        m02 = x;
        m12 = y;
        m22 = z;
    } else {
        VM_OUT_OF_RANGE;
    }
}

template<class T>
void Matrix3<T>::setColumn(size_t column, const Vector3<T>& v) {
    assert(column < 3);
    if (column == 0) {
        m00 = v.x;
        m10 = v.y;
        m20 = v.z;
    } else if (column == 1) {
        m01 = v.x;
        m11 = v.y;
        m21 = v.z;
    } else if (column == 2) {
        m02 = v.x;
        m12 = v.y;
        m22 = v.z;
    } else {
        VM_OUT_OF_RANGE;
    }
}

template<class T>
void Matrix3<T>::setColumn(size_t column,  const T v[]) {
    assert(v != 0);
    assert(column < 3);
    if (column == 0) {
        m00 = v[0];
        m10 = v[1];
        m20 = v[2];
    } else if (column == 1) {
        m01 = v[0];
        m11 = v[1];
        m21 = v[2];
    } else if (column == 2) {
        m02 = v[0];
        m12 = v[1];
        m22 = v[2];
    } else {
        VM_OUT_OF_RANGE;
    }
}


template<class T>
void Matrix3<T>::getColumn(size_t column, Vector3<T>* v) const {
    assert(column < 3);
    assert(v != 0);
    if (column == 0) {
        v->x = m00;
        v->y = m10;
        v->z = m20;
    } else if (column == 1) {
        v->x = m01;
        v->y = m11;
        v->z = m21;
    } else if (column == 2) {
        v->x = m02;
        v->y = m12;
        v->z = m22;
    } else {
        VM_OUT_OF_RANGE;
    }
}

template<class T>
void Matrix3<T>::getColumn(size_t column,  T v[]) const {
    assert(column < 3);
    assert(v != 0);
    if (column == 0) {
        v[0] = m00;
        v[1] = m10;
        v[2] = m20;
    } else if (column == 1) {
        v[0] = m01;
        v[1] = m11;
        v[2] = m21;
    } else if (column == 2) {
        v[0] = m02;
        v[1] = m12;
        v[2] = m22;
    } else {
        VM_OUT_OF_RANGE;
    }
}

template<class T>
T Matrix3<T>::getScale() const {
    return SVD(0);
}

template<class T>
void Matrix3<T>::add(T scalar) {
    m00 += scalar; m01 += scalar; m02 += scalar;
    m10 += scalar; m11 += scalar; m12 += scalar;
    m20 += scalar; m21 += scalar; m22 += scalar;
}

template<class T>
void Matrix3<T>::sub(T scalar) {
    add(-scalar);
}

template<class T>
void Matrix3<T>::add(T scalar, const Matrix3& m1) {
    set(m1);
    add(scalar);
}


template<class T>
void Matrix3<T>::add(const Matrix3& m1, const Matrix3& m2) {
    // note this is alias safe.
    set(
        m1.m00 + m2.m00,
        m1.m01 + m2.m01,
        m1.m02 + m2.m02,
        m1.m10 + m2.m10,
        m1.m11 + m2.m11,
        m1.m12 + m2.m12,
        m1.m20 + m2.m20,
        m1.m21 + m2.m21,
        m1.m22 + m2.m22
        );
}


template<class T>
void Matrix3<T>::add(const Matrix3& m1) {
    m00 += m1.m00; m01 += m1.m01; m02 += m1.m02;
    m10 += m1.m10; m11 += m1.m11; m12 += m1.m12;
    m20 += m1.m20; m21 += m1.m21; m22 += m1.m22;
}


template<class T>
void Matrix3<T>::sub(const Matrix3& m1, const Matrix3& m2) {
    // note this is alias safe.
    set(
        m1.m00 - m2.m00,
        m1.m01 - m2.m01,
        m1.m02 - m2.m02,
        m1.m10 - m2.m10,
        m1.m11 - m2.m11,
        m1.m12 - m2.m12,
        m1.m20 - m2.m20,
        m1.m21 - m2.m21,
        m1.m22 - m2.m22
        );
}

template<class T>
void Matrix3<T>::sub(const Matrix3& m1) {
    m00 -= m1.m00; m01 -= m1.m01; m02 -= m1.m02;
    m10 -= m1.m10; m11 -= m1.m11; m12 -= m1.m12;
    m20 -= m1.m20; m21 -= m1.m21; m22 -= m1.m22;
}


template<class T>
void Matrix3<T>::transpose() {
    T tmp = m01;
    m01 = m10;
    m10 = tmp;

    tmp = m02;
    m02 = m20;
    m20 = tmp;

    tmp = m12;
    m12 = m21;
    m21 = tmp;
}


template<class T>
void Matrix3<T>::transpose(const Matrix3& m1) {
    // alias-safe
    set(m1);
    transpose();
}


template<class T>
void Matrix3<T>::invert(const Matrix3& m1)  {
    // alias-safe way.
    set(m1);
    invert();
}


template<class T>
void Matrix3<T>::invert() {
    T s = determinant();
    // zero div may happen
    s = 1/s;
    // alias-safe way.
    set(
        m11*m22 - m12*m21, m02*m21 - m01*m22, m01*m12 - m02*m11,
        m12*m20 - m10*m22, m00*m22 - m02*m20, m02*m10 - m00*m12,
        m10*m21 - m11*m20, m01*m20 - m00*m21, m00*m11 - m01*m10
        );
    mul(s);
}


template<class T>
T Matrix3<T>::determinant() const {
    // less *,+,- calculation than expanded expression.
    return m00*(m11*m22 - m21*m12)
        - m01*(m10*m22 - m20*m12)
        + m02*(m10*m21 - m20*m11);
}


template<class T>
void Matrix3<T>::set(T scale) {
    m00 = scale; m01 = 0.0;   m02 = 0.0;
    m10 = 0.0;   m11 = scale; m12 = 0.0;
    m20 = 0.0;   m21 = 0.0;   m22 = scale;
}

template<class T>
void Matrix3<T>::rotX(T angle) {
	T c = VmUtil<T>::cos(angle);
	T s = VmUtil<T>::sin(angle);
    m00 = 1.0; m01 = 0.0; m02 = 0.0;
    m10 = 0.0; m11 = c;   m12 = -s;
    m20 = 0.0; m21 = s;   m22 = c;
}

template<class T>
void Matrix3<T>::rotY(T angle) {
	T c = VmUtil<T>::cos(angle);
	T s = VmUtil<T>::sin(angle);
    m00 = c;   m01 = 0.0; m02 = s;
    m10 = 0.0; m11 = 1.0; m12 = 0.0;
    m20 = -s;  m21 = 0.0; m22 = c;
}

template<class T>
void Matrix3<T>::rotZ(T angle) {
	T c = VmUtil<T>::cos(angle);
	T s = VmUtil<T>::sin(angle);
    m00 = c;   m01 = -s;  m02 = 0.0;
    m10 = s;   m11 = c;   m12 = 0.0;
    m20 = 0.0; m21 = 0.0; m22 = 1.0;
}

template<class T>
void Matrix3<T>::mul(T scalar) {
    m00 *= scalar; m01 *= scalar;  m02 *= scalar;
    m10 *= scalar; m11 *= scalar;  m12 *= scalar;
    m20 *= scalar; m21 *= scalar;  m22 *= scalar;
}

template<class T>
void Matrix3<T>::mul(T scalar, const Matrix3& m1) {
    set(m1);
    mul(scalar);
}

template<class T>
void Matrix3<T>::mul(const Matrix3& m1) {
    mul(*this, m1);
}


template<class T>
void Matrix3<T>::mul(const Matrix3& m1, const Matrix3& m2) {
    // alias-safe way.
    set(
        m1.m00*m2.m00 + m1.m01*m2.m10 + m1.m02*m2.m20,
        m1.m00*m2.m01 + m1.m01*m2.m11 + m1.m02*m2.m21,
        m1.m00*m2.m02 + m1.m01*m2.m12 + m1.m02*m2.m22,

        m1.m10*m2.m00 + m1.m11*m2.m10 + m1.m12*m2.m20,
        m1.m10*m2.m01 + m1.m11*m2.m11 + m1.m12*m2.m21,
        m1.m10*m2.m02 + m1.m11*m2.m12 + m1.m12*m2.m22,

        m1.m20*m2.m00 + m1.m21*m2.m10 + m1.m22*m2.m20,
        m1.m20*m2.m01 + m1.m21*m2.m11 + m1.m22*m2.m21,
        m1.m20*m2.m02 + m1.m21*m2.m12 + m1.m22*m2.m22
        );
}

template<class T>
void Matrix3<T>::mulTransposeRight(const Matrix3& m1, const Matrix3& m2) {
    // alias-safe way.
    set(
        m1.m00*m2.m00 + m1.m01*m2.m01 + m1.m02*m2.m02,
        m1.m00*m2.m10 + m1.m01*m2.m11 + m1.m02*m2.m12,
        m1.m00*m2.m20 + m1.m01*m2.m21 + m1.m02*m2.m22,

        m1.m10*m2.m00 + m1.m11*m2.m01 + m1.m12*m2.m02,
        m1.m10*m2.m10 + m1.m11*m2.m11 + m1.m12*m2.m12,
        m1.m10*m2.m20 + m1.m11*m2.m21 + m1.m12*m2.m22,

        m1.m20*m2.m00 + m1.m21*m2.m01 + m1.m22*m2.m02,
        m1.m20*m2.m10 + m1.m21*m2.m11 + m1.m22*m2.m12,
        m1.m20*m2.m20 + m1.m21*m2.m21 + m1.m22*m2.m22
        );
}

template<class T>
void Matrix3<T>::mulTransposeLeft(const Matrix3& m1, const Matrix3& m2) {
    // alias-safe way.
    set(
        m1.m00*m2.m00 + m1.m10*m2.m10 + m1.m20*m2.m20,
        m1.m00*m2.m01 + m1.m10*m2.m11 + m1.m20*m2.m21,
        m1.m00*m2.m02 + m1.m10*m2.m12 + m1.m20*m2.m22,

        m1.m01*m2.m00 + m1.m11*m2.m10 + m1.m21*m2.m20,
        m1.m01*m2.m01 + m1.m11*m2.m11 + m1.m21*m2.m21,
        m1.m01*m2.m02 + m1.m11*m2.m12 + m1.m21*m2.m22,

        m1.m02*m2.m00 + m1.m12*m2.m10 + m1.m22*m2.m20,
        m1.m02*m2.m01 + m1.m12*m2.m11 + m1.m22*m2.m21,
        m1.m02*m2.m02 + m1.m12*m2.m12 + m1.m22*m2.m22
        );
}


template<class T>
bool Matrix3<T>::equals(const Matrix3& m1) const {
    return  m00 == m1.m00
        && m01 == m1.m01
        && m02 == m1.m02 
        && m10 == m1.m10
        && m11 == m1.m11
        && m12 == m1.m12
        && m20 == m1.m20
        && m21 == m1.m21
        && m22 == m1.m22;
}

template<class T>
bool Matrix3<T>::epsilonEquals(const Matrix3& m1, T epsilon) const {
    return  abs(m00 - m1.m00) <= epsilon
        && abs(m01 - m1.m01) <= epsilon
        && abs(m02 - m1.m02 ) <= epsilon

        && abs(m10 - m1.m10) <= epsilon
        && abs(m11 - m1.m11) <= epsilon
        && abs(m12 - m1.m12) <= epsilon

        && abs(m20 - m1.m20) <= epsilon
        && abs(m21 - m1.m21) <= epsilon
        && abs(m22 - m1.m22) <= epsilon;
}


template<class T>
void Matrix3<T>::setZero() {
    m00 = 0.0; m01 = 0.0; m02 = 0.0;
    m10 = 0.0; m11 = 0.0; m12 = 0.0;
    m20 = 0.0; m21 = 0.0; m22 = 0.0;
}

template<class T>
void Matrix3<T>::negate() {
    m00 = -m00; m01 = -m01; m02 = -m02;
    m10 = -m10; m11 = -m11; m12 = -m12;
    m20 = -m20; m21 = -m21; m22 = -m22;
}


template<class T>
void Matrix3<T>::transform(const Tuple3<T>& t, Tuple3<T>* result) const {
    // alias-safe
    result->set(
                m00*t.x + m01*t.y + m02*t.z,
                m10*t.x + m11*t.y + m12*t.z,
                m20*t.x + m21*t.y + m22*t.z
                );
}


template<class T>
T Matrix3<T>::SVD(Matrix3* rot) const {
    // this is a simple svd.
    // Not complete but fast and reasonable.

        /*
         * SVD scale factors(squared) are the 3 roots of
         * 
         *     | xI - M*MT | = 0.
         * 
         * This will be expanded as follows
         * 
         * x^3 - A x^2 + B x - C = 0
         * 
         * where A, B, C can be denoted by 3 roots x0, x1, x2.
         *
         * A = (x0+x1+x2), B = (x0x1+x1x2+x2x0), C = x0x1x2.
         *
         * An avarage of x0,x1,x2 is needed here. C^(1/3) is a cross product
         * normalization factor.
         * So here, I use A/3. Note that x should be sqrt'ed for the
         * actual factor.
         */

    T s = VmUtil<T>::sqrt((
                   m00*m00 + m10*m10 + m20*m20 + 
                   m01*m01 + m11*m11 + m21*m21 +
                   m02*m02 + m12*m12 + m22*m22
                   )/3.0
                  );

    if (rot) {
        // zero-div may occur.
        double n = 1/VmUtil<T>::sqrt(m00*m00 + m10*m10 + m20*m20);
        rot->m00 = m00*n;
        rot->m10 = m10*n;
        rot->m20 = m20*n;

        n = 1/VmUtil<T>::sqrt(m01*m01 + m11*m11 + m21*m21);
        rot->m01 = m01*n;
        rot->m11 = m11*n;
        rot->m21 = m21*n;

        n = 1/VmUtil<T>::sqrt(m02*m02 + m12*m12 + m22*m22);
        rot->m02 = m02*n;
        rot->m12 = m12*n;
        rot->m22 = m22*n;
    }

    return s;
}


template<class T>
void Matrix3<T>::setFromQuat(T x, T y, T z, T w) {
    T n = x*x + y*y + z*z + w*w;
    T s = (n > 0.0) ? (2.0/n) : 0.0;

    T xs = x*s,  ys = y*s,  zs = z*s;
    T wx = w*xs, wy = w*ys, wz = w*zs;
    T xx = x*xs, xy = x*ys, xz = x*zs;
    T yy = y*ys, yz = y*zs, zz = z*zs;

    m00 = 1.0 - (yy + zz); m01 = xy - wz;         m02 = xz + wy;
    m10 = xy + wz;         m11 = 1.0 - (xx + zz); m12 = yz - wx;
    m20 = xz - wy;         m21 = yz + wx;         m22 = 1.0 - (xx + yy);
}

template<class T>
void Matrix3<T>::setFromAxisAngle(T x, T y, T z, T angle) {
    // Taken from Rick's which is taken from Wertz. pg. 412
    // Bug Fixed and changed into right-handed by hiranabe
    T n = VmUtil<T>::sqrt(x*x + y*y + z*z);
    // zero-div may occur
    n = 1/n;
    x *= n;
    y *= n;
    z *= n;
    T c = VmUtil<T>::cos(angle);
    T s = VmUtil<T>::sin(angle);
    T omc = 1.0 - c;
    m00 = c + x*x*omc;
    m11 = c + y*y*omc;
    m22 = c + z*z*omc;

    T tmp1 = x*y*omc;
    T tmp2 = z*s;
    m01 = tmp1 - tmp2;
    m10 = tmp1 + tmp2;

    tmp1 = x*z*omc;
    tmp2 = y*s;
    m02 = tmp1 + tmp2;
    m20 = tmp1 - tmp2;

    tmp1 = y*z*omc;
    tmp2 = x*s;
    m12 = tmp1 - tmp2;
    m21 = tmp1 + tmp2;
}

#ifdef VM_INCLUDE_TOSTRING
template <class T>
VM_STRING_STD::string Matrix3<T>::toString() const {
    VM_TOSTRING
}
#endif /* VM_INCLUDE_TOSTRING */

VM_END_NS

#ifdef VM_INCLUDE_IO
template <class T>
VM_IOSTREAM_STD::ostream& operator<<(VM_IOSTREAM_STD::ostream& o, const VM_VECMATH_NS::Matrix3<T>& t1) {
    return  o << 
		"[ ["<<t1.m00<<","<<t1.m01<<","<<t1.m02<<"]" << VM_IOSTREAM_STD::endl <<
		"  ["<<t1.m10<<","<<t1.m11<<","<<t1.m12<<"]" << VM_IOSTREAM_STD::endl <<
        "  ["<<t1.m20<<","<<t1.m21<<","<<t1.m22<<"] ]";
}

// Anfang Änderung Marcus
template <class T>
VM_IOSTREAM_STD::istream& operator>>(VM_IOSTREAM_STD::istream& i, const VM_VECMATH_NS::Matrix3<T>& t1) {

	char c;

	i >> c;
	i >> c;
	i >> (T) t1.m00;
	i >> c;
	i >> (T) t1.m01;
	i >> c;
	i >> (T) t1.m02;
	i >> c; 
	i >> c;
	i >> (T) t1.m10;
	i >> c;
	i >> (T) t1.m11;
	i >> c;
	i >> (T) t1.m12;
	i >> c;
	i >> c;
	i >> (T) t1.m20;
	i >> c;
	i >> (T) t1.m21;
	i >> c;
	i >> (T) t1.m22;
	i >> c;
	i >> c;

	return i;
}
// Ende Änderung Marcus

#endif /* VM_INCLUDE_IO */


#endif /* MATRIX3_H */
