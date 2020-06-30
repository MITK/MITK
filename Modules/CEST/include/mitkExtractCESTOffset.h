/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef __MITK_EXTRACT_CEST_OFFSET_H_
#define __MITK_EXTRACT_CEST_OFFSET_H_

#include <mitkBaseData.h>

#include "MitkCESTExports.h"

namespace mitk
{
  /**Helper function that gets the CEST offset property ("CEST.Offsets") from the input
  image as vector of ScalarType.
  If it is not possible to generate/get the offset an mitk::Excpetion will be thrown.
  The values of the vector are in [ppm].
  @post Number of extracted offsets equal the number of timesteps of the image.
  */
  MITKCEST_EXPORT std::vector<ScalarType> ExtractCESTOffset(const BaseData* image);

  /**Helper function that gets the CEST offset property ("CEST.TREC") from the input image as vector of ScalarType.
  If it is not possible to generate/get the T1 times an mitk::Excpetion will be thrown.
  The values of the vector are in [sec]. In the property they are stored in [ms] and scaled appropriately
  before returning.
  @post Number of extracted T1 times equal the number of timesteps of the image.
  */
  MITKCEST_EXPORT std::vector<ScalarType> ExtractCESTT1Time(const BaseData* image);
}

#endif
