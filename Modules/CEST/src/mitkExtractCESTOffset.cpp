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

#include "mitkExtractCESTOffset.h"
#include "mitkCustomTagParser.h"

#include <iterator>
#include <regex>


std::vector<mitk::ScalarType> mitk::ExtractCESTT1Time(const BaseData* image)
{
    std::vector<ScalarType> result;

    auto prop = image->GetProperty(CEST_PROPERTY_NAME_TREC().c_str());
    if (prop.IsNotNull())
    {
        auto valueStr = prop->GetValueAsString();

        std::istringstream iss;
        iss.imbue(std::locale("C"));
        iss.str(valueStr);
        double d;

        while (iss >> d)
        {
          if (!iss.fail())
          {
            result.emplace_back(d);
          }
        }

        if (result.size() != image->GetTimeSteps()) mitkThrow() << "Cannot determine T1 times. Selected input has an property \"" << CEST_PROPERTY_NAME_TREC() << "\" that don't match the number of time steps of input.";

        for (auto& value : result)
        {
            value *= 0.001;
        }
    }
    else mitkThrow() << "Cannot determine T1 time grid (TREC). Selected input has no property \"" << CEST_PROPERTY_NAME_TREC() << "\"";

    return result;
}

std::vector<mitk::ScalarType> mitk::ExtractCESTOffset(const BaseData* image)
{
  std::vector<ScalarType> result;

  auto prop = image->GetProperty(CEST_PROPERTY_NAME_OFFSETS().c_str());
  if (prop.IsNotNull())
  {
    auto valueStr = prop->GetValueAsString();

    std::istringstream iss;
    iss.imbue(std::locale("C"));
    iss.str(valueStr);
    double d;

    while (iss >> d)
    {
      if (!iss.fail())
      {
        result.emplace_back(d);
      }
    }

    if (result.size() != image->GetTimeSteps()) mitkThrow() << "Cannot determine offset. Selected input has an property \"" << CEST_PROPERTY_NAME_OFFSETS() << "\" that don't match the number of time steps of input.";

  }
  else mitkThrow() << "Cannot determine frequency. Selected input has no property \"" << CEST_PROPERTY_NAME_OFFSETS() << "\"";

  return result;
}
