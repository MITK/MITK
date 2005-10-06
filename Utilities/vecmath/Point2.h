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
#ifndef POINT2_H
#define POINT2_H

#include "VmUtil.h"
#include "Tuple2.h"

VM_BEGIN_NS

/**
  * A 2 element point that is represented by x,y coordinates.
  * @version specification 1.1, implementation $Revision$, $Date$
  * @author Kenji hiranabe
  */
template<class T>
class Point2 : public Tuple2<T> {
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
 * Revision 1.3  1999/09/10  02:19:20  hiranabe
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
public:
    /**
      * Constructs and initializes a Point2 from the specified xy coordinates.
      * @param x the x coordinate
      * @param y the y coordinate
      */
    Point2(T x, T y): Tuple2<T>(x, y) { }

    /**
      * Constructs and initializes a Point2 from the specified array.
      * @param p the array of length 2 containing xy in order
      */
    Point2(const T p[]): Tuple2<T>(p) { }

    /**
      * Constructs and initializes a Point2 from the specified Tuple2.
      * @param t1 the Tuple2 containing the initialization x y data
      */
    Point2(const Tuple2<T>& t1): Tuple2<T>(t1) { }

    /**
      * Constructs and initializes a Point2 to (0,0).
      */
    Point2(): Tuple2<T>() { }

    /**
      * Computes the square of the distance between this point and point p1.
      * @param  p1 the other point
      */
    T distanceSquared(const Point2& p1) const {
        T dx = this->x - p1.x;
        T dy = this->y - p1.y;
        return dx*dx + dy*dy;
    }

    /**
      * Computes the distance between this point and point p1.
      * @param p1 the other point
      */
    T distance(const Point2& p1) const {
        return VmUtil<T>::sqrt(distanceSquared(p1));
    }

    /**
      * Computes the L-1 (Manhattan) distance between this point and point p1.
      * The L-1 distance is equal to abs(x1-x2) + abs(y1-y2).
      * @param p1 the other point
      */
    T distanceL1(const Point2& p1) const {
        return VmUtil<T>::abs(this->x-p1.x) + VmUtil<T>::abs(this->y-p1.y);
    }

    /**
      * Computes the L-infinite distance between this point and point p1.
      * The L-infinite distance is equal to MAX[abs(x1-x2), abs(y1-y2)].
      * @param p1 the other point
      */
    T distanceLinf(const Point2& p1) const {
        return VmUtil<T>::max(VmUtil<T>::abs(this->x-p1.x), VmUtil<T>::abs(this->y-p1.y));
    }

    // copy constructor and operator = is made by complier

    Point2& operator=(const Tuple2<T>& t) {
        Tuple2<T>::operator=(t);
        return *this;
    }
};

VM_END_NS

#ifdef VM_INCLUDE_IO
template <class T>
inline
VM_IOSTREAM_STD::ostream& operator<<(VM_IOSTREAM_STD::ostream& o, const VM_VECMATH_NS::Point2<T>& t1) {
    return operator<<(o, (const VM_VECMATH_NS::Tuple2<T>&)t1);
}
#endif

VM_BEGIN_NS

typedef Point2<double> Point2d;
typedef Point2<float> Point2f;

/*
 * 0. value_type typedef added
 * 1. copy constructo, oeprator = are delegated to compiler
 * 4. typdef value type
 * 7. typedefs for <float>, <double>
 * removed construction from Vector2f
 */

VM_END_NS

#endif /* POINT2_H */
