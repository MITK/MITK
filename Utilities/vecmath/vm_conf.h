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
#ifndef VM_CONF_H
#define VM_CONF_H

/* -----------------------------------------------------------
 *  you can configure these defines.
 *  commented lines 34/41 out, Christoph Giess 11.12.2000 
 * ----------------------------------------------------------- */

/*
 * can do 'cout << vec;'
 */
#define VM_INCLUDE_IO

/*
 * can do 'vec.toString();'
 * automatically defines VM_INCLUDE_IO
 */
/* #define VM_INCLUDE_TOSTRING */

/*
 * places this library in 'kh_vecmath' namespace
 * later section can turn this off(earlyer version than egcs1.1.2 ).
 * 
 */
// #define VM_INCLUDE_NAMESPACE

/*
 *  can do 'double a[3][3]; mat.set(a);'
 *  don't define this in gcc 2.7.2.(template bug).
 */
// #define VM_INCLUDE_CONVERSION_FROM_2DARRAY

/*
 * can do 'mat(i,j); v[i];'
 * defining this can be a performance problem.
 */
// #define VM_INCLUDE_SUBSCRIPTION_OPERATOR

/*
 * use cmath, cstddef, .... cXXXX type header
 * otherwise use traditional math, stddef ..
 */
#define VM_STD_C_HEADERS

/*
 * uses std:: namespace for sqrt, atan2, ...
 * depends on whether system include files exports these raw names
 */
#define VM_USE_STD_NAMESPACE

/*
 * uses exception mechanizm (not supported now)
 */
//#define VM_USE_EXCEPTION

/*
 * uses old non STL version of iostream library (currently only used for MSVC)
 */
//#define VM_USE_OLDIOSTREAM


/* -----------------------------------------------------------
 *   end user customization section 
 * ----------------------------------------------------------- */
 
// gcc earlier than egcs1.1.2, doesn't support namespace
#if !((__GNUC__ >= 2 && __GNUC_MINOR__ >= 91)|| (__GNUC__ >= 3))
#undef VM_INCLUDE_NAMESPACE
#undef VM_USE_STD_NAMESPACE
#endif


#ifdef _MSC_VER
#  ifndef VM_USE_STD_NAMESPACE
#    define VM_USE_STD_NAMESPACE
#  endif
#endif

#ifdef __BORLANDC__
#  ifndef VM_USE_STD_NAMESPACE
#    define VM_USE_STD_NAMESPACE
#  endif
#endif

//#define VM_USE_STD_NAMESPACE
#ifdef VM_INCLUDE_NAMESPACE
#  define VM_VECMATH_NS  kh_vecmath
#  define VM_BEGIN_NS namespace kh_vecmath {
#  define VM_END_NS }
#else
#  define VM_VECMATH_NS
#  define VM_BEGIN_NS
#  define VM_END_NS
#endif


#ifdef VM_INCLUDE_TOSTRING
#  ifndef VM_INCLUDE_IO
#    define VM_INCLUDE_IO
#  endif
#endif

#ifdef VM_USE_STD_NAMESPACE
#  define VM_STD std
#else
#  define VM_STD
#endif

#ifdef _MSC_VER
#  define VM_MATH_STD
#  define VM_STRING_STD VM_STD
#else
#  define VM_MATH_STD VM_STD
#  define VM_STRING_STD VM_STD
#endif

#ifdef VM_USE_OLDIOSTREAM
#  define VM_IOSTREAM_STD
#else
#  define VM_IOSTREAM_STD VM_STD
#endif

#endif /* VM_CONF_H */
