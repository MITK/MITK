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
#ifndef MATRIX4_H
#define MATRIX4_H

#include "VmUtil.h"
#include "Quat4.h"      // not very good. use temlate export later
#include "AxisAngle4.h"
#include "Matrix4_.h"
#include "Matrix3.h"   // not very good. use template export later

VM_BEGIN_NS

template<class T>
Matrix4<T>::Matrix4(T n00, T n01, T n02, T n03, 
                    T n10, T n11, T n12, T n13,
                    T n20, T n21, T n22, T n23,
                    T n30, T n31, T n32, T n33):
    m00(n00), m01(n01), m02(n02), m03(n03),
    m10(n10), m11(n11), m12(n12), m13(n13),
    m20(n20), m21(n21), m22(n22), m23(n23),
    m30(n30), m31(n31), m32(n32), m33(n33) { }

template<class T>
Matrix4<T>::Matrix4(const T v[]):
    m00(v[ 0]), m01(v[ 1]), m02(v[ 2]), m03(v[ 3]),
    m10(v[ 4]), m11(v[ 5]), m12(v[ 6]), m13(v[ 7]),
    m20(v[ 8]), m21(v[ 9]), m22(v[10]), m23(v[11]),
    m30(v[12]), m31(v[13]), m32(v[14]), m33(v[15]) { }

#ifdef VM_INCLUDE_CONVERSION_FROM_2DARRAY
template<class T>
Matrix4<T>::Matrix4(const T v[][4]):
    m00(v[0][0]), m01(v[0][1]), m02(v[0][2]), m03(v[0][3]),
    m10(v[1][0]), m11(v[1][1]), m12(v[1][2]), m13(v[1][3]),
    m20(v[2][0]), m21(v[2][1]), m22(v[2][2]), m23(v[2][3]),
    m30(v[3][0]), m31(v[3][1]), m32(v[3][2]), m33(v[3][3]) { }
#endif


template<class T>
Matrix4<T>::Matrix4():
    m00(0), m01(0), m02(0), m03(0),
    m10(0), m11(0), m12(0), m13(0),
    m20(0), m21(0), m22(0), m23(0),
    m30(0), m31(0), m32(0), m33(0) { }


template<class T>
void Matrix4<T>::set(T n00, T n01, T n02, T n03, 
                T n10, T n11, T n12, T n13,
                T n20, T n21, T n22, T n23,
                T n30, T n31, T n32, T n33) {
    m00 = n00; m01 = n01; m02 = n02; m03 = n03;
    m10 = n10; m11 = n11; m12 = n12; m13 = n13;
    m20 = n20; m21 = n21; m22 = n22; m23 = n23;
    m30 = n30; m31 = n31; m32 = n32; m33 = n33;
}

template<class T>
void Matrix4<T>::set(const Matrix4& m1) {
    m00 = m1.m00; m01 = m1.m01; m02 = m1.m02; m03 = m1.m03;
    m10 = m1.m10; m11 = m1.m11; m12 = m1.m12; m13 = m1.m13;
    m20 = m1.m20; m21 = m1.m21; m22 = m1.m22; m23 = m1.m23;
    m30 = m1.m30; m31 = m1.m31; m32 = m1.m32; m33 = m1.m33;
}

template<class T>
void Matrix4<T>::set(const T m[]) {
    assert(m != 0);
    m00 = m[ 0]; m01 = m[ 1]; m02 = m[ 2]; m03 = m[ 3];
    m10 = m[ 4]; m11 = m[ 5]; m12 = m[ 6]; m13 = m[ 7];
    m20 = m[ 8]; m21 = m[ 9]; m22 = m[10]; m23 = m[11];
    m30 = m[12]; m31 = m[13]; m32 = m[14]; m33 = m[15];
}

#ifdef VM_INCLUDE_CONVERSION_FROM_2DARRAY
template<class T>
void Matrix4<T>::set(const T m[][4]) {
    assert(m != 0);
    m00 = v[0][0]; m01 = v[0][1]; m02 = v[0][2]; m03 = v[0][3];
    m10 = v[1][0]; m11 = v[1][1]; m12 = v[1][2]; m13 = v[1][3];
    m20 = v[2][0]; m21 = v[2][1]; m22 = v[2][2]; m23 = v[2][3];
    m30 = v[3][0]; m31 = v[3][1]; m32 = v[3][2]; m33 = v[3][3];
}
#endif

template<class T>
void Matrix4<T>::set(const Matrix3<T>& m1)  {
    m00 = m1.m00; m01 = m1.m01; m02 = m1.m02; m03 = 0.0;
    m10 = m1.m10; m11 = m1.m11; m12 = m1.m12; m13 = 0.0;
    m20 = m1.m20; m21 = m1.m21; m22 = m1.m22; m23 = 0.0;
    m30 =    0.0; m31 =    0.0; m32 =    0.0; m33 = 1.0;
}


template<class T>
void Matrix4<T>::set(const Quat4<T>& q1) {
    setFromQuat(q1.x, q1.y, q1.z, q1.w);
}


template<class T>
void Matrix4<T>::set(const AxisAngle4<T>& a1) {
    setFromAxisAngle(a1.x, a1.y, a1.z, a1.angle);
}

template<class T>
void Matrix4<T>::setIdentity() {
    m00 = 1.0; m01 = 0.0; m02 = 0.0; m03 = 0.0;
    m10 = 0.0; m11 = 1.0; m12 = 0.0; m13 = 0.0;
    m20 = 0.0; m21 = 0.0; m22 = 1.0; m23 = 0.0;
    m30 = 0.0; m31 = 0.0; m32 = 0.0; m33 = 1.0;
}

template<class T>
Matrix4<T>::Matrix4(const Quat4<T>& q1, const Vector3<T>& t1, T s) {
    set(q1, t1, s);
}

