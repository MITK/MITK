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

#include "mitkStatisticsToImageRelationRule.h"

mitk::StatisticsToImageRelationRule::StatisticsToImageRelationRule() : 
  GenericIDRelationRule("statisticsToImage", "relation between ImageStatisticsContainer and Image that was used as computation input", "ImageStatisticsContainer", "Image"){
}
