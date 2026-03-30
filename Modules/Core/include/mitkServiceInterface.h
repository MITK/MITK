/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

/**
 * \file mitkServiceInterface.h
 * \brief Provides the MITK_DECLARE_SERVICE_INTERFACE macro for registering C++ interfaces
 *        with both the CppMicroServices framework and (optionally) the Qt plugin system.
 *
 * When Qt's \c Q_DECLARE_INTERFACE is available (i.e. Qt headers have been included before
 * this header), the macro registers the interface with both \c US_DECLARE_SERVICE_INTERFACE
 * and \c Q_DECLARE_INTERFACE. Otherwise, only the CppMicroServices registration is performed.
 *
 * \ingroup Core
 */

#ifndef mitkServiceInterface_h
#define mitkServiceInterface_h

#include <usServiceInterface.h>

#ifdef Q_DECLARE_INTERFACE

/**
 * \brief Declare a C++ interface for use with CppMicroServices and Qt's plugin system.
 *
 * When Qt is available, this macro expands to both \c US_DECLARE_SERVICE_INTERFACE and
 * \c Q_DECLARE_INTERFACE. Otherwise, only the CppMicroServices declaration is emitted.
 *
 * \param IFace The fully qualified C++ interface class name.
 * \param IId   A unique string identifier for the interface (e.g. "org.mitk.IMyService").
 */
#define MITK_DECLARE_SERVICE_INTERFACE(IFace, IId)                                                                     \
  US_DECLARE_SERVICE_INTERFACE(IFace, IId)                                                                             \
  Q_DECLARE_INTERFACE(IFace, IId)

#else

/**
 * \brief Declare a C++ interface for use with CppMicroServices (Qt not available).
 *
 * \param IFace The fully qualified C++ interface class name.
 * \param IId   A unique string identifier for the interface.
 */
#define MITK_DECLARE_SERVICE_INTERFACE(IFace, IId) US_DECLARE_SERVICE_INTERFACE(IFace, IId)

#endif

#endif
