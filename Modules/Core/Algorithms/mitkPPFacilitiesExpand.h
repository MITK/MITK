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
# ifndef MITK_PREPROCESSOR_FACILITIES_EXPAND_HPP
# define MITK_PREPROCESSOR_FACILITIES_EXPAND_HPP
#
# include "mitkPPConfig.h"
#
# if ~MITK_PP_CONFIG_FLAGS() & MITK_PP_CONFIG_MWCC() && ~MITK_PP_CONFIG_FLAGS() & MITK_PP_CONFIG_DMC()
#    define MITK_PP_EXPAND(x) MITK_PP_EXPAND_I(x)
# else
#    define MITK_PP_EXPAND(x) MITK_PP_EXPAND_OO((x))
#    define MITK_PP_EXPAND_OO(par) MITK_PP_EXPAND_I ## par
# endif
#
# define MITK_PP_EXPAND_I(x) x
#
# endif
