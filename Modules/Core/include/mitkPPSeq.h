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
# ifndef MITK_PREPROCESSOR_SEQ_SEQ_HPP
# define MITK_PREPROCESSOR_SEQ_SEQ_HPP
#
# include "mitkPPConfig.h"
# include "mitkPPSeqElem.h"
#
# /* MITK_PP_SEQ_HEAD */
#
# define MITK_PP_SEQ_HEAD(seq) MITK_PP_SEQ_ELEM(0, seq)
#
# /* MITK_PP_SEQ_TAIL */
#
# if MITK_PP_CONFIG_FLAGS() & MITK_PP_CONFIG_MWCC()
#    define MITK_PP_SEQ_TAIL(seq) MITK_PP_SEQ_TAIL_1((seq))
#    define MITK_PP_SEQ_TAIL_1(par) MITK_PP_SEQ_TAIL_2 ## par
#    define MITK_PP_SEQ_TAIL_2(seq) MITK_PP_SEQ_TAIL_I ## seq
# elif MITK_PP_CONFIG_FLAGS() & MITK_PP_CONFIG_MSVC()
#    define MITK_PP_SEQ_TAIL(seq) MITK_PP_SEQ_TAIL_ID(MITK_PP_SEQ_TAIL_I seq)
#    define MITK_PP_SEQ_TAIL_ID(id) id
# elif MITK_PP_CONFIG_FLAGS() & MITK_PP_CONFIG_EDG()
#    define MITK_PP_SEQ_TAIL(seq) MITK_PP_SEQ_TAIL_D(seq)
#    define MITK_PP_SEQ_TAIL_D(seq) MITK_PP_SEQ_TAIL_I seq
# else
#    define MITK_PP_SEQ_TAIL(seq) MITK_PP_SEQ_TAIL_I seq
# endif
#
# define MITK_PP_SEQ_TAIL_I(x)
#
# /* MITK_PP_SEQ_NIL */
#
# define MITK_PP_SEQ_NIL(x) (x)
#
# endif
