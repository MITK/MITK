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
#ifndef TEXCOORD2_H
#define TEXCOORD2_H

#include "VmUtil.h"
#include "Tuple2.h"

VM_BEGIN_NS

/**
  * A 2 element texture coordinate that is represented by x,y coordinates.
  * @version specification 1.1, implementation $Revision$, $Date$
  * @author Kenji hiranabe
  */
template<class T>
class TexCoord2 : public Tuple2<T> {
/*
 * $Log$
 * Revision 1.3  2005/10/13 13:42:39  maleike
 * FIX: doxygen warnings
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
      * Constructs and initializes a TexCoord2 from the specified xy coordinates.
      * @param x the x coordinate
      * @param y the y coordinate
      */
    TexCoord2(T x, T y):  Tuple2<T>(x, y) { }

    /**
      * Constructs and initializes a TexCoord2 from the specified array.
      * @param v The array of length 2 containing xy in order
      */
    TexCoord2(const T v[]): Tuple2<T>(v) { }

    /**
      * Constructs and initializes a TexCoord2 to (0,0).
      */
    TexCoord2(): Tuple2<T>() { }

};

typedef TexCoord2<float> TexCoord2f;
typedef TexCoord2<double> TexCoord2d;

VM_END_NS

#endif /* TEXCOORD2_H */
