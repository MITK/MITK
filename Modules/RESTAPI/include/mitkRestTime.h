/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkRestTime_h
#define mitkRestTime_h

#include <MitkRESTAPIExports.h>

#include <mitkTimeGeometry.h>

#include <nlohmann/json.hpp>

#include <string>
#include <vector>

namespace mitk
{
  class BaseData;
  struct RestRequest;

  /**
   * \brief The time points that the query parameter timepoint_ms of a
   *        request names.
   *
   * Time points are what all data of a scene share; each data maps them to
   * time steps of its own.
   */
  struct MITKRESTAPI_EXPORT RestTimeSelection
  {
    /** \brief Whether all time steps of the data are meant ("all"). */
    bool All = false;

    /** \brief The time point otherwise, in milliseconds. */
    TimePointType TimePoint = 0.0;

    /**
     * \brief The selection of a request: a time point in milliseconds,
     *        "all", or the selected time point without the parameter.
     *
     * Runs in the GUI thread.
     *
     * \throw RestException with status 400 for any other value.
     */
    static RestTimeSelection FromRequest(const RestRequest& request);

    /**
     * \brief The time steps of the data that the selection covers, in order;
     *        empty if the data has no time step at the time point.
     */
    std::vector<TimeStepType> GetTimeSteps(const BaseData* data) const;

    /** \brief The selection as clients write it: a time point or "all". */
    nlohmann::json ToJson() const;
  };

  /**
   * \brief A time step of data as the time points it covers:
   *        {min_timepoint_ms, max_timepoint_ms}, the maximum excluded, as
   *        GET /api/v1/rendering/selected-time names bounds. A bound is null
   *        where the step has none, as for data with a single time step.
   */
  MITKRESTAPI_EXPORT nlohmann::json DescribeRestTimeStep(const BaseData* data, TimeStepType timeStep);

  /** \brief All time steps of data as DescribeRestTimeStep() gives them. */
  MITKRESTAPI_EXPORT nlohmann::json DescribeRestTimeSteps(const BaseData* data);
}

#endif
