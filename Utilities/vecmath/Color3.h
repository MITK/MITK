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
#ifndef COLOR3_H
#define COLOR3_H

#include "VmUtil.h"
#include "Tuple3.h"

VM_BEGIN_NS

/**
  * A 3 element color represented by x,y,z
  * coordinates. 
  * @version specification 1.1, implementation $Revision$, $Date$
  * @author Kenji hiranabe
  */
template<class T>
class Color3 : public Tuple3<T> {
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
      * Constructs and initializes a Color3 from the specified xyz
      * @param x the x coordinate
      * @param y the y coordinate
      * @param z the z coordinate
      */
    Color3(T x, T y, T z): Tuple3<T>(x, y, z) { }

    /**
      * Constructs and initializes a Color3 from input array of length 3.
      * @param c the array of length 3 containing xyz in order
      */
    Color3(const T c[]): Tuple3<T>(c) { }

    /**
      * Constructs and initializes a Color3 from the specified Tuple3d.
      * @param t1 the Tuple3d containing the initialization x y z data
      */
    Color3(const Tuple3<T>& t1): Tuple3<T>(t1) { }

#if 0
    /**
      * Constructs and initializes a Color3 from the specified Tuple3f.
      * @param t1 the Tuple3f containing the initialization x y z data
      */
    Color3(Tuple3f t1) {
	super(t1);
    }
#endif

    /**
      * Constructs and initializes a Color3 to (0,0,0).
      */
    Color3(): Tuple3<T>() { }
};

typedef Color3<float> Color3f;
typedef Color3<double> Color3d;
typedef Color3<unsigned char> Color3b;

VM_END_NS

#endif /* COLOR3_H */
