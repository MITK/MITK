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
