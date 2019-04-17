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

#ifndef mitkStatisticsToImageRelationRule_h
#define mitkStatisticsToImageRelationRule_h

#include <MitkImageStatisticsExports.h>
#include "mitkGenericIDRelationRule.h"

namespace mitk
{
  class MITKIMAGESTATISTICS_EXPORT StatisticsToImageRelationRule : public mitk::GenericIDRelationRule
  {
  public:
    mitkClassMacroItkParent(StatisticsToImageRelationRule, mitk::GenericIDRelationRule)
    itkNewMacro(Self)

  protected:
    StatisticsToImageRelationRule();
  };
}

#endif
