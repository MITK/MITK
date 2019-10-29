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

#ifndef mitkStatisticsToMaskRelationRule_h
#define mitkStatisticsToMaskRelationRule_h

#include <MitkImageStatisticsExports.h>
#include "mitkGenericIDRelationRule.h"

namespace mitk
{
  class MITKIMAGESTATISTICS_EXPORT StatisticsToMaskRelationRule : public mitk::GenericIDRelationRule
  {
  public:
    mitkClassMacroItkParent(StatisticsToMaskRelationRule, mitk::GenericIDRelationRule)
    itkNewMacro(Self)

  protected:
    StatisticsToMaskRelationRule();
  };
}

#endif
