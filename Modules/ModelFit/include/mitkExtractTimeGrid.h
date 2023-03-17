/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkExtractTimeGrid_h
#define mitkExtractTimeGrid_h

#include <iostream>

#include <mitkImage.h>
#include <mitkTimeGeometry.h>

#include "mitkModelBase.h"

#include "MitkModelFitExports.h"

namespace mitk
{

  /*Extracts the time grid of an image (its time geometry) for usage with models.
   * Time grid is empty (size == 0) if image pointer is invalid.
   * @remark the model time grid has a resolution in sec and not ms like the time geometry.*/
  MITKMODELFIT_EXPORT ModelBase::TimeGridType ExtractTimeGrid(const Image* image);

  /*Extracts the time grid of a time geometry for usage with models.
   * Time grid is empty (size == 0) if geometry pointer is invalid.
   * @remark the model time grid has a resolution in sec and not ms like the time geometry.*/
  MITKMODELFIT_EXPORT ModelBase::TimeGridType ExtractTimeGrid(const TimeGeometry* geometry);

}

#endif