template<class T>
Matrix4<T>::Matrix4(const Matrix3<T>& m1, const Vector3<T>& t1, T s) {
    set(m1, t1, s);
}

template<class T>
void Matrix4<T>::setElement(size_t row, size_t column, T value) {
    assert(row < 4);
    assert(column < 4);
    if (row == 0)
        if (column == 0)
            m00 = value;
        else if (column == 1)
            m01 = value;
        else if (column == 2)
            m02 = value;
        else if (column == 3)
            m03 = value;
        else
            VM_OUT_OF_RANGE;
    else if (row == 1)
        if (column == 0)
            m10 = value;
        else if (column == 1)
            m11 = value;
        else if (column == 2)
            m12 = value;
        else if (column == 3)
            m13 = value;
        else
            VM_OUT_OF_RANGE;
    else if (row == 2)
        if (column == 0)
            m20 = value;
        else if (column == 1)
            m21 = value;
        else if (column == 2)
            m22 = value;
        else if (column == 3)
            m23 = value;
        else
            VM_OUT_OF_RANGE;
    else if (row == 3)
        if (column == 0)
            m30 = value;
        else if (column == 1)
            m31 = value;
        else if (column == 2)
            m32 = value;
        else if (column == 3)
            m33 = value;
        else
            VM_OUT_OF_RANGE;
    else
        VM_OUT_OF_RANGE;
}

template<class T>
T Matrix4<T>::getElement(size_t row, size_t column) const {
    if (row == 0)
        if (column == 0)
            return m00;
        else if (column == 1)
            return m01;
        else if (column == 2)
            return m02;
        else if (column == 3)
            return m03;
        else
            VM_OUT_OF_RANGE;
    else if (row == 1)
        if (column == 0)
            return m10;
        else if (column == 1)
            return m11;
        else if (column == 2)
            return m12;
        else if (column == 3)
            return m13;
        else
            VM_OUT_OF_RANGE;
    else if (row == 2)
        if (column == 0)
            return m20;
        else if (column == 1)
            return m21;
        else if (column == 2)
            return m22;
        else if (column == 3)
            return m23;
        else
            VM_OUT_OF_RANGE;
    else if (row == 3)
        if (column == 0)
            return m30;
        else if (column == 1)
            return m31;
        else if (column == 2)
            return m32;
        else if (column == 3)
            return m33;
        else
            VM_OUT_OF_RANGE;
    else
        VM_OUT_OF_RANGE;

    // error!
    return 0;
}

template<class T>
T& Matrix4<T>::getElementReference(size_t row, size_t column) {
    if (row == 0)
        if (column == 0)
            return m00;
        else if (column == 1)
            return m01;
        else if (column == 2)
            return m02;
        else if (column == 3)
            return m03;
        else
            VM_OUT_OF_RANGE;
    else if (row == 1)
        if (column == 0)
            return m10;
        else if (column == 1)
            return m11;
        else if (column == 2)
            return m12;
        else if (column == 3)
            return m13;
        else
            VM_OUT_OF_RANGE;
    else if (row == 2)
        if (column == 0)
            return m20;
        else if (column == 1)
            return m21;
        else if (column == 2)
            return m22;
        else if (column == 3)
            return m23;
        else
            VM_OUT_OF_RANGE;
    else if (row == 3)
        if (column == 0)
            return m30;
        else if (column == 1)
            return m31;
        else if (column == 2)
            return m32;
        else if (column == 3)
            return m33;
        else
            VM_OUT_OF_RANGE;
    else
        VM_OUT_OF_RANGE;

    // error anyway
    static T dummy;
    return dummy;
}



template<class T>
void Matrix4<T>::get(Matrix3<T>* m1) const {
    assert(m1 != 0);
    SVD(m1, 0);
}


template<class T>
T Matrix4<T>::get(Matrix3<T>* m1, Vector3<T>* t1) const {
    assert(m1 != 0);
    assert(t1 != 0);
	get(t1);
	return SVD(m1, 0);
}

template<class T>
void Matrix4<T>::get(Quat4<T>* q1) const {
    assert(q1 !=  0);

    q1->set(*this);
    q1->normalize();
}

template<class T>
void Matrix4<T>::getRotationScale(Matrix3<T>* m1) const {
    assert(m1 != 0);
    m1->m00 = m00; m1->m01 = m01; m1->m02 = m02;
    m1->m10 = m10; m1->m11 = m11; m1->m12 = m12;
    m1->m20 = m20; m1->m21 = m21; m1->m22 = m22;
}

template<class T>
T Matrix4<T>::getScale() const {
    return SVD(0, 0);
}

template<class T>
void Matrix4<T>::setRotationScale(const Matrix3<T>& m1) {
    m00 = m1.m00; m01 = m1.m01; m02 = m1.m02;
    m10 = m1.m10; m11 = m1.m11; m12 = m1.m12;
    m20 = m1.m20; m21 = m1.m21; m22 = m1.m22;
}

template<class T>
void Matrix4<T>::setScale(T scale) {
    // note: this don't change m44.
    SVD(0, this);
    mulRotationScale(scale);
}

template<class T>
void Matrix4<T>::setRow(size_t row, T x, T y, T z, T w) {
    assert(row < 4);
	if (row == 0) {
	    m00 = x;
	    m01 = y;
	    m02 = z;
	    m03 = w;
	} else if (row == 1) {
	    m10 = x;
	    m11 = y;
	    m12 = z;
	    m13 = w;
	} else if (row == 2) {
	    m20 = x;
	    m21 = y;
	    m22 = z;
	    m23 = w;
	} else if (row == 3) {
	    m30 = x;
	    m31 = y;
	    m32 = z;
	    m33 = w;
	} else {
        VM_OUT_OF_RANGE;
	}
}

