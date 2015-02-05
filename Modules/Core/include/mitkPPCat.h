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
# ifndef MITK_PREPROCESSOR_CAT_HPP
# define MITK_PREPROCESSOR_CAT_HPP
#
# include "mitkPPConfig.h"
#
# /* MITK_PP_CAT */
#
# if ~MITK_PP_CONFIG_FLAGS() & MITK_PP_CONFIG_MWCC()
#    define MITK_PP_CAT(a, b) MITK_PP_CAT_I(a, b)
# else
#    define MITK_PP_CAT(a, b) MITK_PP_CAT_OO((a, b))
#    define MITK_PP_CAT_OO(par) MITK_PP_CAT_I ## par
# endif
#
# if ~MITK_PP_CONFIG_FLAGS() & MITK_PP_CONFIG_MSVC()
#    define MITK_PP_CAT_I(a, b) a ## b
# else
#    define MITK_PP_CAT_I(a, b) MITK_PP_CAT_II(a ## b)
#    define MITK_PP_CAT_II(res) res
# endif
#
# endif
