/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef __CEST_PROERTY_HELPER_H
#define __CEST_PROERTY_HELPER_H

#include "mitkIPropertyProvider.h"
#include "mitkIPropertyOwner.h"

#include "MitkCESTExports.h"

namespace mitk
{
  const std::string MITKCEST_EXPORT CEST_PROPERTY_NAME_PREPERATIONTYPE();
  const std::string MITKCEST_EXPORT CEST_PROPERTY_NAME_RECOVERYMODE();
  const std::string MITKCEST_EXPORT CEST_PROPERTY_NAME_SPOILINGTYPE();
  const std::string MITKCEST_EXPORT CEST_PROPERTY_NAME_OFFSETS();
  const std::string MITKCEST_EXPORT CEST_PROPERTY_NAME_TREC();

  const std::string MITKCEST_EXPORT CEST_PROPERTY_NAME_FREQ();
  const std::string MITKCEST_EXPORT CEST_PROPERTY_NAME_PULSEDURATION();
  const std::string MITKCEST_EXPORT CEST_PROPERTY_NAME_B1Amplitude();
  const std::string MITKCEST_EXPORT CEST_PROPERTY_NAME_DutyCycle();

  /**Helper function that gets the CEST B1 amplitude property ("CEST.B1Amplitude") from the passed property provider.
  If it is not possible to generate/get the value an mitk::Exception will be thrown.*/
  double MITKCEST_EXPORT GetCESTB1Amplitude(const IPropertyProvider* provider);

  /**Helper function that gets the CEST frequency property ("CEST.FREQ") from the input image.
  If it is not possible to generate/get the value an mitk::Exception will be thrown.
  The value is returned in [MHz]. Normally in the property it is stored in [Hz].*/
  double MITKCEST_EXPORT GetCESTFrequency(const IPropertyProvider* provider);

  /**Helper function that sets the CEST frequency property ("CEST.FREQ") in the passed owner.
  If it owner is nullptr nothing will be done.
  The value is passed in [MHz] and set in the property in [Hz].*/
  void MITKCEST_EXPORT SetCESTFrequencyMHz(IPropertyOwner* owner, double freqInMHz);

  /**Helper function that gets the CEST pulse duration property ("CEST.PulseDuration") from the input image.
  If it is not possible to generate/get the value an mitk::Exception will be thrown.
  The value is returned in [s]. Normally in the property it is stored in micro secs.*/
  double MITKCEST_EXPORT GetCESTPulseDuration(const IPropertyProvider* provider);

  /**Helper function that gets the CEST duty cycle property ("CEST.DutyCycle") from the input image.
  If it is not possible to generate/get the value an mitk::Exception will be thrown.
  The value is returned as scaling factor (1 == 100%), in contrast to the porperty where it is stored as
  a percentage value (e.g. 56 %, so the function return will be 0.56).*/
  double MITKCEST_EXPORT GetCESTDutyCycle(const IPropertyProvider* provider);
}

#endif // __CEST_PROERTY_HELPER_H
