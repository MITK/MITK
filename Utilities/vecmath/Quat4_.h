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
#ifndef QUAT4__H
#define QUAT4__H

#include "VmUtil.h"
#include "Tuple4.h"
#include "Matrix4_.h"

VM_BEGIN_NS

/**
 * A 4 element quaternion represented by x,y,z,w coordinates. 
 * @version specification 1.1, implementation $Revision$, $Date$
 * @author Kenji hiranabe
 */
template<class T>
class Quat4 : public Tuple4<T> {
/*
 * $Log$
 * Revision 1.3  2005/10/06 15:27:15  nolden
 * FIX: gcc4 compatibility
 *
 * Revision 1.2  2003/04/22 14:42:11  max
 * made inclusions of vecmath header files "local" (for gcc 3.3 support)
 *
 * Revision 1.1  2002/11/16 12:25:36  mark
 * vecmath bibliothek
 * in cvs, da einige bugs behoben
 *
 * Revision 1.3  1999/10/06  02:52:46  hiranabe
 * Java3D 1.2 and namespace
 *
 * Revision 1.2  1999/05/26  00:59:37  hiranabe
 * support Visual C++
 *
 * Revision 1.1  1999/03/04  11:07:09  hiranabe
 * Initial revision
 *
 * Revision 1.1  1999/03/04  11:07:09  hiranabe
 * Initial revision
 *
 */
public:
    /**
     * Constructs and initializes a Quat4 from the specified xyzw coordinates.
     * @param x the x coordinate
     * @param y the y coordinate
     * @param z the z coordinate
     * @param w the w scalar component
     */
    Quat4(T x, T y, T z, T w): Tuple4<T>(x, y, z, w) { }
  
    /**
     * Constructs and initializes a Quat4 from the array of length 4.
     * @param v the array of length 4 containing xyzw in order
     */
    Quat4(const T v[]): Tuple4<T>(v) { }


    /**
      * Constructs and initializes a Quat4 from the specified Tuple4d.
      * @param t1 the Tuple4d containing the initialization x y z w data
      */
    Quat4(const Tuple4<T>& t1): Tuple4<T>(t1) { }

#if 0
    /**
     * Constructs and initializes a Quat4 from the specified Quat4f.
     * @param q1 the Quat4f containing the initialization x y z w data
     */
    Quat4(Quat4f q1) {
	super(q1);
    }

    /**
      * Constructs and initializes a Quat4 from the specified Tuple4f.
      * @param t1 the Tuple4f containing the initialization x y z w data
      */
    Quat4(Tuple4f t1) {
	super(t1);
    }
#endif

    /**
     * Constructs and initializes a Quat4 to (0,0,0,0).
     */
    Quat4(): Tuple4<T>() { }

    /**
      * Sets the value of this tuple to the value of tuple t1.
      * note: other set methods hides this set.
      * @param t1 the tuple to be copied
      */
    void set(const Tuple4<T>& t1) {
        Tuple4<T>::set(t1);
    }

    /**
      * Sets the value of this tuple to the specified xyzw coordinates.
      * note: other set methods hides this set.
      * @param x the x coordinate
      * @param y the y coordinate
      * @param z the z coordinate
      * @param w the w coordinate
      */
    void set(T x, T y, T z, T w) {
        Tuple4<T>::set(x, y, z, w);
    }


    /**
     * Sets the value of this quaternion to the conjugate of quaternion q1.
     * @param q1 the source vector
     */
    void conjugate(const Quat4& q1) {
        this->x = -q1.x;
        this->y = -q1.y;
        this->z = -q1.z;
        this->w = q1.w;
    }

    /**
     * Negate the value of of each of this quaternion's x,y,z coordinates 
     *  in place.
     */
    void conjugate() {
        this->x = -this->x;
        this->y = -this->y;
        this->z = -this->z;
    }

    /**
     * Sets the value of this quaternion to the quaternion product of
     * quaternions q1 and q2 (this = q1 * q2).  
     * Note that this is safe for aliasing (e.g. this can be q1 or q2).
     * @param q1 the first quaternion
     * @param q2 the second quaternion
     */
    void mul(const Quat4& q1, const Quat4& q2);

    /**
     * Sets the value of this quaternion to the quaternion product of
     * itself and q1 (this = this * q1).
     * @param q1 the other quaternion
     */
    void mul(const Quat4& q1);

    /**
     *
     * Multiplies quaternion q1 by the inverse of quaternion q2 and places
     * the value into this quaternion.  The value of both argument quaternions 
     * is preservered (this = q1 * q2^-1).
     * @param q1 the left quaternion
     * @param q2 the right quaternion
     */
    void mulInverse(const Quat4& q1, const Quat4& q2);

