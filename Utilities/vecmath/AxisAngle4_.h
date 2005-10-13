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
#ifndef AXISANGLE4__H
#define AXISANGLE4__H

#include "VmUtil.h"
#include "Matrix4_.h"

VM_BEGIN_NS

/**
  * A 4 element axis angle represented by 
  * x,y,z,angle components. An axis angle is a rotation of angle (radians) about
  * the vector (x,y,z).
  * @version specification 1.1, implementation $Revision$, $Date$
  * @author Kenji hiranabe
  */
template<class T>
class AxisAngle4 {
/*
 * $Log$
 * Revision 1.3  2005/10/13 13:50:04  maleike
 * FIX: doxygen warnings
 *
 * Revision 1.2  2003/04/22 14:42:11  max
 * made inclusions of vecmath header files "local" (for gcc 3.3 support)
 *
 * Revision 1.1  2002/11/16 12:25:36  mark
 * vecmath bibliothek
 * in cvs, da einige bugs behoben
 *
 * Revision 1.4  1999/10/06  02:52:46  hiranabe
 * Java3D 1.2 and namespace
 *
 * Revision 1.3  1999/10/05  06:19:19  hiranabe
 * Java3D 1.2
 * AxisAngle4(Vector3,T)
 * set(Vector3,T)
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
/*
 * I assumed that the length of axis vectors is not significant.
 */
public:
    /**
     * the type for values
     */
    typedef T value_type;

    /**
      * The x coordinate.
      */
    T x;

    /**
      * The y coordinate.
      */
    T y;

    /**
      * The z coordinate.
      */
    T z;

    /**
      * The angle.
      */
    T angle;

    /**
      * Constructs and initializes an AxisAngle4 from the specified x, y, z,
      * and angle.
      * @param xvalue the x coordinate
      * @param yvalue the y coordinate
      * @param zvalue the z coordinate
      * @param anglevalue the angle.
      */
    AxisAngle4(T xvalue, T yvalue, T zvalue, T anglevalue):
        x(xvalue), y(yvalue), z(zvalue), angle(anglevalue) { }

    /**
      * Constructs and initializes an AxisAngle4 from the components contained
      * in the array.
      * @param a the array of length 4 containing x,y,z,angle in order
      */
    AxisAngle4(const T a[]):
        x(a[0]), y(a[1]), z(a[2]), angle(a[3]) { }

#if 0
    /**
      * Constructs and initializes a AxisAngle4 from the specified AxisAngle4f.
      * @param a1 the AxisAngle4f containing the initialization x y z angle data
      */
    AxisAngle4(AxisAngle4f a1) {
	set(a1);
    }
#endif

    /**
      * Constructs and initializes a AxisAngle4 to (0,0,1,0).
      */
    AxisAngle4(): x(0), y(0), z(1), angle(0) { }

    /**
     * Constructs and initializes an AxisAngle4f from the specified axis
     * and angle.
     *
     * @param axis the axis
     * @param ang the angle
     * @since Java 3D 1.2
     */
    AxisAngle4(const Vector3<T>& axis, T ang):
        x(axis.x), y(axis.y), z(axis.z), angle(ang) { }

    /**
     * Sets the value of this AxisAngle4f to the specified axis and
     * angle.
     * @param axis the axis
     * @param ang the angle
     * @since Java 3D 1.2
     */
    void set(const Vector3<T>& axis, T ang) {
        x = axis.x;
        y = axis.y;
        z = axis.z;
        angle = ang;
    }

    /**
      * Sets the value of this axis angle to the specified x,y,z,angle.
      * @param x the x coordinate
      * @param y the y coordinate
      * @param z the z coordinate
      * @param angle the angle
      */
    void set(T x, T y, T z, T angle) {
        this->x = x;
        this->y = y;
        this->z = z;
        this->angle = angle;
    }

    /**
      * Sets the value of this axis angle from the 4 values specified in the array.
      * @param a the array of length 4 containing x,y,z,angle in order
      */
    void set(const T a[]) {
        assert(a != 0);
        x = a[0];
        y = a[1];
        z = a[2];
        angle = a[3];
    }

