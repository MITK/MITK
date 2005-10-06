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
#ifndef POINT3_H
#define POINT3_H

#include "VmUtil.h"
#include "Tuple3.h"
#include "Point4.h"

VM_BEGIN_NS

/**
  * A 3 element point that is represented by x,y,z coordinates.
  * @version specification 1.1, implementation $Revision$, $Date$
  * @author Kenji hiranabe
  */
template<class T>
class Point3 : public Tuple3<T> {
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
 * Revision 1.4  1999/10/06  02:52:46  hiranabe
 * Java3D 1.2 and namespace
 *
 * Revision 1.3  1999/09/10  02:19:09  hiranabe
 * distance*() method to const
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
    /**
      * Constructs and initializes a Point3 from the specified xyz coordinates.
      * @param x the x coordinate
      * @param y the y coordinate
      * @param z the z coordinate
      */
public:
    Point3(T x, T y, T z): Tuple3<T>(x, y, z) { }

    /**
      * Constructs and initializes a Point3 from the specified array.
      * @param p the array of length 3 containing xyz in order
      */
    Point3(const T p[]): Tuple3<T>(p) { }

    /**
      * Constructs and initializes a Point3 from the specified Tuple3d.
      * @param t1 the Tuple3d containing the initialization x y z data
      */
    Point3(const Tuple3<T>& t1): Tuple3<T>(t1) { }

    /**
      * Constructs and initializes a Point3 to (0,0,0).
      */
    Point3(): Tuple3<T>() { }

    /**
      * Computes the square of the distance between this point and point p1.
      * @param  p1 the other point
      * @return the square of distance between these two points as a float
      */
    T distanceSquared(const Point3& p1) const {
        T dx = this->x - p1.x;
        T dy = this->y - p1.y;
        T dz = this->z - p1.z;
        return dx*dx + dy*dy + dz*dz;
    }

    /**
      * Returns the distance between this point and point p1.
      * @param p1 the other point
      * @return the distance between these two points as a float
      */
    T distance(const Point3& p1) const {
        return VmUtil<T>::sqrt(distanceSquared(p1));
    }

    /**
      * Computes the L-1 (Manhattan) distance between this point and point p1.
      * The L-1 distance is equal to abs(x1-x2) + abs(y1-y2).
      * @param p1 the other point
      */
    T distanceL1(const Point3& p1) const {
        return VmUtil<T>::abs(this->x-p1.x) + VmUtil<T>::abs(this->y-p1.y) + VmUtil<T>::abs(this->z-p1.z);
    }

    /**
      * Computes the L-infinite distance between this point and point p1.
      * The L-infinite distance is equal to MAX[abs(x1-x2), abs(y1-y2)].
      * @param p1 the other point
      */
    T distanceLinf(const Point3& p1) const {
        return VmUtil<T>::max(VmUtil<T>::abs(this->x-p1.x), VmUtil<T>::abs(this->y-p1.y), VmUtil<T>::abs(this->z-p1.z));
    }

    /**
     * Multiplies each of the x,y,z components of the Point4 parameter
     * by 1/w and places the projected values into this point.
     * @param p1 the source Point4, which is not modified
     */
    void project(const Point4<T>& p1) {
        // zero div may occur.
        this->x = p1.x/p1.w;
        this->y = p1.y/p1.w;
        this->z = p1.z/p1.w;
    }

    // copy constructor and operator = is made by complier

    Point3& operator=(const Tuple3<T>& t) {
        Tuple3<T>::operator=(t);
        return *this;
    }
};

VM_END_NS

template <class T>
inline
VM_VECMATH_NS::Point3<T> operator*(T s, const VM_VECMATH_NS::Point3<T>& t1) {
    return operator*(s, (const VM_VECMATH_NS::Tuple3<T>&)t1);
}

#ifdef VM_INCLUDE_IO
template <class T>
inline
VM_IOSTREAM_STD::ostream& operator<<(VM_IOSTREAM_STD::ostream& o, const VM_VECMATH_NS::Point3<T>& t1) {
    return operator<<(o, (const VM_VECMATH_NS::Tuple3<T>&)t1);
}
#endif

VM_BEGIN_NS

typedef Point3<double> Point3d;
typedef Point3<float> Point3f;

VM_END_NS


#endif /* POINT3_H */
