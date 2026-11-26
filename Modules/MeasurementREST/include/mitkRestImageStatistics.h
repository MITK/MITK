/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkRestImageStatistics_h
#define mitkRestImageStatistics_h

#include <MitkMeasurementRESTExports.h>

#include <mitkDataNode.h>
#include <mitkImageStatisticsContainer.h>
#include <mitkIRestServerService.h>
#include <mitkRestTime.h>

#include <nlohmann/json.hpp>

#include <string>
#include <vector>

namespace mitk
{
  class DataStorage;

  /**
   * \brief The image a mask belongs to: its source in the data storage.
   *
   * Runs in the GUI thread.
   *
   * \return The handle of the first source that holds an image, null if there is none.
   */
  MITKMEASUREMENTREST_EXPORT DataNodeHandle FindRestReferenceImage(
    const IRestServerService& server, const DataStorage* dataStorage, const DataNode* mask);

  /**
   * \brief The time steps of the data that the selection covers.
   *
   * \param path The request path, for errors.
   * \throw RestException with status 422 and code NO_DATA_AT_TIME_POINT if the
   *        data has no time step at the selected time point.
   */
  MITKMEASUREMENTREST_EXPORT std::vector<TimeStepType> GetRestTimeSteps(
    const RestTimeSelection& selection, const DataNode* node, const std::string& path);

  /**
   * \brief Checks that the statistics measure a segmentation where the
   *        selection names it.
   *
   * The statistics take the segmentation at the time point where each
   * measured time step of the image begins. For all time steps, the
   * segmentation needs data there; for a time point, the same data as at the
   * time point.
   *
   * \param imageTimeSteps The measured time steps of the image, as
   *        GetRestTimeSteps() gives them.
   * \param path The request path, for errors.
   * \throw RestException with status 422 and code NO_DATA_AT_TIME_POINT
   *        otherwise.
   */
  MITKMEASUREMENTREST_EXPORT void CheckRestMaskTimeSteps(const RestTimeSelection& selection, const DataNode* imageNode,
    const DataNode* maskNode, const std::vector<TimeStepType>& imageTimeSteps, const std::string& path);

  /**
   * \brief Adds the measured values of a region to a JSON object: voxels, the
   *        volume in milliliters if the values have one, and, for images with
   *        scalar pixels, mean, median, standard_deviation, min and max.
   */
  MITKMEASUREMENTREST_EXPORT void AddRestStatisticsValues(
    nlohmann::json& json, const ImageStatisticsContainer::ImageStatisticsObject& values);
}

#endif
