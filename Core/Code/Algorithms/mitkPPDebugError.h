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
# ifndef MITK_PREPROCESSOR_DEBUG_ERROR_HPP
# define MITK_PREPROCESSOR_DEBUG_ERROR_HPP
#
# include "mitkPPCat.h"
# include "mitkPPConfig.h"
#
# /* MITK_PP_ERROR */
#
# if MITK_PP_CONFIG_ERRORS
#    define MITK_PP_ERROR(code) MITK_PP_CAT(MITK_PP_ERROR_, code)
# endif
#
# define MITK_PP_ERROR_0x0000 MITK_PP_ERROR(0x0000, MITK_PP_INDEX_OUT_OF_BOUNDS)
# define MITK_PP_ERROR_0x0001 MITK_PP_ERROR(0x0001, MITK_PP_WHILE_OVERFLOW)
# define MITK_PP_ERROR_0x0002 MITK_PP_ERROR(0x0002, MITK_PP_FOR_OVERFLOW)
# define MITK_PP_ERROR_0x0003 MITK_PP_ERROR(0x0003, MITK_PP_REPEAT_OVERFLOW)
# define MITK_PP_ERROR_0x0004 MITK_PP_ERROR(0x0004, MITK_PP_LIST_FOLD_OVERFLOW)
# define MITK_PP_ERROR_0x0005 MITK_PP_ERROR(0x0005, MITK_PP_SEQ_FOLD_OVERFLOW)
# define MITK_PP_ERROR_0x0006 MITK_PP_ERROR(0x0006, MITK_PP_ARITHMETIC_OVERFLOW)
# define MITK_PP_ERROR_0x0007 MITK_PP_ERROR(0x0007, MITK_PP_DIVISION_BY_ZERO)
#
# endif
