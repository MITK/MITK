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
# ifndef MITK_PREPROCESSOR_CONTROL_EXPR_IIF_HPP
# define MITK_PREPROCESSOR_CONTROL_EXPR_IIF_HPP
#
# include "mitkPPConfig.h"
#
# /* MITK_PP_EXPR_IIF */
#
# if ~MITK_PP_CONFIG_FLAGS() & MITK_PP_CONFIG_MWCC()
#    define MITK_PP_EXPR_IIF(bit, expr) MITK_PP_EXPR_IIF_I(bit, expr)
# else
#    define MITK_PP_EXPR_IIF(bit, expr) MITK_PP_EXPR_IIF_OO((bit, expr))
#    define MITK_PP_EXPR_IIF_OO(par) MITK_PP_EXPR_IIF_I ## par
# endif
#
# define MITK_PP_EXPR_IIF_I(bit, expr) MITK_PP_EXPR_IIF_ ## bit(expr)
#
# define MITK_PP_EXPR_IIF_0(expr)
# define MITK_PP_EXPR_IIF_1(expr) expr
#
# endif
