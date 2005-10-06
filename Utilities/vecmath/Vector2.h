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
#ifndef VECTOR2_H
#define VECTOR2_H

#include "VmUtil.h"
#include "Tuple2.h"

VM_BEGIN_NS

/**
  * A 2 element vector that is represented by x,y coordinates.
  * @version specification 1.1, implementation $Revision$, $Date$
  * @author Kenji hiranabe
  */

template<class T>
class Vector2 : public Tuple2<T> {
/*
 * $Log$
 * Revision 1.4  2005/10/06 15:27:15  nolden
 * FIX: gcc4 compatibility
 *
 * Revision 1.3  2003/11/04 14:19:17  nolden
 * gcc2 compatibility fix
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
      * Constructs and initializes a Vector2 from the specified xy coordinates.
      * @param x the x coordinate
      * @param y the y coordinate
      */
    Vector2(T x, T y) : Tuple2<T>(x, y) { }

    /**
      * Constructs and initializes a Vector2 from the specified array.
      * @param v the array of length 2 containing xy in order
      */
    Vector2(const T v[]) : Tuple2<T>(v) {  }

    /**
      * Constructs and initializes a Vector2 from the specified Tuple2.
      * @param t1 the Tuple2 containing the initialization x y data
      */
    Vector2(const Tuple2<T>& t1) : Tuple2<T>(t1) { }

    /**
      * Constructs and initializes a Vector2 to (0,0).
      */
    Vector2(): Tuple2<T>() { }

    /**
      * Computes the dot product of the this vector and vector v1.
      * @param  v1 the other vector
      */
    T dot(const Vector2& v1) const {
        return this->x*v1.x + this->y*v1.y;
    }

    /**
      * Returns the squared length of this vector.
      * @return the squared length of this vector
      */
    T lengthSquared() const {
        return this->x*this->x + this->y*this->y;
    }

    /**
      * Returns the length of this vector.
      * @return the length of this vector
      */
    T length() const {
        return VmUtil<T>::sqrt(lengthSquared());
    }

    /**
      * Normalizes this vector in place.
      */
    void normalize() {
        T d = length();

        // zero-div may occur.
        this->x /= d;
        this->y /= d;
    }

    /**
      * Sets the value of this vector to the normalization of vector v1.
      * @param v1 the un-normalized vector
      */
    void normalize(const Vector2& v1) {
        // "this->" is necessary for compatibility with std::set in gcc2
        this->set(v1);
        normalize();
    }

    /**
      * Returns the angle in radians between this vector and
      * the vector parameter; the return value is constrained to the
      * range [0,PI].
      * @param v1  the other vector
      * @return the angle in radians in the range [0,PI]
      */
    T angle(const Vector2& v1) const {
        // stabler than acos
        return VmUtil<T>::abs(VmUtil<T>::atan2(this->x*v1.y - this->y*v1.x , dot(v1)));
    }

    // copy constructor and operator = is made by complier

    Vector2& operator=(const Tuple2<T>& t) {
        Tuple2<T>::operator=(t);
        return *this;
    }

};

/*
 * 0. value_type typedef added
 * 1. copy constructo, oeprator = are delegated to compiler
 * 4. typdef value type
 * 7. typedefs for <float>, <double>
 * removed construction from Vector2f
 */

VM_END_NS

template <class T>
inline
VM_VECMATH_NS::Vector2<T> operator*(T s, const VM_VECMATH_NS::Vector2<T>& t1) {
    return operator*(s, (const VM_VECMATH_NS::Tuple2<T>&)t1);
}

#ifdef VM_INCLUDE_IO
template <class T>
inline
VM_IOSTREAM_STD::ostream& operator<<(VM_IOSTREAM_STD::ostream& o, const VM_VECMATH_NS::Vector2<T>& t1) {
    return operator<<(o, (const VM_VECMATH_NS::Tuple2<T>&)t1);
}
#endif

VM_BEGIN_NS

typedef Vector2<double> Vector2d;
typedef Vector2<float> Vector2f;

VM_END_NS



#endif /* VECTOR2_H */
