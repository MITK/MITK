/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/
# /* **************************************************************************
#  *                                                                          *
#  *     (C) Copyright Paul Mensonides 2002.
#  *     Distributed under the Boost Software License, Version 1.0. (See
#  *     accompanying file LICENSE_1_0.txt or copy at
#  *     http://www.boost.org/LICENSE_1_0.txt)
#  *                                                                          *
#  ************************************************************************** */
#
# /* See http://www.boost.org for most recent version. */
#
# ifndef MITK_PREPROCESSOR_TUPLE_REM_HPP
# define MITK_PREPROCESSOR_TUPLE_REM_HPP
#
# include "mitkPPConfig.h"
#
# /* MITK_PP_TUPLE_REM */
#
# if ~MITK_PP_CONFIG_FLAGS() & MITK_PP_CONFIG_MWCC()
#    define MITK_PP_TUPLE_REM(size) MITK_PP_TUPLE_REM_I(size)
# else
#    define MITK_PP_TUPLE_REM(size) MITK_PP_TUPLE_REM_OO((size))
#    define MITK_PP_TUPLE_REM_OO(par) MITK_PP_TUPLE_REM_I ## par
# endif
#
# define MITK_PP_TUPLE_REM_I(size) MITK_PP_TUPLE_REM_ ## size
#
# define MITK_PP_TUPLE_REM_0()
# define MITK_PP_TUPLE_REM_1(a) a
# define MITK_PP_TUPLE_REM_2(a, b) a, b
# define MITK_PP_TUPLE_REM_3(a, b, c) a, b, c
# define MITK_PP_TUPLE_REM_4(a, b, c, d) a, b, c, d
# define MITK_PP_TUPLE_REM_5(a, b, c, d, e) a, b, c, d, e
# define MITK_PP_TUPLE_REM_6(a, b, c, d, e, f) a, b, c, d, e, f
# define MITK_PP_TUPLE_REM_7(a, b, c, d, e, f, g) a, b, c, d, e, f, g
# define MITK_PP_TUPLE_REM_8(a, b, c, d, e, f, g, h) a, b, c, d, e, f, g, h
# define MITK_PP_TUPLE_REM_9(a, b, c, d, e, f, g, h, i) a, b, c, d, e, f, g, h, i
# define MITK_PP_TUPLE_REM_10(a, b, c, d, e, f, g, h, i, j) a, b, c, d, e, f, g, h, i, j
# define MITK_PP_TUPLE_REM_11(a, b, c, d, e, f, g, h, i, j, k) a, b, c, d, e, f, g, h, i, j, k
# define MITK_PP_TUPLE_REM_12(a, b, c, d, e, f, g, h, i, j, k, l) a, b, c, d, e, f, g, h, i, j, k, l
# define MITK_PP_TUPLE_REM_13(a, b, c, d, e, f, g, h, i, j, k, l, m) a, b, c, d, e, f, g, h, i, j, k, l, m
# define MITK_PP_TUPLE_REM_14(a, b, c, d, e, f, g, h, i, j, k, l, m, n) a, b, c, d, e, f, g, h, i, j, k, l, m, n
# define MITK_PP_TUPLE_REM_15(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o) a, b, c, d, e, f, g, h, i, j, k, l, m, n, o
# define MITK_PP_TUPLE_REM_16(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p) a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p
# define MITK_PP_TUPLE_REM_17(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q) a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q
# define MITK_PP_TUPLE_REM_18(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r) a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r
# define MITK_PP_TUPLE_REM_19(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s) a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s
# define MITK_PP_TUPLE_REM_20(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t) a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t
# define MITK_PP_TUPLE_REM_21(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u) a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u
# define MITK_PP_TUPLE_REM_22(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v) a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v
# define MITK_PP_TUPLE_REM_23(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w) a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w
# define MITK_PP_TUPLE_REM_24(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x) a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x
# define MITK_PP_TUPLE_REM_25(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y) a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y
#
# /* MITK_PP_TUPLE_REM_CTOR */
#
# if ~MITK_PP_CONFIG_FLAGS() & MITK_PP_CONFIG_EDG()
#    define MITK_PP_TUPLE_REM_CTOR(size, tuple) MITK_PP_TUPLE_REM_CTOR_I(MITK_PP_TUPLE_REM(size), tuple)
# else
#    define MITK_PP_TUPLE_REM_CTOR(size, tuple) MITK_PP_TUPLE_REM_CTOR_D(size, tuple)
#    define MITK_PP_TUPLE_REM_CTOR_D(size, tuple) MITK_PP_TUPLE_REM_CTOR_I(MITK_PP_TUPLE_REM(size), tuple)
# endif
#
# if ~MITK_PP_CONFIG_FLAGS() & MITK_PP_CONFIG_MWCC()
#    define MITK_PP_TUPLE_REM_CTOR_I(ext, tuple) ext tuple
# else
#    define MITK_PP_TUPLE_REM_CTOR_I(ext, tuple) MITK_PP_TUPLE_REM_CTOR_OO((ext, tuple))
#    define MITK_PP_TUPLE_REM_CTOR_OO(par) MITK_PP_TUPLE_REM_CTOR_II ## par
#    define MITK_PP_TUPLE_REM_CTOR_II(ext, tuple) ext ## tuple
# endif
#
# endif
