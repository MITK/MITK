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
#ifndef POINT4_H
#define POINT4_H

#include "VmUtil.h"
#include "Tuple4.h"
#include "Tuple3.h"

VM_BEGIN_NS

/**
  * A 4 element point that is represented x,y,z,w coordinates.
  * @version specification 1.1, implementation $Revision$, $Date$
  * @author Kenji hiranabe
  */
template<class T>
class Point4 : public Tuple4<T> {
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
      * Constructs and initializes a Point4 from the specified xyzw coordinates.
      * @param x the x coordinate
      * @param y the y coordinate
      * @param z the z coordinate
      * @param w the w coordinate
      */
    Point4(T x, T y, T z, T w): Tuple4<T>(x, y, z, w) { }

    /**
      * Constructs and initializes a Point4 from the specified array.
      * @param p the array of length 4 containing xyzw in order
      */
    Point4(const T p[]): Tuple4<T>(p) { }

    /**
      * Constructs and initializes a Point4 from the specified Tuple4d.
      * @param t1 the Tuple4d containing the initialization x y z w data
      */
    Point4(const Tuple4<T>& t1): Tuple4<T>(t1) { }

    /**
      * Constructs and initializes a Point4 to (0,0,0,0).
      */
    Point4(): Tuple4<T>() { }

    /**
     * Constructs and initializes a Point4 from the specified Tuple3.
     * The x,y,z  components of this point are set to the corresponding
     * components
     * of tuple t1. The w component of this point is set to 1.
     *
     * @param t1 the tuple to be copied
     * @since Java3D 1.2
     */
    Point4(const Tuple3<T>& t1): Tuple4<T>(t1.x, t1.y, t1.z, 1) { }

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
        this->x = t1.x;
        this->y = t1.y;
        this->z = t1.z;
        this->w = 1;
    }

    /**
      * Computes the square of the distance between this point and point p1.
      * @param  p1 the other point
      * @return the square of distance between this point and p1
      */
    T distanceSquared(const Point4& p1) const {
        T dx = this->x - p1.x;
        T dy = this->y - p1.y;
        T dz = this->z - p1.z;
        T dw = this->w - p1.w;
        return dx*dx + dy*dy + dz*dz + dw*dw;
    }

    /**
      * Returns the distance between this point and point p1.
      * @param p1 the other point
      * @return the distance between this point and point p1.
      */
    T distance(const Point4& p1) const {
        return VmUtil<T>::sqrt(distanceSquared(p1));
    }

    /**
      * Computes the L-1 (Manhattan) distance between this point and point p1.
      * The L-1 distance is equal to abs(x1-x2) + abs(y1-y2)
      * + abs(z1-z2) + abs(w1-w2).
      * @param p1 the other point
      * @return L-1 distance
      */
    T distanceL1(const Point4& p1) const {
        return VmUtil<T>::abs(this->x-p1.x) + VmUtil<T>::abs(this->y-p1.y)
	    + VmUtil<T>::abs(this->z-p1.z) + VmUtil<T>::abs(this->w-p1.w);
    }

    /**
      * Computes the L-infinite distance between this point and point p1.
      * The L-infinite distance is equal to MAX[abs(x1-x2), abs(y1-y2), abs(z1-z2), abs(w1-w2)].
      * @param p1 the other point
      * @return L-infinite distance
      */
    T distanceLinf(const Point4& p1) const {
        return VmUtil<T>::max(VmUtil<T>::abs(this->x-p1.x), VmUtil<T>::abs(this->y-p1.y), VmUtil<T>::abs(this->z-p1.z), VmUtil<T>::abs(this->w-p1.w));
    }

    /**
      * Multiplies each of the x,y,z components of the Point4 parameter by 1/w,
      * places the projected values into this point, and places a 1 as the w
      * parameter of this point.
      * @param p1 the source Point4, which is not modified
      */
     void project(const Point4& p1) {
         // zero div may occur.
         this->x = p1.x/p1.w;
         this->y = p1.y/p1.w;
         this->z = p1.z/p1.w;
         this->w = 1;
     }

    // copy constructor and operator = is made by complier

    Point4& operator=(const Tuple4<T>& t) {
        Tuple4<T>::operator=(t);
        return *this;
    }
};

VM_END_NS

template <class T>
inline
VM_VECMATH_NS::Point4<T> operator*(T s, const VM_VECMATH_NS::Point4<T>& t1) {
    return operator*(s, (const VM_VECMATH_NS::Tuple4<T>&)t1);
}

#ifdef VM_INCLUDE_IO
template <class T>
inline
VM_IOSTREAM_STD::ostream& operator<<(VM_IOSTREAM_STD::ostream& o, const VM_VECMATH_NS::Point4<T>& t1) {
    return operator<<(o, (const VM_VECMATH_NS::Tuple4<T>&)t1);
}
#endif

VM_BEGIN_NS

typedef Point4<double> Point4d;
typedef Point4<float> Point4f;

VM_END_NS

#endif /* POINT4_H */

