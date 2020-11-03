/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkStatisticsToMaskRelationRule.h"

mitk::StatisticsToMaskRelationRule::StatisticsToMaskRelationRule() :
  GenericIDRelationRule("statisticsToMask", "relation between ImageStatisticsContainer and Mask that was used as computation input", "ImageStatisticsContainer", "Mask") {
}
