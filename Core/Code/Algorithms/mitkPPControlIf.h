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
# ifndef MITK_PREPROCESSOR_CONTROL_IF_HPP
# define MITK_PREPROCESSOR_CONTROL_IF_HPP
#
# include "mitkPPConfig.h"
# include "mitkPPControlIIf.h"
# include "mitkPPLogicalBool.h"
#
# /* MITK_PP_IF */
#
# if ~MITK_PP_CONFIG_FLAGS() & MITK_PP_CONFIG_EDG()
#    define MITK_PP_IF(cond, t, f) MITK_PP_IIF(MITK_PP_BOOL(cond), t, f)
# else
#    define MITK_PP_IF(cond, t, f) MITK_PP_IF_I(cond, t, f)
#    define MITK_PP_IF_I(cond, t, f) MITK_PP_IIF(MITK_PP_BOOL(cond), t, f)
# endif
#
# endif
