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
# ifndef MITK_PREPROCESSOR_STRINGIZE_HPP
# define MITK_PREPROCESSOR_STRINGIZE_HPP
#
# include "mitkPPConfig.h"
#
# /* MITK_PP_STRINGIZE */
#
# if MITK_PP_CONFIG_FLAGS() & MITK_PP_CONFIG_MSVC()
#    define MITK_PP_STRINGIZE(text) MITK_PP_STRINGIZE_A((text))
#    define MITK_PP_STRINGIZE_A(arg) MITK_PP_STRINGIZE_I arg
# elif MITK_PP_CONFIG_FLAGS() & MITK_PP_CONFIG_MWCC()
#    define MITK_PP_STRINGIZE(text) MITK_PP_STRINGIZE_OO((text))
#    define MITK_PP_STRINGIZE_OO(par) MITK_PP_STRINGIZE_I ## par
# else
#    define MITK_PP_STRINGIZE(text) MITK_PP_STRINGIZE_I(text)
# endif
#
# define MITK_PP_STRINGIZE_I(text) #text
#
# endif