template<class T>
void Matrix4<T>::setRow(size_t row, const Vector4<T>& v) {
    assert(row < 4);
	if (row == 0) {
	    m00 = v.x;
	    m01 = v.y;
	    m02 = v.z;
	    m03 = v.w;
	} else if (row == 1) {
	    m10 = v.x;
	    m11 = v.y;
	    m12 = v.z;
	    m13 = v.w;
	} else if (row == 2) {
	    m20 = v.x;
	    m21 = v.y;
	    m22 = v.z;
	    m23 = v.w;
	} else if (row == 3) {
	    m30 = v.x;
	    m31 = v.y;
	    m32 = v.z;
	    m33 = v.w;
	} else {
        VM_OUT_OF_RANGE;
	}
}

template<class T>
void Matrix4<T>::setRow(size_t row, const T v[]) {
	if (row == 0) {
	    m00 = v[0];
	    m01 = v[1];
	    m02 = v[2];
	    m03 = v[3];
	} else if (row == 1) {
	    m10 = v[0];
	    m11 = v[1];
	    m12 = v[2];
	    m13 = v[3];
	} else if (row == 2) {
	    m20 = v[0];
	    m21 = v[1];
	    m22 = v[2];
	    m23 = v[3];
	} else if (row == 3) {
	    m30 = v[0];
	    m31 = v[1];
	    m32 = v[2];
	    m33 = v[3];
	} else {
        VM_OUT_OF_RANGE;
	}
}

template<class T>
void Matrix4<T>::getRow(size_t row, Vector4<T>* v) const {
    assert(row < 4);
    assert(v != 0);
	if (row == 0) {
	    v->x = m00;
	    v->y = m01;
	    v->z = m02;
	    v->w = m03;
	} else if (row == 1) {
	    v->x = m10;
	    v->y = m11;
	    v->z = m12;
	    v->w = m13;
	} else if (row == 2) {
	    v->x = m20;
	    v->y = m21;
	    v->z = m22;
	    v->w = m23;
	} else if (row == 3) {
	    v->x = m30;
	    v->y = m31;
	    v->z = m32;
	    v->w = m33;
	} else {
        VM_OUT_OF_RANGE;
	}
}

template<class T>
void Matrix4<T>::getRow(size_t row, T v[]) const {
    assert(row < 4);
	if (row == 0) {
	    v[0] = m00;
	    v[1] = m01;
	    v[2] = m02;
	    v[3] = m03;
	} else if (row == 1) {
	    v[0] = m10;
	    v[1] = m11;
	    v[2] = m12;
	    v[3] = m13;
	} else if (row == 2) {
	    v[0] = m20;
	    v[1] = m21;
	    v[2] = m22;
	    v[3] = m23;
	} else if (row == 3) {
	    v[0] = m30;
	    v[1] = m31;
	    v[2] = m32;
	    v[3] = m33;
	} else {
        VM_OUT_OF_RANGE;
	}
}

template<class T>
void Matrix4<T>::setColumn(size_t column, T x, T y, T z, T w) {
    assert(column < 4);
	if (column == 0) {
	    m00 = x;
	    m10 = y;
	    m20 = z;
	    m30 = w;
	}  else if (column == 1) {
	    m01 = x;
	    m11 = y;
	    m21 = z;
	    m31 = w;
	} else if (column == 2) {
	    m02 = x;
	    m12 = y;
	    m22 = z;
	    m32 = w;
	} else if (column == 3) {
	    m03 = x;
	    m13 = y;
	    m23 = z;
	    m33 = w;
	} else {
        VM_OUT_OF_RANGE;
	}
}

template<class T>
void Matrix4<T>::setColumn(size_t column, const Vector4<T>& v) {
	if (column == 0) {
	    m00 = v.x;
	    m10 = v.y;
	    m20 = v.z;
	    m30 = v.w;
	} else if (column == 1) {
	    m01 = v.x;
	    m11 = v.y;
	    m21 = v.z;
	    m31 = v.w;
	} else if (column == 2) {
	    m02 = v.x;
	    m12 = v.y;
	    m22 = v.z;
	    m32 = v.w;
	} else if (column == 3) {
	    m03 = v.x;
	    m13 = v.y;
	    m23 = v.z;
	    m33 = v.w;
	} else {
        VM_OUT_OF_RANGE;
	}
}

template<class T>
void Matrix4<T>::setColumn(size_t column,  const T v[]) {
	if (column == 0) {
	    m00 = v[0];
	    m10 = v[1];
	    m20 = v[2];
	    m30 = v[3];
	} else if (column == 1) {
	    m01 = v[0];
	    m11 = v[1];
	    m21 = v[2];
	    m31 = v[3];
	} else if (column == 2) {
	    m02 = v[0];
	    m12 = v[1];
	    m22 = v[2];
	    m32 = v[3];
	} else if (column == 3) {
	    m03 = v[0];
	    m13 = v[1];
	    m23 = v[2];
	    m33 = v[3];
	} else {
        VM_OUT_OF_RANGE;
	}
}

template<class T>
void Matrix4<T>::getColumn(size_t column, Vector4<T>* v) const {
    assert(column < 4);
    assert(v != 0);
	if (column == 0) {
	    v->x = m00;
	    v->y = m10;
	    v->z = m20;
	    v->w = m30;
	} else if (column == 1) {
	    v->x = m01;
	    v->y = m11;
	    v->z = m21;
	    v->w = m31;
	} else if (column == 2) {
	    v->x = m02;
	    v->y = m12;
	    v->z = m22;
	    v->w = m32;
	} else if (column == 3) {
	    v->x = m03;
	    v->y = m13;
	    v->z = m23;
	    v->w = m33;
	} else {
        VM_OUT_OF_RANGE;
	}
}

