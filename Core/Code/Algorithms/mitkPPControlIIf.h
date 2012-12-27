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
# ifndef MITK_PREPROCESSOR_CONTROL_IIF_HPP
# define MITK_PREPROCESSOR_CONTROL_IIF_HPP
#
# include "mitkPPConfig.h"
#
# if ~MITK_PP_CONFIG_FLAGS() & MITK_PP_CONFIG_MWCC()
#    define MITK_PP_IIF(bit, t, f) MITK_PP_IIF_I(bit, t, f)
# else
#    define MITK_PP_IIF(bit, t, f) MITK_PP_IIF_OO((bit, t, f))
#    define MITK_PP_IIF_OO(par) MITK_PP_IIF_I ## par
# endif
#
# if ~MITK_PP_CONFIG_FLAGS() & MITK_PP_CONFIG_MSVC()
#    define MITK_PP_IIF_I(bit, t, f) MITK_PP_IIF_ ## bit(t, f)
# else
#    define MITK_PP_IIF_I(bit, t, f) MITK_PP_IIF_II(MITK_PP_IIF_ ## bit(t, f))
#    define MITK_PP_IIF_II(id) id
# endif
#
# define MITK_PP_IIF_0(t, f) f
# define MITK_PP_IIF_1(t, f) t
#
# endif
