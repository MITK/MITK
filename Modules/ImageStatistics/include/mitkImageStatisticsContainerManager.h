/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef mitkImageStatisticsContainerManager_h
#define mitkImageStatisticsContainerManager_h

#include <MitkImageStatisticsExports.h>

#include <mitkDataStorage.h>
#include <mitkImageStatisticsContainer.h>
#include <mitkBaseData.h>
#include <mitkNodePredicateBase.h>
#include <mitkGenericIDRelationRule.h>
#include <mitkPropertyRelations.h>

namespace mitk
{

  /** \brief Property name for the histogram bin count stored on statistics nodes. */
  static const std::string STATS_HISTOGRAM_BIN_PROPERTY_NAME = "MITK.statistic.histogram_bins";
  /** \brief Property name indicating whether zero voxels were ignored. */
  static const std::string STATS_IGNORE_ZERO_VOXEL_PROPERTY_NAME = "MITK.statistic.ignore_zero_voxel";
  /** \brief Property name for the generation status of the statistics. */
  static const std::string STATS_GENERATION_STATUS_PROPERTY_NAME = "MITK.statistic.generation.status";
  /** \brief Status value indicating statistics computation is in progress. */
  static const std::string STATS_GENERATION_STATUS_VALUE_WORK_IN_PROGRESS = "workInProgress";
  /** \brief Status value indicating statistics computation is pending. */
  static const std::string STATS_GENERATION_STATUS_VALUE_PENDING = "pending";
  /** \brief Status value indicating statistics computation failed. */
  static const std::string STATS_GENERATION_STATUS_VALUE_BASE_DATA_FAILED = "failed";

  /**
   * \brief Manages lookup of ImageStatisticsContainer nodes in a DataStorage.
   *
   * This utility class provides static methods to find or retrieve
   * ImageStatisticsContainer instances that were previously computed for a
   * given image and optional mask, and stored as DataNode objects in a
   * DataStorage.
   *
   * \sa ImageStatisticsContainer
   * \sa ImageStatisticsCalculator
   * \sa StatisticsToImageRelationRule
   * \sa StatisticsToMaskRelationRule
   */
  class MITKIMAGESTATISTICS_EXPORT ImageStatisticsContainerManager
  {
  public:
    /**
     * \brief Find and return the ImageStatisticsContainer for the given image and mask.
     *
     * Searches the data storage for an ImageStatisticsContainer that was computed
     * on the specified image (and optionally mask). If multiple containers match,
     * the one with the newest modification time is returned.
     *
     * \param[in] dataStorage Pointer to the DataStorage to search.
     * \param[in] image Pointer to the source image.
     * \param[in] mask Pointer to the mask (can be nullptr if unmasked statistics are desired).
     * \param[in] ignoreZeroVoxel Whether the desired statistics exclude zero voxels.
     * \param[in] histogramNBins Number of histogram bins the desired statistics should have.
     * \param[in] onlyIfUpToDate If true, only returns statistics that are newer than image and mask.
     * \param[in] noWIP If true, only returns completed results (not work-in-progress placeholders).
     *
     * \return A valid ImageStatisticsContainer, or nullptr if none is found.
     *
     * \pre dataStorage must point to a valid instance.
     * \pre image must point to a valid instance.
     */
    static mitk::ImageStatisticsContainer::Pointer GetImageStatistics(const mitk::DataStorage* dataStorage, const mitk::BaseData* image, const mitk::BaseData* mask=nullptr, bool ignoreZeroVoxel = false, unsigned int histogramNBins = 100, bool onlyIfUpToDate = true, bool noWIP = true);

    /**
     * \brief Find and return the DataNode containing the ImageStatisticsContainer.
     *
     * Same search logic as GetImageStatistics(), but returns the DataNode wrapper.
     *
     * \param[in] dataStorage Pointer to the DataStorage to search.
     * \param[in] image Pointer to the source image.
     * \param[in] mask Pointer to the mask (can be nullptr).
     * \param[in] ignoreZeroVoxel Whether zero voxels were ignored.
     * \param[in] histogramNBins Number of histogram bins.
     * \param[in] onlyIfUpToDate Only return up-to-date results.
     * \param[in] noWIP Only return completed results.
     *
     * \return The matching DataNode, or nullptr if none is found.
     */
    static mitk::DataNode::Pointer GetImageStatisticsNode(const mitk::DataStorage* dataStorage, const mitk::BaseData* image, const mitk::BaseData* mask = nullptr, bool ignoreZeroVoxel = false, unsigned int histogramNBins = 100, bool onlyIfUpToDate = true, bool noWIP = true);

    /**
     * \brief Create a predicate to search for statistics containers of the given sources.
     *
     * \param[in] image Pointer to the source image.
     * \param[in] mask Pointer to the mask (can be nullptr).
     *
     * \return A node predicate matching statistics containers for the given sources.
     */
    static mitk::NodePredicateBase::ConstPointer GetStatisticsPredicateForSources(const mitk::BaseData* image, const mitk::BaseData* mask = nullptr);
  };
}
#endif
