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
#ifndef COLOR4_H
#define COLOR4_H

#include "VmUtil.h"
#include "Tuple4.h"

VM_BEGIN_NS

/**
  * A 4 element color represented by x,y,z,w
  * coordinates. Color and alpha components.
  * @version specification 1.1, implementation 1.0 $Revision$, $Date$
  * @author Kenji hiranabe
  */
template<class T>
class Color4 : public Tuple4<T> {
/*
 * $Log$
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
      * Constructs and initializes a Color4 from the specified xyzw
      * @param x the x coordinate
      * @param y the y coordinate
      * @param z the z coordinate
      * @param w the w coordinate
      */
    Color4(T x, T y, T z, T w): Tuple4<T>(x, y, z, w) { }

    /**
      * Constructs and initializes a Color4 from input array of length 4.
      * @param c the array of length 4 containing xyzw in order
      */
    Color4(const T c[]): Tuple4<T>(c) { }

    /**
      * Constructs and initializes a Color4 from the specified Tuple4d.
      * @param t1 the Tuple4d containing the initialization x y z w data
      */
    Color4(const Tuple4<T>& t1): Tuple4<T>(t1) {  }

#if 0
    /**
      * Constructs and initializes a Color4 from the specified Tuple4.
      * @param t1 the Tuple4 containing the initialization x y z w data
      */
    Color4(Tuple4 t1) {
	Tuple4<T>(t1);
    }
#endif

    /**
      * Constructs and initializes a Color4 to (0,0,0,0).
      */
    Color4(): Tuple4<T>() { }
};

typedef Color4<float> Color4f;
typedef Color4<double> Color4d;
typedef Color4<unsigned char> Color4b;

VM_END_NS

#endif /* COLOR4_H */
