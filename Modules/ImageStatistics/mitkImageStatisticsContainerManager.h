/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
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
    @return a valid StatisticsContainer or nullptr if no StatisticContainer is found.
    @details if more than one StatisticsContainer is found, only the newest (ModifiedTime) is returned
    @pre Datastorage must point to a valid instance.
    @pre image must Point to a valid instance.
    */
    static mitk::ImageStatisticsContainer::ConstPointer GetImageStatistics(const mitk::DataStorage* dataStorage, const mitk::BaseData* image, const mitk::BaseData* mask=nullptr);

    /** Returns the predicate that can be used to search for statistic containers of
    the given image (and mask) in the passed data storage.*/
    static mitk::NodePredicateBase::ConstPointer GetStatisticsPredicateForSources(const mitk::BaseData* image, const mitk::BaseData* mask = nullptr);
  };
}
#endif