template<class T>
void Matrix4<T>::getColumn(size_t column, T v[]) const {
    assert(column < 4);
    assert(v != 0);
	if (column == 0) {
	    v[0] = m00;
	    v[1] = m10;
	    v[2] = m20;
	    v[3] = m30;
	} else if (column == 1) {
	    v[0] = m01;
	    v[1] = m11;
	    v[2] = m21;
	    v[3] = m31;
	} else if (column == 2) {
	    v[0] = m02;
	    v[1] = m12;
	    v[2] = m22;
	    v[3] = m32;
	} else if (column == 3) {
	    v[0] = m03;
	    v[1] = m13;
	    v[2] = m23;
	    v[3] = m33;
	} else {
        VM_OUT_OF_RANGE;
	}
}

template<class T>
void Matrix4<T>::add(T scalar) {
    m00 += scalar; m01 += scalar; m02 += scalar; m03 += scalar;
    m10 += scalar; m11 += scalar; m12 += scalar; m13 += scalar;
    m20 += scalar; m21 += scalar; m22 += scalar; m23 += scalar;
    m30 += scalar; m31 += scalar; m32 += scalar; m33 += scalar;
}

template<class T>
void Matrix4<T>::sub(T scalar) {
    add(-scalar);
}

template<class T>
void Matrix4<T>::add(const Matrix4& m1, const Matrix4& m2) {
	// note this is alias safe.
	set(
	    m1.m00 + m2.m00,
	    m1.m01 + m2.m01,
	    m1.m02 + m2.m02,
	    m1.m03 + m2.m03,
	    m1.m10 + m2.m10,
	    m1.m11 + m2.m11,
	    m1.m12 + m2.m12,
	    m1.m13 + m2.m13,
	    m1.m20 + m2.m20,
	    m1.m21 + m2.m21,
	    m1.m22 + m2.m22,
	    m1.m23 + m2.m23,
	    m1.m30 + m2.m30,
	    m1.m31 + m2.m31,
	    m1.m32 + m2.m32,
	    m1.m33 + m2.m33
	    );
}

template<class T>
void Matrix4<T>::add(const Matrix4& m1) {
    m00 += m1.m00; m01 += m1.m01; m02 += m1.m02; m03 += m1.m03;
    m10 += m1.m10; m11 += m1.m11; m12 += m1.m12; m13 += m1.m13;
    m20 += m1.m20; m21 += m1.m21; m22 += m1.m22; m23 += m1.m23;
    m30 += m1.m30; m31 += m1.m31; m32 += m1.m32; m33 += m1.m33;
}


template<class T>
void Matrix4<T>::sub(const Matrix4& m1, const Matrix4& m2) {
	// note this is alias safe.
	set(
	    m1.m00 - m2.m00,
	    m1.m01 - m2.m01,
	    m1.m02 - m2.m02,
	    m1.m03 - m2.m03,
	    m1.m10 - m2.m10,
	    m1.m11 - m2.m11,
	    m1.m12 - m2.m12,
	    m1.m13 - m2.m13,
	    m1.m20 - m2.m20,
	    m1.m21 - m2.m21,
	    m1.m22 - m2.m22,
	    m1.m23 - m2.m23,
	    m1.m30 - m2.m30,
	    m1.m31 - m2.m31,
	    m1.m32 - m2.m32,
	    m1.m33 - m2.m33
	    );
    }

template<class T>
void Matrix4<T>::sub(const Matrix4& m1) {
    m00 -= m1.m00; m01 -= m1.m01; m02 -= m1.m02; m03 -= m1.m03;
    m10 -= m1.m10; m11 -= m1.m11; m12 -= m1.m12; m13 -= m1.m13;
    m20 -= m1.m20; m21 -= m1.m21; m22 -= m1.m22; m23 -= m1.m23;
    m30 -= m1.m30; m31 -= m1.m31; m32 -= m1.m32; m33 -= m1.m33;
}

template<class T>
void Matrix4<T>::transpose() {
    T tmp = m01; m01 = m10; m10 = tmp;
    tmp = m02;	m02 = m20;	m20 = tmp;
    tmp = m03;	m03 = m30;	m30 = tmp;
    tmp = m12;	m12 = m21;	m21 = tmp;
    tmp = m13;	m13 = m31;	m31 = tmp;
    tmp = m23;	m23 = m32;	m32 = tmp;
}


template<class T>
void Matrix4<T>::set(const Quat4<T>& q1, const Vector3<T>& t1, T s) {
    set(q1);
    mulRotationScale(s);
    m03 = t1.x;
    m13 = t1.y;
    m23 = t1.z;
}


template<class T>
void Matrix4<T>::invert(const Matrix4<T>& m1) {
    set(m1);
    invert();
}


