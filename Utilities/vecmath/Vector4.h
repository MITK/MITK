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
#ifndef VECTOR4_H
#define VECTOR4_H

#include <VmUtil.h>
#include <Tuple4.h>
#include <Tuple3.h>

VM_BEGIN_NS

/**
  * A 4 element vector that is represented by x,y,z,w coordinates. 
  * @version specification 1.1, implementation $Revision$, $Date$
  * @author Kenji hiranabe
  */
template<class T>
class Vector4 : public Tuple4<T> {
/*
 * $Log$
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
      * Constructs and initializes a Vector4 from the specified xyzw coordinates.
      * @param x the x coordinate
      * @param y the y coordinate
      * @param z the z coordinate
      * @param w the w coordinate
      */
    Vector4(T x, T y, T z, T w): Tuple4<T>(x, y, z, w) { }

    /**
      * Constructs and initializes a Vector4 from the specified array of length 4.
      * @param v the array of length 4 containing xyzw in order
      */
    Vector4(const T v[]): Tuple4<T>(v) { }

    /**
      * Constructs and initializes a Vector4 from the specified Tuple4d.
      * @param t1 the Tuple4d containing the initialization x y z w data
      */
    Vector4(const Tuple4<T>& t1): Tuple4<T>(t1) { }

    /**
      * Constructs and initializes a Vector4 to (0,0,0,0).
      */
    Vector4(): Tuple4<T>() { }

    /**
     * Constructs and initializes a Vector4 from the specified Tuple3.
     * The x,y,z  components of this point are set to the corresponding
     * components
     * of tuple t1. The w component of this point is set to 0.
     *
     * @param t1 the tuple to be copied
     * @since Java3D 1.2
     */
    Vector4(const Tuple3<T>& t1): Tuple4<T>(t1.x, t1.y, t1.z, 0) { }

    /**
     * Sets the x,y,z components of this point to the corresponding
     * components of tuple t1. The w component of this point is set to 1.
     *
     * NOTE: 
     * This method's name should be 'set'. Newer compilers support
     * 'using Tuple4<T>::set;' and use the name 'set'.
     * but for now, renamed to 'set3'.
     *
     * @param t1 the tuple to be copied
     * @since Java3D 1.2
     */
    void set3(const Tuple3<T>& t1) {
        x = t1.x;
        y = t1.y;
        z = t1.z;
        w = 0;
    }



    /**
      * Returns the squared length of this vector.
      * @return the squared length of this vector
      */
    T lengthSquared() const {
        return x*x + y*y + z*z + w*w;
    }

    /**
      * Returns the length of this vector.
      * @return the length of this vector
      */
    T length() const {
        return VmUtil<T>::sqrt(lengthSquared());
    }

    /**
     * Computes the dot product of the this vector and vector v1.
     * @param  v1 the other vector
     * @return the dot product of this vector and vector v1
     */
    T dot(const Vector4& v1) const {
        return x*v1.x + y*v1.y + z*v1.z + w*v1.w;
    }

    /**
      * Sets the value of this vector to the normalization of vector v1.
      * @param v1 the un-normalized vector
      */
    void normalize(const Vector4& v1) {
        Tuple4<T>::set(v1);
        normalize();
    }

    /**
      * Normalizes this vector in place.
      */
    void normalize() {
        T d = length();

        // zero-div may occur.
        x /= d;
        y /= d;
        z /= d;
        w /= d;
    }

    /**
      * Returns the (4-space) angle in radians between this vector and
      * the vector parameter; the return value is constrained to the
      * range [0,PI].
      * @param v1  the other vector
      * @return the angle in radians in the range [0,PI]
      */
    T angle(const Vector4& v1) const {
        // zero div may occur.
        T d = dot(v1);
        T v1_length = v1.length();
        T v_length = length();

        // numerically, domain error may occur
        return T(acos(d/v1_length/v_length));
    }

    // copy constructor and operator = is made by complier

    Vector4& operator=(const Tuple4<T>& t) {
        Tuple4<T>::operator=(t);
        return *this;
    }
};

VM_END_NS

template <class T>
inline
VM_VECMATH_NS::Vector4<T> operator*(T s, const VM_VECMATH_NS::Vector4<T>& t1) {
    return operator*(s, (const VM_VECMATH_NS::Tuple4<T>&)t1);
}

#ifdef VM_INCLUDE_IO
template <class T>
inline
VM_IOSTREAM_STD::ostream& operator<<(VM_IOSTREAM_STD::ostream& o, const VM_VECMATH_NS::Vector4<T>& t1) {
    return operator<<(o, (const VM_VECMATH_NS::Tuple4<T>&)t1);
}
#endif

VM_BEGIN_NS

typedef Vector4<double> Vector4d;
typedef Vector4<float> Vector4f;

VM_END_NS


#endif /* VECTOR4_H */
