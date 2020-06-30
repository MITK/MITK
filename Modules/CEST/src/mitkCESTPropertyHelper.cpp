/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkCESTPropertyHelper.h"

#include "mitkDICOMProperty.h"
#include "mitkStringProperty.h"

const std::string mitk::CEST_PROPERTY_NAME_PREPERATIONTYPE()
{
  return "CEST.PreparationType";
}

const std::string mitk::CEST_PROPERTY_NAME_RECOVERYMODE()
{
  return "CEST.RecoveryMode";
}

const std::string mitk::CEST_PROPERTY_NAME_SPOILINGTYPE()
{
  return "CEST.SpoilingType";
}

const std::string mitk::CEST_PROPERTY_NAME_OFFSETS()
{
  return "CEST.Offsets";
}

const std::string mitk::CEST_PROPERTY_NAME_TREC()
{
  return std::string("CEST.TREC");
}

const std::string mitk::CEST_PROPERTY_NAME_FREQ()
{
  return std::string("CEST.FREQ");
}

const std::string mitk::CEST_PROPERTY_NAME_PULSEDURATION()
{
  return std::string("CEST.PulseDuration");
}

const std::string mitk::CEST_PROPERTY_NAME_B1Amplitude()
{
  return std::string("CEST.B1Amplitude");
}

const std::string mitk::CEST_PROPERTY_NAME_DutyCycle()
{
  return std::string("CEST.DutyCycle");
}


double mitk::GetCESTB1Amplitude(const IPropertyProvider* provider)
{
  if (!provider)
  {
    mitkThrow() << "Cannot determine B! amplitude. Passed property provider is invalid.";
  }

  double result = 0.0;

  auto prop = provider->GetConstProperty(CEST_PROPERTY_NAME_B1Amplitude().c_str());
  if (prop.IsNotNull())
  {
    result = mitk::ConvertDICOMStrToValue<double>(prop->GetValueAsString());
  }
  else mitkThrow() << "Cannot determine B! amplitude. Selected input has no property \"" << CEST_PROPERTY_NAME_B1Amplitude() << "\"";

  return result;
}

double mitk::GetCESTFrequency(const IPropertyProvider* provider)
{
  if (!provider)
  {
    mitkThrow() << "Cannot determine frequency. Passed property provider is invalid.";
  }

  double result = 0.0;

  auto prop = provider->GetConstProperty(CEST_PROPERTY_NAME_FREQ().c_str());
  if (prop.IsNotNull())
  {
    result = mitk::ConvertDICOMStrToValue<double>(prop->GetValueAsString()) * 0.000001;
  }
  else mitkThrow() << "Cannot determine frequency. Selected input has no property \"" << CEST_PROPERTY_NAME_FREQ() << "\"";

  return result;
}

void mitk::SetCESTFrequencyMHz(IPropertyOwner* owner, double freqInMHz)
{
  if (nullptr != owner)
  {
    owner->SetProperty(CEST_PROPERTY_NAME_FREQ().c_str(), mitk::StringProperty::New(ConvertValueToDICOMStr(freqInMHz * 1000000)));
  }
}

double mitk::GetCESTPulseDuration(const IPropertyProvider* provider)
{
  if (!provider)
  {
    mitkThrow() << "Cannot determine pulse duration. Passed property provider is invalid.";
  }

  double result = 0.0;

  auto prop = provider->GetConstProperty(CEST_PROPERTY_NAME_PULSEDURATION().c_str());
  if (prop.IsNotNull())
  {
    result = mitk::ConvertDICOMStrToValue<double>(prop->GetValueAsString()) * 0.000001;
  }
  else mitkThrow() << "Cannot determine pulse duration. Selected input has no property \"" << CEST_PROPERTY_NAME_PULSEDURATION() << "\"";

  return result;
}

double mitk::GetCESTDutyCycle(const IPropertyProvider* provider)
{
  if (!provider)
  {
    mitkThrow() << "Cannot determine duty cycle. Passed property provider is invalid.";
  }

  double result = 0.0;

  auto prop = provider->GetConstProperty(CEST_PROPERTY_NAME_DutyCycle().c_str());
  if (prop.IsNotNull())
  {
    result = mitk::ConvertDICOMStrToValue<double>(prop->GetValueAsString()) * 0.01;
  }
  else mitkThrow() << "Cannot determine duty cycle. Selected input has no property \"" << CEST_PROPERTY_NAME_DutyCycle() << "\"";

  return result;
}
