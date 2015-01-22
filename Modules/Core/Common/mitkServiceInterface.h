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

#ifndef mitkServiceInterface_h
#define mitkServiceInterface_h

#include <usServiceInterface.h>

#ifdef Q_DECLARE_INTERFACE

#define MITK_DECLARE_SERVICE_INTERFACE(IFace, IId) \
  US_DECLARE_SERVICE_INTERFACE(IFace, IId)         \
  Q_DECLARE_INTERFACE(IFace, IId)

#else

#define MITK_DECLARE_SERVICE_INTERFACE(IFace, IId) US_DECLARE_SERVICE_INTERFACE(IFace, IId)

#endif

#endif
