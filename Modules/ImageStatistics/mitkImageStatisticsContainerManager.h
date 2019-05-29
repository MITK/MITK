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
#ifndef QmitkImageStatisticsContainerManager_H__INCLUDED
#define QmitkImageStatisticsContainerManager_H__INCLUDED

#include "MitkImageStatisticsExports.h"

#include <mitkDataStorage.h>
#include <mitkImageStatisticsContainer.h>
#include <mitkBaseData.h>
#include <mitkNodePredicateBase.h>
#include <mitkGenericIDRelationRule.h>
#include <mitkPropertyRelations.h>

namespace mitk
{
  /**
  \brief Returns the StatisticsContainer that was computed on given input (image/mask/planar figure) and is added as DataNode in a DataStorage
  */
  class MITKIMAGESTATISTICS_EXPORT ImageStatisticsContainerManager
  {
  public:
    /**Documentation
    @brief Returns the StatisticContainer for the given image and mask
    @return a valid StatisticsContainer or nullptr if no StatisticContainer is found or no rules have been defined
    @details if more than one StatisticsContainer is found, only the newest (ModifiedTime) is returned
    @pre Datastorage must point to a valid instance.
    @pre image must Point to a valid instance.
    */
    static mitk::ImageStatisticsContainer::ConstPointer GetImageStatistics(const mitk::DataStorage* dataStorage, const mitk::BaseData* image, const mitk::BaseData* mask=nullptr);

  protected:
    static mitk::NodePredicateBase::ConstPointer GetPredicateForSources(const mitk::BaseData* image, const mitk::BaseData* mask = nullptr);
  };
}
#endif