template<class T>
void Matrix4<T>::invert() {
	T s = determinant();
	if (s == 0.0)
	    return;
	s = 1/s;
	// alias-safe way.
	// less *,+,- calculation than expanded expression.
	set(
	    m11*(m22*m33 - m23*m32) + m12*(m23*m31 - m21*m33) + m13*(m21*m32 - m22*m31),
	    m21*(m02*m33 - m03*m32) + m22*(m03*m31 - m01*m33) + m23*(m01*m32 - m02*m31),
	    m31*(m02*m13 - m03*m12) + m32*(m03*m11 - m01*m13) + m33*(m01*m12 - m02*m11),
	    m01*(m13*m22 - m12*m23) + m02*(m11*m23 - m13*m21) + m03*(m12*m21 - m11*m22),

	    m12*(m20*m33 - m23*m30) + m13*(m22*m30 - m20*m32) + m10*(m23*m32 - m22*m33),
	    m22*(m00*m33 - m03*m30) + m23*(m02*m30 - m00*m32) + m20*(m03*m32 - m02*m33),
	    m32*(m00*m13 - m03*m10) + m33*(m02*m10 - m00*m12) + m30*(m03*m12 - m02*m13),
	    m02*(m13*m20 - m10*m23) + m03*(m10*m22 - m12*m20) + m00*(m12*m23 - m13*m22),

	    m13*(m20*m31 - m21*m30) + m10*(m21*m33 - m23*m31) + m11*(m23*m30 - m20*m33),
	    m23*(m00*m31 - m01*m30) + m20*(m01*m33 - m03*m31) + m21*(m03*m30 - m00*m33),
	    m33*(m00*m11 - m01*m10) + m30*(m01*m13 - m03*m11) + m31*(m03*m10 - m00*m13),
	    m03*(m11*m20 - m10*m21) + m00*(m13*m21 - m11*m23) + m01*(m10*m23 - m13*m20),

	    m10*(m22*m31 - m21*m32) + m11*(m20*m32 - m22*m30) + m12*(m21*m30 - m20*m31),
	    m20*(m02*m31 - m01*m32) + m21*(m00*m32 - m02*m30) + m22*(m01*m30 - m00*m31),
	    m30*(m02*m11 - m01*m12) + m31*(m00*m12 - m02*m10) + m32*(m01*m10 - m00*m11),
	    m00*(m11*m22 - m12*m21) + m01*(m12*m20 - m10*m22) + m02*(m10*m21 - m11*m20)
	    );

	mul(s);
}

template<class T>
T Matrix4<T>::determinant() const {
	// less *,+,- calculation than expanded expression.
	return
	    (m00*m11 - m01*m10)*(m22*m33 - m23*m32)
	   -(m00*m12 - m02*m10)*(m21*m33 - m23*m31)
	   +(m00*m13 - m03*m10)*(m21*m32 - m22*m31)
	   +(m01*m12 - m02*m11)*(m20*m33 - m23*m30)
	   -(m01*m13 - m03*m11)*(m20*m32 - m22*m30)
	   +(m02*m13 - m03*m12)*(m20*m31 - m21*m30);
}

template<class T>
void Matrix4<T>::set(T scale) {
    m00 = scale; m01 = 0.0;   m02 = 0.0;   m03 = 0.0;
    m10 = 0.0;   m11 = scale; m12 = 0.0;   m13 = 0.0;
    m20 = 0.0;   m21 = 0.0;   m22 = scale; m23 = 0.0;
    m30 = 0.0;   m31 = 0.0;   m32 = 0.0;   m33 = 1.0;
}

template<class T>
void Matrix4<T>::set(const Vector3<T>& v1, T scale) {
	m00 = scale; m01 = 0.0;   m02 = 0.0;   m03 = scale*v1.x;
	m10 = 0.0;   m11 = scale; m12 = 0.0;   m13 = scale*v1.y;
	m20 = 0.0;   m21 = 0.0;   m22 = scale; m23 = scale*v1.z;
	m30 = 0.0;   m31 = 0.0;   m32 = 0.0;   m33 = 1.0;
}

template<class T>
void Matrix4<T>::set(const Matrix3<T>& m1, const Vector3<T>& t1, T scale) {
	setRotationScale(m1);
	mulRotationScale(scale);
	setTranslation(t1);
	m33 = 1.0;
}

template<class T>
void Matrix4<T>::rotX(T angle)  {
	T c = VmUtil<T>::cos(angle);
	T s = VmUtil<T>::sin(angle);
	m00 = 1.0; m01 = 0.0; m02 = 0.0; m03 = 0.0;
	m10 = 0.0; m11 = c;   m12 = -s;  m13 = 0.0;
	m20 = 0.0; m21 = s;   m22 = c;   m23 = 0.0;
	m30 = 0.0; m31 = 0.0; m32 = 0.0; m33 = 1.0; 
}


template<class T>
void Matrix4<T>::rotY(T angle)  {
	T c = VmUtil<T>::cos(angle);
	T s = VmUtil<T>::sin(angle);
	m00 = c;   m01 = 0.0; m02 = s;   m03 = 0.0;
	m10 = 0.0; m11 = 1.0; m12 = 0.0; m13 = 0.0;
	m20 = -s;  m21 = 0.0; m22 = c;   m23 = 0.0;
	m30 = 0.0; m31 = 0.0; m32 = 0.0; m33 = 1.0; 
}


template<class T>
void Matrix4<T>::rotZ(T angle)  {
	T c = VmUtil<T>::cos(angle);
	T s = VmUtil<T>::sin(angle);
	m00 = c;   m01 = -s;  m02 = 0.0; m03 = 0.0;
	m10 = s;   m11 = c;   m12 = 0.0; m13 = 0.0;
	m20 = 0.0; m21 = 0.0; m22 = 1.0; m23 = 0.0;
	m30 = 0.0; m31 = 0.0; m32 = 0.0; m33 = 1.0; 
}


template<class T>
void Matrix4<T>::mul(T scalar) {
	m00 *= scalar; m01 *= scalar;  m02 *= scalar; m03 *= scalar;
	m10 *= scalar; m11 *= scalar;  m12 *= scalar; m13 *= scalar;
	m20 *= scalar; m21 *= scalar;  m22 *= scalar; m23 *= scalar;
	m30 *= scalar; m31 *= scalar;  m32 *= scalar; m33 *= scalar;
}


