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
# /* Copyright (C) 2001
#  * Housemarque Oy
#  * http://www.housemarque.com
#  *
#  * Distributed under the Boost Software License, Version 1.0. (See
#  * accompanying file LICENSE_1_0.txt or copy at
#  * http://www.boost.org/LICENSE_1_0.txt)
#  */
#
# /* Revised by Paul Mensonides (2002) */
#
# /* See http://www.boost.org for most recent version. */
#
# ifndef MITK_PREPROCESSOR_TUPLE_EAT_HPP
# define MITK_PREPROCESSOR_TUPLE_EAT_HPP
#
# include "mitkPPConfig.h"
#
# /* MITK_PP_TUPLE_EAT */
#
# if ~MITK_PP_CONFIG_FLAGS() & MITK_PP_CONFIG_MWCC()
#    define MITK_PP_TUPLE_EAT(size) MITK_PP_TUPLE_EAT_I(size)
# else
#    define MITK_PP_TUPLE_EAT(size) MITK_PP_TUPLE_EAT_OO((size))
#    define MITK_PP_TUPLE_EAT_OO(par) MITK_PP_TUPLE_EAT_I ## par
# endif
#
# define MITK_PP_TUPLE_EAT_I(size) MITK_PP_TUPLE_EAT_ ## size
#
# define MITK_PP_TUPLE_EAT_0()
# define MITK_PP_TUPLE_EAT_1(a)
# define MITK_PP_TUPLE_EAT_2(a, b)
# define MITK_PP_TUPLE_EAT_3(a, b, c)
# define MITK_PP_TUPLE_EAT_4(a, b, c, d)
# define MITK_PP_TUPLE_EAT_5(a, b, c, d, e)
# define MITK_PP_TUPLE_EAT_6(a, b, c, d, e, f)
# define MITK_PP_TUPLE_EAT_7(a, b, c, d, e, f, g)
# define MITK_PP_TUPLE_EAT_8(a, b, c, d, e, f, g, h)
# define MITK_PP_TUPLE_EAT_9(a, b, c, d, e, f, g, h, i)
# define MITK_PP_TUPLE_EAT_10(a, b, c, d, e, f, g, h, i, j)
# define MITK_PP_TUPLE_EAT_11(a, b, c, d, e, f, g, h, i, j, k)
# define MITK_PP_TUPLE_EAT_12(a, b, c, d, e, f, g, h, i, j, k, l)
# define MITK_PP_TUPLE_EAT_13(a, b, c, d, e, f, g, h, i, j, k, l, m)
# define MITK_PP_TUPLE_EAT_14(a, b, c, d, e, f, g, h, i, j, k, l, m, n)
# define MITK_PP_TUPLE_EAT_15(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o)
# define MITK_PP_TUPLE_EAT_16(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p)
# define MITK_PP_TUPLE_EAT_17(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q)
# define MITK_PP_TUPLE_EAT_18(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r)
# define MITK_PP_TUPLE_EAT_19(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s)
# define MITK_PP_TUPLE_EAT_20(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t)
# define MITK_PP_TUPLE_EAT_21(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u)
# define MITK_PP_TUPLE_EAT_22(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v)
# define MITK_PP_TUPLE_EAT_23(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w)
# define MITK_PP_TUPLE_EAT_24(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x)
# define MITK_PP_TUPLE_EAT_25(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y)
#
# endif
