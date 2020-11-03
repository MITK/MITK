/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkStatisticsToImageRelationRule_h
#define mitkStatisticsToImageRelationRule_h

#include <MitkImageStatisticsExports.h>
#include "mitkGenericIDRelationRule.h"

namespace mitk
{
  class MITKIMAGESTATISTICS_EXPORT StatisticsToImageRelationRule : public mitk::GenericIDRelationRule
  {
  public:
    mitkClassMacro(StatisticsToImageRelationRule, mitk::GenericIDRelationRule);
    itkNewMacro(Self);

  protected:
    StatisticsToImageRelationRule();
  };
}

#endif