template<class T>
void Matrix4<T>::mul(const Matrix4& m1, const Matrix4& m2) {
	// alias-safe way
	set(
	    m1.m00*m2.m00 + m1.m01*m2.m10 + m1.m02*m2.m20 + m1.m03*m2.m30,
	    m1.m00*m2.m01 + m1.m01*m2.m11 + m1.m02*m2.m21 + m1.m03*m2.m31,
	    m1.m00*m2.m02 + m1.m01*m2.m12 + m1.m02*m2.m22 + m1.m03*m2.m32,
	    m1.m00*m2.m03 + m1.m01*m2.m13 + m1.m02*m2.m23 + m1.m03*m2.m33,

	    m1.m10*m2.m00 + m1.m11*m2.m10 + m1.m12*m2.m20 + m1.m13*m2.m30,
	    m1.m10*m2.m01 + m1.m11*m2.m11 + m1.m12*m2.m21 + m1.m13*m2.m31,
	    m1.m10*m2.m02 + m1.m11*m2.m12 + m1.m12*m2.m22 + m1.m13*m2.m32,
	    m1.m10*m2.m03 + m1.m11*m2.m13 + m1.m12*m2.m23 + m1.m13*m2.m33,

	    m1.m20*m2.m00 + m1.m21*m2.m10 + m1.m22*m2.m20 + m1.m23*m2.m30,
	    m1.m20*m2.m01 + m1.m21*m2.m11 + m1.m22*m2.m21 + m1.m23*m2.m31,
	    m1.m20*m2.m02 + m1.m21*m2.m12 + m1.m22*m2.m22 + m1.m23*m2.m32,
	    m1.m20*m2.m03 + m1.m21*m2.m13 + m1.m22*m2.m23 + m1.m23*m2.m33,

	    m1.m30*m2.m00 + m1.m31*m2.m10 + m1.m32*m2.m20 + m1.m33*m2.m30,
	    m1.m30*m2.m01 + m1.m31*m2.m11 + m1.m32*m2.m21 + m1.m33*m2.m31,
	    m1.m30*m2.m02 + m1.m31*m2.m12 + m1.m32*m2.m22 + m1.m33*m2.m32,
	    m1.m30*m2.m03 + m1.m31*m2.m13 + m1.m32*m2.m23 + m1.m33*m2.m33
	    );
}

template<class T>
void Matrix4<T>::mulTransposeRight(const Matrix4& m1, const Matrix4& m2) {
	// alias-safe way.
	set(
	    m1.m00*m2.m00 + m1.m01*m2.m01 + m1.m02*m2.m02 + m1.m03*m2.m03,
	    m1.m00*m2.m10 + m1.m01*m2.m11 + m1.m02*m2.m12 + m1.m03*m2.m13,
	    m1.m00*m2.m20 + m1.m01*m2.m21 + m1.m02*m2.m22 + m1.m03*m2.m23,
	    m1.m00*m2.m30 + m1.m01*m2.m31 + m1.m02*m2.m32 + m1.m03*m2.m33,

	    m1.m10*m2.m00 + m1.m11*m2.m01 + m1.m12*m2.m02 + m1.m13*m2.m03,
	    m1.m10*m2.m10 + m1.m11*m2.m11 + m1.m12*m2.m12 + m1.m13*m2.m13,
	    m1.m10*m2.m20 + m1.m11*m2.m21 + m1.m12*m2.m22 + m1.m13*m2.m23,
	    m1.m10*m2.m30 + m1.m11*m2.m31 + m1.m12*m2.m32 + m1.m13*m2.m33,

	    m1.m20*m2.m00 + m1.m21*m2.m01 + m1.m22*m2.m02 + m1.m23*m2.m03,
	    m1.m20*m2.m10 + m1.m21*m2.m11 + m1.m22*m2.m12 + m1.m23*m2.m13,
	    m1.m20*m2.m20 + m1.m21*m2.m21 + m1.m22*m2.m22 + m1.m23*m2.m23,
	    m1.m20*m2.m30 + m1.m21*m2.m31 + m1.m22*m2.m32 + m1.m23*m2.m33,
	    
	    m1.m30*m2.m00 + m1.m31*m2.m01 + m1.m32*m2.m02 + m1.m33*m2.m03,
	    m1.m30*m2.m10 + m1.m31*m2.m11 + m1.m32*m2.m12 + m1.m33*m2.m13,
	    m1.m30*m2.m20 + m1.m31*m2.m21 + m1.m32*m2.m22 + m1.m33*m2.m23,
	    m1.m30*m2.m30 + m1.m31*m2.m31 + m1.m32*m2.m32 + m1.m33*m2.m33
	    );
}


template<class T>
void Matrix4<T>::mulTransposeLeft(const Matrix4& m1, const Matrix4& m2) {
	// alias-safe way.
	set(
	    m1.m00*m2.m00 + m1.m10*m2.m10 + m1.m20*m2.m20 + m1.m30*m2.m30,
	    m1.m00*m2.m01 + m1.m10*m2.m11 + m1.m20*m2.m21 + m1.m30*m2.m31,
	    m1.m00*m2.m02 + m1.m10*m2.m12 + m1.m20*m2.m22 + m1.m30*m2.m32,
	    m1.m00*m2.m03 + m1.m10*m2.m13 + m1.m20*m2.m23 + m1.m30*m2.m33,

	    m1.m01*m2.m00 + m1.m11*m2.m10 + m1.m21*m2.m20 + m1.m31*m2.m30,
	    m1.m01*m2.m01 + m1.m11*m2.m11 + m1.m21*m2.m21 + m1.m31*m2.m31,
	    m1.m01*m2.m02 + m1.m11*m2.m12 + m1.m21*m2.m22 + m1.m31*m2.m32,
	    m1.m01*m2.m03 + m1.m11*m2.m13 + m1.m21*m2.m23 + m1.m31*m2.m33,

	    m1.m02*m2.m00 + m1.m12*m2.m10 + m1.m22*m2.m20 + m1.m32*m2.m30,
	    m1.m02*m2.m01 + m1.m12*m2.m11 + m1.m22*m2.m21 + m1.m32*m2.m31,
	    m1.m02*m2.m02 + m1.m12*m2.m12 + m1.m22*m2.m22 + m1.m32*m2.m32,
	    m1.m02*m2.m03 + m1.m12*m2.m13 + m1.m22*m2.m23 + m1.m32*m2.m33,

	    m1.m03*m2.m00 + m1.m13*m2.m10 + m1.m23*m2.m20 + m1.m33*m2.m30,
	    m1.m03*m2.m01 + m1.m13*m2.m11 + m1.m23*m2.m21 + m1.m33*m2.m31,
	    m1.m03*m2.m02 + m1.m13*m2.m12 + m1.m23*m2.m22 + m1.m33*m2.m32,
	    m1.m03*m2.m03 + m1.m13*m2.m13 + m1.m23*m2.m23 + m1.m33*m2.m33
	    );
}

