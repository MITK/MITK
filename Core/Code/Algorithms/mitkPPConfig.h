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
# ifndef MITK_PREPROCESSOR_CONFIG_CONFIG_HPP
# define MITK_PREPROCESSOR_CONFIG_CONFIG_HPP
#
# /* MITK_PP_CONFIG_FLAGS */
#
# define MITK_PP_CONFIG_STRICT() 0x0001
# define MITK_PP_CONFIG_IDEAL() 0x0002
#
# define MITK_PP_CONFIG_MSVC() 0x0004
# define MITK_PP_CONFIG_MWCC() 0x0008
# define MITK_PP_CONFIG_BCC() 0x0010
# define MITK_PP_CONFIG_EDG() 0x0020
# define MITK_PP_CONFIG_DMC() 0x0040
#
# ifndef MITK_PP_CONFIG_FLAGS
#    if defined(__GCCXML__)
#        define MITK_PP_CONFIG_FLAGS() (MITK_PP_CONFIG_STRICT())
#    elif defined(__WAVE__)
#        define MITK_PP_CONFIG_FLAGS() (MITK_PP_CONFIG_STRICT())
#    elif defined(__MWERKS__) && __MWERKS__ >= 0x3200
#        define MITK_PP_CONFIG_FLAGS() (MITK_PP_CONFIG_STRICT())
#    elif defined(__EDG__) || defined(__EDG_VERSION__)
#        if defined(_MSC_VER) && __EDG_VERSION__ >= 308
#            define MITK_PP_CONFIG_FLAGS() (MITK_PP_CONFIG_MSVC())
#        else
#            define MITK_PP_CONFIG_FLAGS() (MITK_PP_CONFIG_EDG() | MITK_PP_CONFIG_STRICT())
#        endif
#    elif defined(__MWERKS__)
#        define MITK_PP_CONFIG_FLAGS() (MITK_PP_CONFIG_MWCC())
#    elif defined(__DMC__)
#        define MITK_PP_CONFIG_FLAGS() (MITK_PP_CONFIG_DMC())
#    elif defined(__BORLANDC__) && __BORLANDC__ >= 0x581
#        define MITK_PP_CONFIG_FLAGS() (MITK_PP_CONFIG_STRICT())
#    elif defined(__BORLANDC__) || defined(__IBMC__) || defined(__IBMCPP__) || defined(__SUNPRO_CC)
#        define MITK_PP_CONFIG_FLAGS() (MITK_PP_CONFIG_BCC())
#    elif defined(_MSC_VER)
#        define MITK_PP_CONFIG_FLAGS() (MITK_PP_CONFIG_MSVC())
#    else
#        define MITK_PP_CONFIG_FLAGS() (MITK_PP_CONFIG_STRICT())
#    endif
# endif
#
# /* MITK_PP_CONFIG_EXTENDED_LINE_INFO */
#
# ifndef MITK_PP_CONFIG_EXTENDED_LINE_INFO
#    define MITK_PP_CONFIG_EXTENDED_LINE_INFO 0
# endif
#
# /* MITK_PP_CONFIG_ERRORS */
#
# ifndef MITK_PP_CONFIG_ERRORS
#    ifdef NDEBUG
#        define MITK_PP_CONFIG_ERRORS 0
#    else
#        define MITK_PP_CONFIG_ERRORS 1
#    endif
# endif
#
# endif
