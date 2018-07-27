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

#ifndef __MITK_EXTRACT_TIME_GRID_H_
#define __MITK_EXTRACT_TIME_GRID_H_

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
