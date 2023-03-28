/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef mitkImageStatisticsContainerManager_h
#define mitkImageStatisticsContainerManager_h

#include "MitkImageStatisticsExports.h"

#include <mitkDataStorage.h>
#include <mitkImageStatisticsContainer.h>
#include <mitkBaseData.h>
#include <mitkNodePredicateBase.h>
#include <mitkGenericIDRelationRule.h>
#include <mitkPropertyRelations.h>

namespace mitk
{

  static const std::string STATS_HISTOGRAM_BIN_PROPERTY_NAME = "MITK.statistic.histogram_bins";
  static const std::string STATS_IGNORE_ZERO_VOXEL_PROPERTY_NAME = "MITK.statistic.ignore_zero_voxel";
  static const std::string STATS_GENERATION_STATUS_PROPERTY_NAME = "MITK.statistic.generation.status";
  static const std::string STATS_GENERATION_STATUS_VALUE_WORK_IN_PROGRESS = "workInProgress";
  static const std::string STATS_GENERATION_STATUS_VALUE_PENDING = "pending";
  static const std::string STATS_GENERATION_STATUS_VALUE_BASE_DATA_FAILED = "failed";

  /**
  \brief Returns the StatisticsContainer that was computed on given input (image/mask/planar figure) and is added as DataNode in a DataStorage
  */
  class MITKIMAGESTATISTICS_EXPORT ImageStatisticsContainerManager
  {
  public:
    /**Documentation
    @brief Returns the StatisticsContainer for the given image and mask from the storage-
    @return a valid StatisticsContainer or nullptr if no StatisticsContainer is found.
    @details if more than one StatisticsContainer is found, only the newest (ModifiedTime) is returned
    @pre Datastorage must point to a valid instance.
    @pre image must Point to a valid instance.
    @param dataStorage
    @param image
    @param mask
    @param onlyIfUpToDate Indicates if results should only be returned if the are up to date, thus not older then image and ROI.
    @param noWIP If noWIP is true, the function only returns valid final result and not just its placeholder (WIP).
    If noWIP equals false it might also return a WIP, thus the valid result is currently processed/ordered but might not be ready yet.
    @param ignoreZeroVoxel indicates the wanted statistics are calculated with or w/o zero voxels.
    @param histogramNBins Number of bins the statistics should have that are searched for.
    */
    static mitk::ImageStatisticsContainer::Pointer GetImageStatistics(const mitk::DataStorage* dataStorage, const mitk::BaseData* image, const mitk::BaseData* mask=nullptr, bool ignoreZeroVoxel = false, unsigned int histogramNBins = 100, bool onlyIfUpToDate = true, bool noWIP = true);
    static mitk::DataNode::Pointer GetImageStatisticsNode(const mitk::DataStorage* dataStorage, const mitk::BaseData* image, const mitk::BaseData* mask = nullptr, bool ignoreZeroVoxel = false, unsigned int histogramNBins = 100, bool onlyIfUpToDate = true, bool noWIP = true);

    /** Returns the predicate that can be used to search for statistic containers of
    the given image (and mask) in the passed data storage.*/
    static mitk::NodePredicateBase::ConstPointer GetStatisticsPredicateForSources(const mitk::BaseData* image, const mitk::BaseData* mask = nullptr);
  };
}
#endif