template<class T>
bool Matrix4<T>::equals(const Matrix4<T>& m1) const {
	return  m00 == m1.m00
		&& m01 == m1.m01
		&& m02 == m1.m02 
		&& m03 == m1.m03
		&& m10 == m1.m10
		&& m11 == m1.m11
		&& m12 == m1.m12
		&& m13 == m1.m13
		&& m20 == m1.m20
		&& m21 == m1.m21
		&& m22 == m1.m22
		&& m23 == m1.m23
		&& m30 == m1.m30
		&& m31 == m1.m31
		&& m32 == m1.m32
		&& m33 == m1.m33;
}

template<class T>
bool Matrix4<T>::epsilonEquals(const Matrix4<T>& m1, T epsilon) const {
	  return  abs(m00 - m1.m00) <= epsilon
		&& abs(m01 - m1.m01) <= epsilon
		&& abs(m02 - m1.m02 ) <= epsilon
		&& abs(m03 - m1.m03) <= epsilon

		&& abs(m10 - m1.m10) <= epsilon
		&& abs(m11 - m1.m11) <= epsilon
		&& abs(m12 - m1.m12) <= epsilon
		&& abs(m13 - m1.m13) <= epsilon

		&& abs(m20 - m1.m20) <= epsilon
		&& abs(m21 - m1.m21) <= epsilon
		&& abs(m22 - m1.m22) <= epsilon
		&& abs(m23 - m1.m23) <= epsilon

		&& abs(m30 - m1.m30) <= epsilon
		&& abs(m31 - m1.m31) <= epsilon
		&& abs(m32 - m1.m32) <= epsilon
        && abs(m33 - m1.m33) <= epsilon;
}



template<class T>
void Matrix4<T>::transform(const Tuple4<T>& vec, Tuple4<T>* vecOut) const {
	// alias-safe
    assert(vecOut != 0);
	vecOut->set(
	    m00*vec.x + m01*vec.y + m02*vec.z + m03*vec.w,
	    m10*vec.x + m11*vec.y + m12*vec.z + m13*vec.w,
	    m20*vec.x + m21*vec.y + m22*vec.z + m23*vec.w,
	    m30*vec.x + m31*vec.y + m32*vec.z + m33*vec.w
	    );
}


template<class T>
void Matrix4<T>::transform(const Point3<T>& point, Point3<T>* pointOut) const {
    assert(pointOut != 0);
	pointOut->set(
	    m00*point.x + m01*point.y + m02*point.z + m03,
	    m10*point.x + m11*point.y + m12*point.z + m13,
	    m20*point.x + m21*point.y + m22*point.z + m23
	    );
}


template<class T>
void Matrix4<T>::transform(const Vector3<T>& normal, Vector3<T>* normalOut) const {
	normalOut->set(
	    m00 * normal.x + m01 * normal.y + m02 * normal.z,
	    m10 * normal.x + m11 * normal.y + m12 * normal.z,
	    m20 * normal.x + m21 * normal.y + m22 * normal.z
	    );
}


template<class T>
void Matrix4<T>::setRotation(const Matrix3<T>& m1) {
	T scale = SVD(0, 0);
	setRotationScale(m1);
	mulRotationScale(scale);
}


template<class T>
void Matrix4<T>::setRotation(const Quat4<T>& q1) {
	T scale = SVD(0, 0);
	// save other values
	T tx = m03; 
	T ty = m13; 
	T tz = m23; 
	T w0 = m30;                  
	T w1 = m31;
	T w2 = m32;
	T w3 = m33;

	set(q1);
	mulRotationScale(scale);

	// set back
	m03 = tx;
	m13 = ty;
	m23 = tz;
	m30 = w0;
	m31 = w1;
	m32 = w2;
	m33 = w3;
}


template<class T>
void Matrix4<T>::setRotation(const AxisAngle4<T>& a1) {
	T scale = SVD(0, 0);
	// save other values
	T tx = m03; 
	T ty = m13; 
	T tz = m23; 
	T w0 = m30;                  
	T w1 = m31;
	T w2 = m32;
	T w3 = m33;

	set(a1);
	mulRotationScale(scale);

	// set back
	m03 = tx;
	m13 = ty;
	m23 = tz;
	m30 = w0;
	m31 = w1;
	m32 = w2;
	m33 = w3;
}


template<class T>
void Matrix4<T>::setZero() {
    m00 = 0.0; m01 = 0.0; m02 = 0.0; m03 = 0.0;
    m10 = 0.0; m11 = 0.0; m12 = 0.0; m13 = 0.0;
    m20 = 0.0; m21 = 0.0; m22 = 0.0; m23 = 0.0;
    m30 = 0.0; m31 = 0.0; m32 = 0.0; m33 = 0.0;
}


template<class T>
void Matrix4<T>::negate() {
    m00 = -m00; m01 = -m01; m02 = -m02; m03 = -m03;
    m10 = -m10; m11 = -m11; m12 = -m12; m13 = -m13;
    m20 = -m20; m21 = -m21; m22 = -m22; m23 = -m23;
    m30 = -m30; m31 = -m31; m32 = -m32; m33 = -m33;
}


