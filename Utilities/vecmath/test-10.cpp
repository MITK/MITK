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
#include <assert.h>

#include "TexCoord2.h"
#include "Color3.h"
#include "Color4.h"

#ifdef VM_INCLUDE_NAMESPACE
using namespace kh_vecmath;
#endif

/**
 * test for Simple tuples.
 */
#ifdef TESTALL
int test_10() {
#else
int main(int, char**) {
#endif
    Color4<float> c;
    TexCoord2f tc;
    Color3b cb(255, 123, 254);
    cb.x = 0;
    tc.x = 10;
    c.y = 1.0f;
    return 0;
}
