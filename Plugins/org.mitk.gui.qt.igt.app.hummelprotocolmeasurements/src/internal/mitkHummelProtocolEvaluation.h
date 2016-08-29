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


#ifndef MITKHummelProtocolEvaluation_H_HEADER_INCLUDED_
#define MITKHummelProtocolEvaluation_H_HEADER_INCLUDED_

#include <mitkPointSet.h>


namespace mitk
{

  /**Documentation
  * \brief TODO
  *
  * \ingroup IGT
  */
  class HummelProtocolEvaluation

  {
  public:
    struct HummelProtocolDistanceError {double distanceError; std::string description;};
  enum HummelProtocolMeasurementVolume { small, standard };
    static bool EvaluateDistances(mitk::PointSet::Pointer p, HummelProtocolMeasurementVolume m, std::vector<HummelProtocolDistanceError> &Results);
  };
} // namespace mitk

#endif /* MITKHummelProtocolEvaluation_H_HEADER_INCLUDED_ */
