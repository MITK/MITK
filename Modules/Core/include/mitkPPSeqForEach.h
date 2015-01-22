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
# ifndef MITK_PREPROCESSOR_SEQ_FOR_EACH_HPP
# define MITK_PREPROCESSOR_SEQ_FOR_EACH_HPP
#
# include "mitkPPArithmeticDec.h"
# include "mitkPPConfig.h"
# include "mitkPPRepetitionFor.h"
# include "mitkPPSeq.h"
# include "mitkPPSeqSize.h"
# include "mitkPPTupleElem.h"
# include "mitkPPTupleRem.h"
#
# /* MITK_PP_SEQ_FOR_EACH */
#
# if ~MITK_PP_CONFIG_FLAGS() & MITK_PP_CONFIG_EDG()
#    define MITK_PP_SEQ_FOR_EACH(macro, data, seq) MITK_PP_FOR((macro, data, seq (nil)), MITK_PP_SEQ_FOR_EACH_P, MITK_PP_SEQ_FOR_EACH_O, MITK_PP_SEQ_FOR_EACH_M)
# else
#    define MITK_PP_SEQ_FOR_EACH(macro, data, seq) MITK_PP_SEQ_FOR_EACH_D(macro, data, seq)
#    define MITK_PP_SEQ_FOR_EACH_D(macro, data, seq) MITK_PP_FOR((macro, data, seq (nil)), MITK_PP_SEQ_FOR_EACH_P, MITK_PP_SEQ_FOR_EACH_O, MITK_PP_SEQ_FOR_EACH_M)
# endif
#
# define MITK_PP_SEQ_FOR_EACH_P(r, x) MITK_PP_DEC(MITK_PP_SEQ_SIZE(MITK_PP_TUPLE_ELEM(3, 2, x)))
#
# if MITK_PP_CONFIG_FLAGS() & MITK_PP_CONFIG_STRICT()
#    define MITK_PP_SEQ_FOR_EACH_O(r, x) MITK_PP_SEQ_FOR_EACH_O_I x
# else
#    define MITK_PP_SEQ_FOR_EACH_O(r, x) MITK_PP_SEQ_FOR_EACH_O_I(MITK_PP_TUPLE_ELEM(3, 0, x), MITK_PP_TUPLE_ELEM(3, 1, x), MITK_PP_TUPLE_ELEM(3, 2, x))
# endif
#
# define MITK_PP_SEQ_FOR_EACH_O_I(macro, data, seq) (macro, data, MITK_PP_SEQ_TAIL(seq))
#
# if MITK_PP_CONFIG_FLAGS() & MITK_PP_CONFIG_STRICT()
#    define MITK_PP_SEQ_FOR_EACH_M(r, x) MITK_PP_SEQ_FOR_EACH_M_IM(r, MITK_PP_TUPLE_REM_3 x)
#    define MITK_PP_SEQ_FOR_EACH_M_IM(r, im) MITK_PP_SEQ_FOR_EACH_M_I(r, im)
# else
#    define MITK_PP_SEQ_FOR_EACH_M(r, x) MITK_PP_SEQ_FOR_EACH_M_I(r, MITK_PP_TUPLE_ELEM(3, 0, x), MITK_PP_TUPLE_ELEM(3, 1, x), MITK_PP_TUPLE_ELEM(3, 2, x))
# endif
#
# define MITK_PP_SEQ_FOR_EACH_M_I(r, macro, data, seq) macro(r, data, MITK_PP_SEQ_HEAD(seq))
#
# /* MITK_PP_SEQ_FOR_EACH_R */
#
# if ~MITK_PP_CONFIG_FLAGS() & MITK_PP_CONFIG_EDG()
#    define MITK_PP_SEQ_FOR_EACH_R(r, macro, data, seq) MITK_PP_FOR_ ## r((macro, data, seq (nil)), MITK_PP_SEQ_FOR_EACH_P, MITK_PP_SEQ_FOR_EACH_O, MITK_PP_SEQ_FOR_EACH_M)
# else
#    define MITK_PP_SEQ_FOR_EACH_R(r, macro, data, seq) MITK_PP_SEQ_FOR_EACH_R_I(r, macro, data, seq)
#    define MITK_PP_SEQ_FOR_EACH_R_I(r, macro, data, seq) MITK_PP_FOR_ ## r((macro, data, seq (nil)), MITK_PP_SEQ_FOR_EACH_P, MITK_PP_SEQ_FOR_EACH_O, MITK_PP_SEQ_FOR_EACH_M)
# endif
#
# endif