    /**
     * Multiplies this quaternion by the inverse of quaternion q1 and places
     * the value into this quaternion.  The value of the argument quaternion
     * is preserved (this = this * q^-1).
     * @param q1 the other quaternion
     */
    void mulInverse(const Quat4& q1);

protected:
    T norm() const {
        return this->x*this->x + this->y*this->y + this->z*this->z + this->w*this->w;
    }
    void setFromMat(T m00, T m01, T m02,
		       T m10, T m11, T m12,
		       T m20, T m21, T m22);

public:
    /**
     * Sets the value of this quaternion to quaternion inverse of quaternion q1.
     * @param q1 the quaternion to be inverted
     */
    void inverse(const Quat4& q1) {
        T n = q1.norm();
        // zero-div may occur.
        this->x = -q1.x/n;
        this->y = -q1.y/n;
        this->z = -q1.z/n;
        this->w = q1.w/n;
    }

    /**
     * Sets the value of this quaternion to the quaternion inverse of itself.
     */
    void inverse() {
        T n = norm();
        // zero-div may occur.
        this->x = -this->x/n;
        this->y = -this->y/n;
        this->z = -this->z/n;
        this->w /= n;
    }

    /**
     * Sets the value of this quaternion to the normalized value
     * of quaternion q1.
     * @param q1 the quaternion to be normalized.
     */
    void normalize(const Quat4& q1) {
        T n = VmUtil<T>::sqrt(q1.norm());
        // zero-div may occur.
        this->x = q1.x/n;
        this->y = q1.y/n;
        this->z = q1.z/n;
        this->w = q1.w/n;
    }

    /**
     * Normalizes the value of this quaternion in place.
     */
    void normalize() {
        T n = VmUtil<T>::sqrt(norm());
        // zero-div may occur.
        this->x /= n;
        this->y /= n;
        this->z /= n;
        this->w /= n;
    }

#if 0
    /**
     * Sets the value of this quaternion to the rotational component of
     * the passed matrix.
     * @param m1 the matrix4f
     */
    void set(Matrix4f m1) {
	setFromMat(
	    m1.m00, m1.m01, m1.m02,
	    m1.m10, m1.m11, m1.m12,
	    m1.m20, m1.m21, m1.m22
	    );
    }

    /**
     * Sets the value of this quaternion to the rotational component of
     * the passed matrix.
     * @param m1 the matrix3f
     */
    void set(Matrix3f m1) {
	setFromMat(
	    m1.m00, m1.m01, m1.m02,
	    m1.m10, m1.m11, m1.m12,
	    m1.m20, m1.m21, m1.m22
	    );
    }

    /**
     * Sets the value of this quaternion to the equivalent rotation of teh
     * AxisAngle argument.
     * @param a1 the axis-angle
     */
    void set(AxisAngle4f a1) {
	x = a1.x;
	y = a1.y;
	z = a1.z;
	T n = Math.sqrt(x*x + y*y + z*z);
	// zero-div may occur.
	T s = Math.sin(0.5*a1.angle)/n;
	x *= s;
	y *= s;
	z *= s;
	w = Math.cos(0.5*a1.angle);
    }

#endif

    /**
     * Sets the value of this quaternion to the rotational component of
     * the passed matrix.
     * @param m1 the matrix4d
     */
    void set(const Matrix4<T>& m1);

    /**
     * Sets the value of this quaternion to the rotational component of
     * the passed matrix.
     * @param m1 the matrix3d
     */
    void set(const Matrix3<T>& m1);


    /**
     * Sets the value of this quaternion to the equivalent rotation of teh
     * AxisAngle argument.
     * @param a1 the axis-angle
     */
    void set(const AxisAngle4<T>& a1);

    /**
      * Performs a great circle interpolation between this quaternion and the
      * quaternion parameter and places the result into this quaternion.
      * @param q1 the other quaternion
      * @param alpha the alpha interpolation parameter
      */
    void interpolate(const Quat4& q1, T alpha);

    /**
      * Performs a great circle interpolation between quaternion q1 and
      * quaternion q2 and places the result into this quaternion.
      * @param q1 the first quaternion
      * @param q2 the second quaternion
      * @param alpha the alpha interpolation parameter
      */
    void interpolate(const Quat4& q1, const Quat4& q2, T alpha);

    // copy constructor and operator = is made by complier

    Quat4& operator*=(const Quat4& m1);
    Quat4 operator*(const Quat4& m1) const;
};

VM_END_NS

#ifdef VM_INCLUDE_IO
template <class T>
VM_IOSTREAM_STD::ostream& operator<<(VM_IOSTREAM_STD::ostream& o, const VM_VECMATH_NS::Quat4<T>& q1);
#endif

VM_BEGIN_NS

typedef Quat4<double> Quat4d;
typedef Quat4<float> Quat4f;

VM_END_NS


#endif /* QUAT4__H */
