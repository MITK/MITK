/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkModelFitParameterValueExtraction_h
#define mitkModelFitParameterValueExtraction_h

#include <mitkPoint.h>

#include "mitkModelFitInfo.h"
#include "mitkModelTraitsInterface.h"

#include "MitkModelFitExports.h"


namespace mitk
{

    /** Helper function that gets the voxel value (as double) of an image (given an 3D position and a time step).
     Function is used in different contexts of model fit view.
     @param image
     @param position
     @param timestep
     @param noThrow If set to false, function will throw exceptions. If set to true, exceptions will be blocked, an MITK_ERROR will be notified and the return is 0.0.*/
  MITKMODELFIT_EXPORT ModelTraitsInterface::ParameterValueType ReadVoxel(const mitk::Image* image,
      const mitk::Point3D& position, unsigned int timestep = 0, bool noThrow = true);

    /** Helper function that gets the voxel value (as double) of an image (given an image index and a time step).
    Function is used in different contexts of model fit view.
    @param image
    @param index
    @param timestep
    @param noThrow If set to false, function will throw exceptions.If set to true, exceptions will be blocked, an MITK_ERROR will be notified and the return is 0.0. */
  MITKMODELFIT_EXPORT ModelTraitsInterface::ParameterValueType ReadVoxel(const mitk::Image* image,
      const ::itk::Index<3>& index, unsigned int timestep = 0, bool noThrow = true);

    using ParameterValueMapType = std::map<ModelTraitsInterface::ParameterNameType, double>;

    /** Helper function that extracts a map (keys are the parameter names, values are the associated parameter values)
    of parameter values for the specified 3D point given a model fit info instance.
    @pre fit must point to a valid instance.
    */
    MITKMODELFIT_EXPORT ParameterValueMapType ExtractParameterValueMapFromModelFit(const mitk::modelFit::ModelFitInfo* fitInfo,
      const mitk::Point3D& position);
    MITKMODELFIT_EXPORT ParameterValueMapType ExtractParameterValueMapFromModelFit(const mitk::modelFit::ModelFitInfo* fitInfo,
      const ::itk::Index<3>& index);

    /** Helper function that converts a parameter value map (like returned by ExtractParameterValueMapFromModelFit() into the ParametersType.
      the ordering will be deduced with the help of the passed trait interface.
      @pre pTraitInterface must point to a valid instance.
      @pre valueMap must contain all parameter names of pTraitsInterface*/
    MITKMODELFIT_EXPORT ModelTraitsInterface::ParametersType
      ConvertParameterMapToParameterVector(const ParameterValueMapType &valueMap, const ModelTraitsInterface * pTraitInterface);

}

#endif
