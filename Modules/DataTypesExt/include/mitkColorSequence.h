/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef mitkColorSequence_h
#define mitkColorSequence_h

#include "MitkDataTypesExtExports.h"
#include <mitkColorProperty.h>

namespace mitk
{
  /*!
    \brief Inferface for creating a sequence of nice/matching/appropriate/... colors.

    See derived classes for implemented sequences.
  */
  class MITKDATATYPESEXT_EXPORT ColorSequence
  {
  public:
    ColorSequence();
    virtual ~ColorSequence();

    /*!
    \brief Return another color
    */
    virtual Color GetNextColor() = 0;

    /*!
    \brief Set the color-index to begin again
    */
    virtual void GoToBegin() = 0;
  };
}

#endif
