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
#ifndef AXISANGLE4_H
#define AXISANGLE4_H

#include <VmUtil.h>
#include <AxisAngle4_.h>
#include <Quat4_.h>

VM_BEGIN_NS

template<class T>
void AxisAngle4<T>::set(const Quat4<T>& q1) {
    setFromQuat(q1.x, q1.y, q1.z, q1.w);
}

template<class T>
void AxisAngle4<T>::setFromMat(T m00, T m01, T m02,
                               T m10, T m11, T m12,
                               T m20, T m21, T m22) {
	// assuming M is normalized.
	T cos = (m00 + m11 + m22 - 1.0)*0.5;
	x = m21 - m12;
	y = m02 - m20;
	z = m10 - m01;
	T sin = 0.5*VmUtil<T>::sqrt(x*x + y*y + z*z);
	angle = VmUtil<T>::atan2(sin, cos);

	// no need to normalize
	// x /= n;
	// y /= n;
	// z /= n;

}




template<class T>
void AxisAngle4<T>::setFromQuat(T x, T y, T z, T w) {
	// This logic can calculate angle without normalization.
	// The direction of (x,y,z) and the sign of rotation cancel
	// each other to calculate a right answer.

	T sin_a2 = VmUtil<T>::sqrt(x*x + y*y + z*z);  // |sin a/2|, w = cos a/2
	this->angle = 2.0*VmUtil<T>::atan2(sin_a2, w); // 0 <= angle <= PI , because 0 < sin_a2
	this->x = x;
	this->y = y;
	this->z = z;
}

#ifdef VM_INCLUDE_TOSTRING
template<class T>
VM_STRING_STD::string AxisAngle4<T>::toString() const {
    VM_TOSTRING
}
#endif

VM_END_NS

#ifdef VM_INCLUDE_IO
template <class T>
VM_IOSTREAM_STD::ostream& operator<<(VM_IOSTREAM_STD::ostream& o, const VM_VECMATH_NS::AxisAngle4<T>& a1) {
    return o << "(" << a1.x << "," << a1.y << ","
             << a1.z  << "," << a1.angle << ")";
}
#endif

#endif /* AXISANGLE4_H */
