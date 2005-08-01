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
#ifndef VECTOR3_H
#define VECTOR3_H

#include "Tuple3.h"

VM_BEGIN_NS

/**
  * A 3 element vector that is represented by 
  * x,y,z coordinates. If this value represents a normal, then it should be
  * normalized.
  * @version specification 1.1, implementation $Revision$, $Date$
  * @author Kenji hiranabe
  */
template<class T>
class Vector3 : public Tuple3<T> {
/*
 * $Log$
 * Revision 1.5  2005/08/01 15:51:30  nolden
 * COMP: gcc 3.4 issues
 *
 * Revision 1.4  2003/11/04 14:19:17  nolden
 * gcc2 compatibility fix
 *
 * Revision 1.3  2003/04/22 14:42:11  max
 * made inclusions of vecmath header files "local" (for gcc 3.3 support)
 *
 * Revision 1.2  2002/12/09 13:02:33  tobias
 * added functions cart2sphere() and sphere2cart() for transformation from
 * cartesian to spherical coordinates and vice versa
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
      * Constructs and initializes a Vector3 from the specified xyz coordinates.
      * @param x the x coordinate
      * @param y the y coordinate
      * @param z the z coordinate
      */
    Vector3(T x, T y, T z): Tuple3<T>(x, y, z) { }

    /**
      * Constructs and initializes a Vector3 from the specified array of length 3.
      * @param v the array of length 3 containing xyz in order
      */
    Vector3(const T v[]): Tuple3<T>(v) {  }

    /**
      * Constructs and initializes a Vector3 from the specified Tuple3d.
      * @param t1 the Tuple3d containing the initialization x y z data
      */
    Vector3(const Tuple3<T>& t1): Tuple3<T>(t1) { }

    /**
      * Constructs and initializes a Vector3 to (0,0,0).
      */
    Vector3(): Tuple3<T>() {   }

    /**
      * Sets this vector to be the vector cross product of vectors v1 and v2.
      * @param v1 the first vector
      * @param v2 the second vector
      */
    void cross(const Vector3& v1, const Vector3& v2) {
        // store on stack once for aliasing-safty
        // i.e. safe when a.cross(a, b)
 
        // this-> is necessary for compatibility with std::set in gcc2
        this->set(
            v1.y*v2.z - v1.z*v2.y,
            v1.z*v2.x - v1.x*v2.z,
            v1.x*v2.y - v1.y*v2.x
            );
    }
    

    /**
      * Sets the value of this vector to the normalization of vector v1.
      * @param v1 the un-normalized vector
      */
    void normalize(const Vector3& v1) {
        // this-> is necessary for compatibility with std::set in gcc2
        this->set(v1);
        normalize();
    }

    /**
      * Normalizes this vector in place.
      */
    void normalize() {
        T d = length();

        // zero-div may occur.
        this->x /= d;
        this->y /= d;
        this->z /= d;
    }

    /**
      * Computes the dot product of the this vector and vector v1.
      * @param  v1 the other vector
      */
    T dot(const Vector3& v1) const {
        return this->x*v1.x +this->y*v1.y +this->z*v1.z;
    }


    /**
      * Returns the squared length of this vector.
      * @return the squared length of this vector
      */
    T lengthSquared() const {
        return this->x*this->x + this->y*this->y + this->z*this->z;
    }

    /**
      * Returns the length of this vector.
      * @return the length of this vector
      */
    T length() const {
        return VmUtil<T>::sqrt(lengthSquared());
    }

    /**
      * Returns the angle in radians between this vector and
      * the vector parameter; the return value is constrained to the
      * range [0,PI].
      * @param v1  the other vector
      * @return the angle in radians in the range [0,PI]
      */
    T angle(const Vector3& v1) const {
        // return (double)Math.acos(dot(v1)/v1.length()/v.length());
        // Numerically, near 0 and PI are very bad condition for acos.
        // In 3-space, |atan2(sin,cos)| is much stable.
        Vector3 c;
        c.cross(*this, v1);
        T sin = c.length();

        return VmUtil<T>::abs(VmUtil<T>::atan2(sin, dot(v1)));
    }

	/**
      * Transforms cartesian into spherical coordinates
	  * Spherical coordinates are encoded as follows:
	  * x: azimuth angle in xy-plane from x-axis, [-PI,PI]
      * y: polar angle from z-axis, [0,PI]
	  * z: distance from origin (radius)
      * @return the vector in spherical coordinates
      */
    Vector3 cart2sphere() const {
           
		T radius = length();
		if (radius == 0) return Vector3( 0, 0, 0 );

		T polar = VmUtil<T>::acos( this->z / radius );
		T azimuth = VmUtil<T>::atan2( this->y, this->x );
		return Vector3( azimuth, polar, radius );
    }

	/**
      * Transforms spherical into cartesian coordinates
	  * Spherical coordinates are encoded as follows:
	  * x: azimuth angle in xy-plane from x-axis, [-PI,PI]
      * y: polar angle from z-axis, [0,PI]
	  * z: distance from origin (radius)
      * @return the vector in cartesian coordinates
      */
    Vector3 sphere2cart() const {
           
		T cartX = this->z * VmUtil<T>::cos( this->x ) * VmUtil<T>::sin( this->y );
		T cartY = this->z * VmUtil<T>::sin( this->x ) * VmUtil<T>::sin( this->y );
		T cartZ = this->z * VmUtil<T>::cos( this->y );
		return Vector3( cartX, cartY, cartZ );
    }

    // copy constructor and operator = is made by complier

    Vector3& operator=(const Tuple3<T>& t) {
        Tuple3<T>::operator=(t);
        return *this;
    }
};

VM_END_NS

template <class T>
inline
VM_VECMATH_NS::Vector3<T> operator*(T s, const VM_VECMATH_NS::Vector3<T>& t1) {
    return operator*(s, (const VM_VECMATH_NS::Tuple3<T>&)t1);
}

#ifdef VM_INCLUDE_IO
template <class T>
inline
VM_IOSTREAM_STD::ostream& operator<<(VM_IOSTREAM_STD::ostream& o, const VM_VECMATH_NS::Vector3<T>& t1) {
    return operator<<(o, (const VM_VECMATH_NS::Tuple3<T>&)t1);
}
#endif

VM_BEGIN_NS

typedef Vector3<double> Vector3d;
typedef Vector3<float> Vector3f;

VM_END_NS

#endif /* VECTOR3_H */
