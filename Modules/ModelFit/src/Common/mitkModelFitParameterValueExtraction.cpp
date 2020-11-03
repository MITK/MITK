/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkModelFitParameterValueExtraction.h"
#include "mitkModelFitException.h"

#include <mitkImagePixelReadAccessor.h>
#include <mitkPixelTypeMultiplex.h>


/**
*	@brief	Reads the intensity with the given pixel type from the given image at the given position and time
*			step (at the currently selected voxel) and stores it in the variable 'value'.
*	@tparam TPixel The type of the voxel (e.g. float).
*	@param image	The image whose voxel should be read.
*	@param position	The position in the image that should be read out.
*	@param timestep	The time step at which the voxel should be read.
*	@param value	The variable where the result is stored.
*/
template<typename TPixel>
void ReadVoxelInternal(const mitk::PixelType, const mitk::Image *image,
  const ::itk::Index<3> index,
  unsigned int timestep, mitk::ModelTraitsInterface::ParameterValueType &value)
{
  // Read voxel
  if (image->GetDimension() == 2)
  {
    mitk::ImagePixelReadAccessor<TPixel, 2> readAccess(image, image->GetSliceData(0));
    itk::Index<2> idx;
    idx[0] = index[0];
    idx[1] = index[1];
    value = static_cast<double>(readAccess.GetPixelByIndex(idx));
  }
  else if (image->GetDimension() == 3)
  {
    mitk::ImagePixelReadAccessor<TPixel, 3> readAccess(image, image->GetVolumeData(0));
    value = static_cast<double>(readAccess.GetPixelByIndex(index));
  }
  else if (image->GetDimension() == 4)
  {
    mitk::ImagePixelReadAccessor<TPixel, 3> readAccess(image, image->GetVolumeData(timestep));
    value = static_cast<double>(readAccess.GetPixelByIndex(index));
  }

  MITK_DEBUG << "Voxel [" << index[0] << ", " << index[1] << ", " << index[2] << ", " << timestep
    << "]: " << value;
}


mitk::ModelTraitsInterface::ParameterValueType
mitk::ReadVoxel(const mitk::Image* image,
  const mitk::Point3D& position, unsigned int timestep, bool noThrow)
{
  // Calculate index
  ::itk::Index<3> index;
  mitk::BaseGeometry::Pointer geometry = image->GetTimeGeometry()->GetGeometryForTimeStep(timestep);

  // check for invalid time step
  if (geometry.IsNull())
  {
    geometry = image->GetTimeGeometry()->GetGeometryForTimeStep(0);
  }

  geometry->WorldToIndex(position, index);

  return ReadVoxel(image, index, timestep, noThrow);
}

mitk::ModelTraitsInterface::ParameterValueType
mitk::ReadVoxel(const mitk::Image* image,
const ::itk::Index<3> &index, unsigned int timestep, bool noThrow)
{
  double result = 0.0;

  // check for invalid time step
  if (timestep >= image->GetTimeSteps())
  {
    timestep = 0;
  }

  try
  {
    mitkPixelTypeMultiplex4(ReadVoxelInternal, image->GetPixelType(), image, index, timestep, result);
  }
  catch (const std::exception& e)
  {
    if (noThrow)
    {
      MITK_ERROR << "Exception in ReadVoxel (index: " << index << "; timestep: " << timestep << "). Reason: " << e.what();
    }
    else throw;
  }
  catch (...)
  {
    if (noThrow)
    {
      MITK_ERROR << "Unknown exception in ReadVoxel (index: " << index << "; timestep: " << timestep << ").";
    }
    else throw;
  }

  return result;
}

mitk::ParameterValueMapType
mitk::ExtractParameterValueMapFromModelFit(const mitk::modelFit::ModelFitInfo* fitInfo,
  const mitk::Point3D& position)
{
  if (!fitInfo)
  {
    mitkThrow() << "Cannot extract parameter value map out of model fit instance. Passed model fit instance is a null ptr.";
  }

  ParameterValueMapType parameterMap;

  for (mitk::modelFit::ModelFitInfo::ConstIterType paramIter = fitInfo->GetParameters().begin();
    paramIter != fitInfo->GetParameters().end();
    ++paramIter)
  {
    const mitk::modelFit::Parameter* param = *paramIter;

    if (param->type == mitk::modelFit::Parameter::ParameterType)
    {
      parameterMap[param->name] = mitk::ReadVoxel(param->image, position);
    }
  }

  return parameterMap;
};

mitk::ParameterValueMapType
mitk::ExtractParameterValueMapFromModelFit(const mitk::modelFit::ModelFitInfo* fitInfo,
const ::itk::Index<3> &index)
{
  if (!fitInfo)
  {
    mitkThrow() << "Cannot extract parameter value map out of model fit instance. Passed model fit instance is a null ptr.";
  }

  ParameterValueMapType parameterMap;

  for (mitk::modelFit::ModelFitInfo::ConstIterType paramIter = fitInfo->GetParameters().begin();
    paramIter != fitInfo->GetParameters().end();
    ++paramIter)
  {
    const mitk::modelFit::Parameter* param = *paramIter;

    if (param->type == mitk::modelFit::Parameter::ParameterType)
    {
      parameterMap[param->name] = mitk::ReadVoxel(param->image, index);
    }
  }

  return parameterMap;
};

mitk::ModelTraitsInterface::ParametersType
mitk::ConvertParameterMapToParameterVector(const ParameterValueMapType &valueMap, const ModelTraitsInterface * pTraitInterface)
{
  if (!pTraitInterface)
  {
    mitkThrow() << "Cannot convert parameter values. Passed traits interface is a null ptr.";
  }

  const mitk::ModelTraitsInterface::ParameterNamesType paramNameVector =
    pTraitInterface->GetParameterNames();

  mitk::ModelTraitsInterface::ParametersType paramArray(paramNameVector.size());
  mitk::ModelTraitsInterface::ParametersSizeType paramIndex = 0;

  for (const auto& name : paramNameVector)
  {
    ParameterValueMapType::const_iterator iter = valueMap.find(name);

    if (iter != valueMap.end())
    {
      paramArray[paramIndex] = iter->second;
      ++paramIndex;
    }
    else
    {
      mitkThrow() << "Cannot convert parameter values. Parameter name ('" << name << "') is missing in value map.";
    }
  }

  return paramArray;
}
