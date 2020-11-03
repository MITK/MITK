/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkServiceInterface_h
#define mitkServiceInterface_h

#include <usServiceInterface.h>

#ifdef Q_DECLARE_INTERFACE

#define MITK_DECLARE_SERVICE_INTERFACE(IFace, IId)                                                                     \
  US_DECLARE_SERVICE_INTERFACE(IFace, IId)                                                                             \
  Q_DECLARE_INTERFACE(IFace, IId)

#else

#define MITK_DECLARE_SERVICE_INTERFACE(IFace, IId) US_DECLARE_SERVICE_INTERFACE(IFace, IId)

#endif

#endif
