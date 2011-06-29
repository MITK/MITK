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
# ifndef MITK_PREPROCESSOR_SEQ_TO_TUPLE_HPP
# define MITK_PREPROCESSOR_SEQ_TO_TUPLE_HPP
#
# include "mitkPPConfig.h"
# include "mitkPPSeqEnum.h"
#
# /* MITK_PP_SEQ_TO_TUPLE */
#
# if ~MITK_PP_CONFIG_FLAGS() & MITK_PP_CONFIG_EDG()
#    define MITK_PP_SEQ_TO_TUPLE(seq) (MITK_PP_SEQ_ENUM(seq))
# else
#    define MITK_PP_SEQ_TO_TUPLE(seq) MITK_PP_SEQ_TO_TUPLE_I(seq)
#    define MITK_PP_SEQ_TO_TUPLE_I(seq) (MITK_PP_SEQ_ENUM(seq))
# endif
#
# endif
