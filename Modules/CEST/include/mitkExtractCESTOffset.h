/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkExtractCESTOffset_h
#define mitkExtractCESTOffset_h

#include <mitkBaseData.h>

#include <MitkCESTExports.h>

namespace mitk
{
  /**
  \brief Extract CEST frequency offsets from an image's properties as a vector.

  Reads the "CEST.Offsets" property from the input image, parses the space-separated
  offset values, and returns them as a vector of ScalarType. Offsets with absolute
  value > 299 indicate M0 normalization images.

  \param[in] image Pointer to the BaseData (typically an mitk::Image) from which to
             extract the offsets.
  \return A vector of offset values in ppm, one per timestep.
  \post The number of extracted offsets equals the number of timesteps of the image.
  \throw mitk::Exception if the image has no "CEST.Offsets" property.
  \throw mitk::Exception if the number of parsed offsets does not match the number of timesteps.
  \sa CEST_PROPERTY_NAME_OFFSETS, CESTImageNormalizationFilter
  */
  MITKCEST_EXPORT std::vector<ScalarType> ExtractCESTOffset(const BaseData* image);

  /**
  \brief Extract CEST T1 recovery times from an image's properties as a vector.

  Reads the "CEST.TREC" property from the input image, parses the space-separated
  time values (stored in milliseconds), converts them to seconds (by multiplying
  with 0.001), and returns them as a vector of ScalarType.

  \param[in] image Pointer to the BaseData (typically an mitk::Image) from which to
             extract the T1 recovery times.
  \return A vector of T1 recovery times in seconds, one per timestep.
  \post The number of extracted T1 times equals the number of timesteps of the image.
  \throw mitk::Exception if the image has no "CEST.TREC" property.
  \throw mitk::Exception if the number of parsed T1 times does not match the number of timesteps.
  \sa CEST_PROPERTY_NAME_TREC, CustomTagParser
  */
  MITKCEST_EXPORT std::vector<ScalarType> ExtractCESTT1Time(const BaseData* image);
}

#endif