    /**
      * Sets the value of this axis angle to the value of axis angle t1.
      * @param a1 the axis angle to be copied
      */
    void set(const AxisAngle4& a1) {
        x = a1.x;
        y = a1.y;
        z = a1.z;
        angle = a1.angle;
    }

#if 0
    /**
      * Sets the value of this axis angle to the value of axis angle t1.
      * @param t1 the axis angle to be copied
      */
    void set(AxisAngle4f a1) {
	x = (float)(a1.x);
	y = (float)(a1.y);
	z = (float)(a1.z);
	angle = (float)(a1.angle);
    }
#endif

    /**
      * Gets the value of this axis angle into the array a of
      * length four in x,y,z,angle order.
      * @param a the array of length four
      */
    void get(T a[]) const {
        assert(a != 0);
        a[0] = x;
        a[1] = y;
        a[2] = z;
        a[3] = angle;
    }

    /**
      * Sets the value of this axis-angle to the rotational component of the
      * passed matrix.
      * @param m1 the matrix4
      */
    void set(const Matrix4<T>& m1) {
        setFromMat(
                   m1.m00, m1.m01, m1.m02,
                   m1.m10, m1.m11, m1.m12,
                   m1.m20, m1.m21, m1.m22
                   );
    }

    /**
      * Sets the value of this axis-angle to the rotational component of the
      * passed matrix.
      * @param m1 the matrix3f
      */
    void set(const Matrix3<T>& m1) {
        setFromMat(
                   m1.m00, m1.m01, m1.m02,
                   m1.m10, m1.m11, m1.m12,
                   m1.m20, m1.m21, m1.m22
                   );
    }

    /**
      * Sets the value of this axis-angle to the rotational equivalent of the
      * passed quaternion.
      * @param q1 the Quat4
      */
    void set(const Quat4<T>& q1);

    // helper method
protected:
    void setFromMat(T m00, T m01, T m02,
		       T m10, T m11, T m12,
		       T m20, T m21, T m22);

    void setFromQuat(T x, T y, T z, T w);

public:
    /**
      * Returns true if all of the data members of AxisAngle4 t1 are equal to the corresponding
      * data members in this
      * @param a1 the vector with which the comparison is made.
      */
    bool equals(const AxisAngle4& a1) const {
        return x == a1.x && y == a1.y && z == a1.z && angle == a1.angle;
    }

    /**
      * Returns true if the L-infinite distance between this axis-angle and axis-angle t1 is
      * less than or equal to the epsilon parameter, otherwise returns false. The L-infinite
      * distance is equal to MAX[abs(x1-x2), abs(y1-y2), abs(z1-z2), abs(angle1-angle2)].
      * @param a1 the axis-angle to be compared to this axis-angle
      * @param epsilon the threshold value
      */
    bool epsilonEquals(const AxisAngle4& a1, T epsilon) const {
        return (VmUtil<T>::abs(a1.x - x) <= epsilon) &&
            (VmUtil<T>::abs(a1.y - y) <= epsilon) &&
            (VmUtil<T>::abs(a1.z - z) <= epsilon) &&
            (VmUtil<T>::abs(a1.angle - angle) <= epsilon);
    }

    /**
      * Returns a hash number based on the data values in this object. 
      * Two different AxisAngle4 objects with identical data  values
      * (ie, returns true for equals(AxisAngle4) ) will return the same hash number.
      * Two vectors with different data members may return the same hash value,
      * although this is not likely.
      */
    size_t hashCode() const {
        return VmUtil<T>::hashCode(sizeof *this, this);
    }

    /**
      * Returns a string that contains the values of this AxisAngle4. The form is (x,y,z,angle).
      * @return the String representation
      */
#ifdef VM_INCLUDE_TOSTRING
VM_STRING_STD::string toString() const;
#endif

    // copy constructor and operator = is made by complier

    bool operator==(const AxisAngle4& m1) const {
        return equals(m1);
    }

};

VM_END_NS

#ifdef VM_INCLUDE_IO
template <class T>
VM_IOSTREAM_STD::ostream& operator<<(VM_IOSTREAM_STD::ostream& o, const VM_VECMATH_NS::AxisAngle4<T>& t1);
#endif

VM_BEGIN_NS

typedef AxisAngle4<double> AxisAngle4d;
typedef AxisAngle4<float> AxisAngle4f;

VM_END_NS



#endif /* AXISANGLE4__H */