template<class T>
T Matrix4<T>::SVD(Matrix3<T>* rot3, Matrix4* rot4) const {
	// this is a simple svd.
	// Not complete but fast and reasonable.
	// See comment in Matrix3d.

	T s = VmUtil<T>::sqrt(
	    (
	     m00*m00 + m10*m10 + m20*m20 + 
	     m01*m01 + m11*m11 + m21*m21 +
	     m02*m02 + m12*m12 + m22*m22
	    )/3.0
	    );

	if (rot3) {
	    this->getRotationScale(rot3);
        // zero-div may occur.
        double n = 1/VmUtil<T>::sqrt(m00*m00 + m10*m10 + m20*m20);
        rot3->m00 *= n;
        rot3->m10 *= n;
        rot3->m20 *= n;

        n = 1/VmUtil<T>::sqrt(m01*m01 + m11*m11 + m21*m21);
        rot3->m01 *= n;
        rot3->m11 *= n;
        rot3->m21 *= n;

        n = 1/VmUtil<T>::sqrt(m02*m02 + m12*m12 + m22*m22);
        rot3->m02 *= n;
        rot3->m12 *= n;
        rot3->m22 *= n;
	}

	if (rot4) {
	    if (rot4 != this)
            rot4->setRotationScale(*this);  // private method

        // zero-div may occur.
        double n = 1/VmUtil<T>::sqrt(m00*m00 + m10*m10 + m20*m20);
        rot4->m00 *= n;
        rot4->m10 *= n;
        rot4->m20 *= n;

        n = 1/VmUtil<T>::sqrt(m01*m01 + m11*m11 + m21*m21);
        rot4->m01 *= n;
        rot4->m11 *= n;
        rot4->m21 *= n;

        n = 1/VmUtil<T>::sqrt(m02*m02 + m12*m12 + m22*m22);
        rot4->m02 *= n;
        rot4->m12 *= n;
        rot4->m22 *= n;
	}

	return s;
}


template<class T>
void Matrix4<T>::mulRotationScale(T scale) {
	m00 *= scale; m01 *= scale; m02 *= scale;
	m10 *= scale; m11 *= scale; m12 *= scale;
	m20 *= scale; m21 *= scale; m22 *= scale;
}


template<class T>
void Matrix4<T>::setRotationScale(const Matrix4& m1) {
	m00 = m1.m00; m01 = m1.m01; m02 = m1.m02;
	m10 = m1.m10; m11 = m1.m11; m12 = m1.m12;
	m20 = m1.m20; m21 = m1.m21; m22 = m1.m22;
}


template<class T>
void Matrix4<T>::setFromQuat(T x, T y, T z, T w) {
	T n = x*x + y*y + z*z + w*w;
	T s = (n > 0.0) ? (2.0/n) : 0.0;

	T xs = x*s,  ys = y*s,  zs = z*s;
	T wx = w*xs, wy = w*ys, wz = w*zs;
	T xx = x*xs, xy = x*ys, xz = x*zs;
	T yy = y*ys, yz = y*zs, zz = z*zs;

	setIdentity();
	m00 = 1.0 - (yy + zz);	m01 = xy - wz;         m02 = xz + wy;
	m10 = xy + wz;          m11 = 1.0 - (xx + zz); m12 = yz - wx;
	m20 = xz - wy;          m21 = yz + wx;         m22 = 1.0 - (xx + yy);
}


template<class T>
void Matrix4<T>::setFromAxisAngle(T x, T y, T z, T angle) {
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
VM_STRING_STD::string Matrix4<T>::toString() const {
    VM_TOSTRING
}
#endif /* VM_INCLUDE_TOSTRING */

VM_END_NS

#ifdef VM_INCLUDE_IO
template <class T>
VM_IOSTREAM_STD::ostream& operator<<(VM_IOSTREAM_STD::ostream& o, const VM_VECMATH_NS::Matrix4<T>& t1) {
    return  o << 
    "[ ["<<t1.m00<<","<<t1.m01<<","<<t1.m02<<","<<t1.m03<<"]" << VM_IOSTREAM_STD::endl <<
    "  ["<<t1.m10<<","<<t1.m11<<","<<t1.m12<<","<<t1.m13<<"]" << VM_IOSTREAM_STD::endl <<
    "  ["<<t1.m20<<","<<t1.m21<<","<<t1.m22<<","<<t1.m23<<"]" << VM_IOSTREAM_STD::endl <<
    "  ["<<t1.m30<<","<<t1.m31<<","<<t1.m32<<","<<t1.m33<<"]""] ]";
}

template <class T>
VM_IOSTREAM_STD::istream& operator>>(VM_IOSTREAM_STD::istream& i, const VM_VECMATH_NS::Matrix4<T>& m ) {

	char c;

	i >> c;
	i >> c;
	i >> (T) m.m00;
	i >> c;
	i >> (T) m.m01;
	i >> c;
	i >> (T) m.m02;
	i >> c;
	i >> (T) m.m03;
	i >> c; 
	i>> c;
	i >> (T) m.m10;
	i >> c;
	i >> (T) m.m11;
	i >> c;
	i >> (T) m.m12;
	i >> c;
	i >> (T) m.m13;
	i >> c;
	i >> c;
	i >> (T) m.m20;
	i >> c;
	i >> (T) m.m21;
	i >> c;
	i >> (T) m.m22;
	i >> c;
	i >> (T) m.m23;
	i >> c;
	i >> c;
	i >> (T) m.m30;
	i >> c;
	i >> (T) m.m31;
	i >> c;
	i >> (T) m.m32;
	i >> c;
	i >> (T) m.m33;
	i >> c;
	i >> c;

    return  i; 
}

#endif


#endif /* MATRIX4_H */
