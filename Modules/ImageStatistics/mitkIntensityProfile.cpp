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

#include <itkLinearInterpolateImageFunction.h>
#include <itkNearestNeighborInterpolateImageFunction.h>
#include <itkPolyLineParametricPath.h>
#include <itkWindowedSincInterpolateImageFunction.h>
#include <mitkImageAccessByItk.h>
#include <mitkImagePixelReadAccessor.h>
#include <mitkPixelTypeMultiplex.h>
#include <mitkImageStatisticsContainer.h>
#include "mitkIntensityProfile.h"

using namespace mitk;

template <class T>
static void ReadPixel(const PixelType&, Image::Pointer image, const itk::Index<3>& index, ScalarType* returnValue)
{
  switch (image->GetDimension())
  {
  case 2:
    {
      ImagePixelReadAccessor<T, 2> readAccess(image, image->GetSliceData(0));
      *returnValue = readAccess.GetPixelByIndex(reinterpret_cast<const itk::Index<2>&>(index));
      break;
    }

  case 3:
    {
      ImagePixelReadAccessor<T, 3> readAccess(image, image->GetVolumeData(0));
      *returnValue = readAccess.GetPixelByIndex(index);
      break;
    }

  default:
    *returnValue = 0;
    break;
  }
}

static IntensityProfile::Pointer ComputeIntensityProfile(Image::Pointer image, itk::PolyLineParametricPath<3>::Pointer path)
{
  if (image->GetDimension() == 4)
  {
    mitkThrow() << "computation of intensity profiles not supported for 4D images";
  }

  IntensityProfile::Pointer intensityProfile = IntensityProfile::New();
  itk::PolyLineParametricPath<3>::InputType input = path->StartOfInput();
  BaseGeometry* imageGeometry = image->GetGeometry();
  const PixelType pixelType = image->GetPixelType();

  IntensityProfile::MeasurementVectorType measurementVector;
  itk::PolyLineParametricPath<3>::OffsetType offset;
  Point3D worldPoint;
  itk::Index<3> index;

  do
  {
    imageGeometry->IndexToWorld(path->Evaluate(input), worldPoint);
    imageGeometry->WorldToIndex(worldPoint, index);

    mitkPixelTypeMultiplex3(ReadPixel, pixelType, image, index, measurementVector.GetDataPointer());
    intensityProfile->PushBack(measurementVector);

    offset = path->IncrementInput(input);
  } while ((offset[0] | offset[1] | offset[2]) != 0);

  return intensityProfile;
}

template <class TInputImage>
static typename itk::InterpolateImageFunction<TInputImage>::Pointer CreateInterpolateImageFunction(InterpolateImageFunction::Enum interpolator)
{
  switch (interpolator)
  {
  case InterpolateImageFunction::NearestNeighbor:
    return itk::NearestNeighborInterpolateImageFunction<TInputImage>::New().GetPointer();

  case InterpolateImageFunction::Linear:
    return itk::LinearInterpolateImageFunction<TInputImage>::New().GetPointer();

  case InterpolateImageFunction::WindowedSinc_Blackman_3:
    return itk::WindowedSincInterpolateImageFunction<TInputImage, 3, itk::Function::BlackmanWindowFunction<3> >::New().GetPointer();

  case InterpolateImageFunction::WindowedSinc_Blackman_4:
    return itk::WindowedSincInterpolateImageFunction<TInputImage, 4, itk::Function::BlackmanWindowFunction<4> >::New().GetPointer();

  case InterpolateImageFunction::WindowedSinc_Blackman_5:
    return itk::WindowedSincInterpolateImageFunction<TInputImage, 5, itk::Function::BlackmanWindowFunction<5> >::New().GetPointer();

  case InterpolateImageFunction::WindowedSinc_Cosine_3:
    return itk::WindowedSincInterpolateImageFunction<TInputImage, 3, itk::Function::CosineWindowFunction<3> >::New().GetPointer();

  case InterpolateImageFunction::WindowedSinc_Cosine_4:
    return itk::WindowedSincInterpolateImageFunction<TInputImage, 4, itk::Function::CosineWindowFunction<4> >::New().GetPointer();

  case InterpolateImageFunction::WindowedSinc_Cosine_5:
    return itk::WindowedSincInterpolateImageFunction<TInputImage, 5, itk::Function::CosineWindowFunction<5> >::New().GetPointer();

  case InterpolateImageFunction::WindowedSinc_Hamming_3:
    return itk::WindowedSincInterpolateImageFunction<TInputImage, 3, itk::Function::HammingWindowFunction<3> >::New().GetPointer();

  case InterpolateImageFunction::WindowedSinc_Hamming_4:
    return itk::WindowedSincInterpolateImageFunction<TInputImage, 4, itk::Function::HammingWindowFunction<4> >::New().GetPointer();

  case InterpolateImageFunction::WindowedSinc_Hamming_5:
    return itk::WindowedSincInterpolateImageFunction<TInputImage, 5, itk::Function::HammingWindowFunction<5> >::New().GetPointer();

  case InterpolateImageFunction::WindowedSinc_Lanczos_3:
    return itk::WindowedSincInterpolateImageFunction<TInputImage, 3, itk::Function::LanczosWindowFunction<3> >::New().GetPointer();

  case InterpolateImageFunction::WindowedSinc_Lanczos_4:
    return itk::WindowedSincInterpolateImageFunction<TInputImage, 4, itk::Function::LanczosWindowFunction<4> >::New().GetPointer();

  case InterpolateImageFunction::WindowedSinc_Lanczos_5:
    return itk::WindowedSincInterpolateImageFunction<TInputImage, 5, itk::Function::LanczosWindowFunction<5> >::New().GetPointer();

  case InterpolateImageFunction::WindowedSinc_Welch_3:
    return itk::WindowedSincInterpolateImageFunction<TInputImage, 3, itk::Function::WelchWindowFunction<3> >::New().GetPointer();

  case InterpolateImageFunction::WindowedSinc_Welch_4:
    return itk::WindowedSincInterpolateImageFunction<TInputImage, 4, itk::Function::WelchWindowFunction<4> >::New().GetPointer();

  case InterpolateImageFunction::WindowedSinc_Welch_5:
    return itk::WindowedSincInterpolateImageFunction<TInputImage, 5, itk::Function::WelchWindowFunction<5> >::New().GetPointer();

  default:
    return itk::NearestNeighborInterpolateImageFunction<TInputImage>::New().GetPointer();
  }
}

template <class TPixel, unsigned int VImageDimension>
static void ComputeIntensityProfile(itk::Image<TPixel, VImageDimension>* image, itk::PolyLineParametricPath<3>::Pointer path, unsigned int numSamples, InterpolateImageFunction::Enum interpolator, IntensityProfile::Pointer intensityProfile)
{
  typename itk::InterpolateImageFunction<itk::Image<TPixel, VImageDimension> >::Pointer interpolateImageFunction = CreateInterpolateImageFunction<itk::Image<TPixel, VImageDimension> >(interpolator);
  interpolateImageFunction->SetInputImage(image);

  const itk::PolyLineParametricPath<3>::InputType startOfInput = path->StartOfInput();
  const itk::PolyLineParametricPath<3>::InputType delta = 1.0 / (numSamples - 1);

  IntensityProfile::MeasurementVectorType measurementVector;

  for (unsigned int i = 0; i < numSamples; ++i)
  {
    measurementVector[0] = interpolateImageFunction->EvaluateAtContinuousIndex(path->Evaluate(startOfInput + i * delta));
    intensityProfile->PushBack(measurementVector);
  }
}

static IntensityProfile::Pointer ComputeIntensityProfile(Image::Pointer image, itk::PolyLineParametricPath<3>::Pointer path, unsigned int numSamples, InterpolateImageFunction::Enum interpolator)
{
  IntensityProfile::Pointer intensityProfile = IntensityProfile::New();
  AccessFixedDimensionByItk_n(image, ComputeIntensityProfile, 3, (path, numSamples, interpolator, intensityProfile));
  return intensityProfile;
}

class AddPolyLineElementToPath
{
public:
  AddPolyLineElementToPath(const PlaneGeometry* planarFigureGeometry, const BaseGeometry* imageGeometry, itk::PolyLineParametricPath<3>::Pointer path)
    : m_PlanarFigureGeometry(planarFigureGeometry),
      m_ImageGeometry(imageGeometry),
      m_Path(path)
  {
  }

  void operator()(const PlanarFigure::PolyLineElement& polyLineElement)
  {
    m_PlanarFigureGeometry->Map(polyLineElement, m_WorldPoint);
    m_ImageGeometry->WorldToIndex(m_WorldPoint, m_ContinuousIndexPoint);
    m_Vertex.CastFrom(m_ContinuousIndexPoint);
    m_Path->AddVertex(m_Vertex);
  }

private:
  const PlaneGeometry* m_PlanarFigureGeometry;
  const BaseGeometry* m_ImageGeometry;
  itk::PolyLineParametricPath<3>::Pointer m_Path;

  Point3D m_WorldPoint;
  Point3D m_ContinuousIndexPoint;
  itk::PolyLineParametricPath<3>::ContinuousIndexType m_Vertex;
};

static itk::PolyLineParametricPath<3>::Pointer CreatePathFromPlanarFigure(BaseGeometry* imageGeometry, PlanarFigure* planarFigure)
{
  itk::PolyLineParametricPath<3>::Pointer path = itk::PolyLineParametricPath<3>::New();
  const PlanarFigure::PolyLineType polyLine = planarFigure->GetPolyLine(0);

  std::for_each(polyLine.begin(), polyLine.end(),
    AddPolyLineElementToPath(planarFigure->GetPlaneGeometry(), imageGeometry, path));

  return path;
}

static void AddPointToPath(const BaseGeometry* imageGeometry, const Point3D& point, itk::PolyLineParametricPath<3>::Pointer path)
{
  Point3D continuousIndexPoint;
  imageGeometry->WorldToIndex(point, continuousIndexPoint);

  itk::PolyLineParametricPath<3>::ContinuousIndexType vertex;
  vertex.CastFrom(continuousIndexPoint);

  path->AddVertex(vertex);
}

static itk::PolyLineParametricPath<3>::Pointer CreatePathFromPoints(BaseGeometry* imageGeometry, const Point3D& startPoint, const Point3D& endPoint)
{
  itk::PolyLineParametricPath<3>::Pointer path = itk::PolyLineParametricPath<3>::New();

  AddPointToPath(imageGeometry, startPoint, path);
  AddPointToPath(imageGeometry, endPoint, path);

  return path;
}

IntensityProfile::Pointer mitk::ComputeIntensityProfile(Image::Pointer image, PlanarFigure::Pointer planarFigure)
{
  return ::ComputeIntensityProfile(image, CreatePathFromPlanarFigure(image->GetGeometry(), planarFigure));
}

IntensityProfile::Pointer mitk::ComputeIntensityProfile(Image::Pointer image, PlanarLine::Pointer planarLine, unsigned int numSamples, InterpolateImageFunction::Enum interpolator)
{
  return ::ComputeIntensityProfile(image, CreatePathFromPlanarFigure(image->GetGeometry(), planarLine.GetPointer()), numSamples, interpolator);
}

IntensityProfile::Pointer mitk::ComputeIntensityProfile(Image::Pointer image, const Point3D& startPoint, const Point3D& endPoint, unsigned int numSamples, InterpolateImageFunction::Enum interpolator)
{
  return ::ComputeIntensityProfile(image, CreatePathFromPoints(image->GetGeometry(), startPoint, endPoint), numSamples, interpolator);
}

IntensityProfile::InstanceIdentifier mitk::ComputeGlobalMaximum(IntensityProfile::ConstPointer intensityProfile, IntensityProfile::MeasurementType &max)
{
  max = -vcl_numeric_limits<IntensityProfile::MeasurementType>::min();
  IntensityProfile::InstanceIdentifier maxIndex = 0;

  IntensityProfile::ConstIterator end = intensityProfile->End();
  IntensityProfile::MeasurementType measurement;

  for (IntensityProfile::ConstIterator it = intensityProfile->Begin(); it != end; ++it)
  {
    measurement = it.GetMeasurementVector()[0];

    if (measurement > max)
    {
      max = measurement;
      maxIndex = it.GetInstanceIdentifier();
    }
  }

  return maxIndex;
}

IntensityProfile::InstanceIdentifier mitk::ComputeGlobalMinimum(IntensityProfile::ConstPointer intensityProfile, IntensityProfile::MeasurementType &min)
{
  min = vcl_numeric_limits<IntensityProfile::MeasurementType>::max();
  IntensityProfile::InstanceIdentifier minIndex = 0;

  IntensityProfile::ConstIterator end = intensityProfile->End();
  IntensityProfile::MeasurementType measurement;

  for (IntensityProfile::ConstIterator it = intensityProfile->Begin(); it != end; ++it)
  {
    measurement = it.GetMeasurementVector()[0];

    if (measurement < min)
    {
      min = measurement;
      minIndex = it.GetInstanceIdentifier();
    }
  }

  return minIndex;
}

IntensityProfile::InstanceIdentifier mitk::ComputeCenterOfMaximumArea(IntensityProfile::ConstPointer intensityProfile, IntensityProfile::InstanceIdentifier radius)
{
  //const IntensityProfile::MeasurementType min = intensityProfile->GetMeasurementVector(ComputeGlobalMinimum(intensityProfile))[0];
  IntensityProfile::MeasurementType min;
  ComputeGlobalMinimum(intensityProfile, min);
  const IntensityProfile::InstanceIdentifier areaWidth = 1 + 2 * radius;

  IntensityProfile::MeasurementType maxArea = 0;

  for (IntensityProfile::InstanceIdentifier i = 0; i < areaWidth; ++i)
    maxArea += intensityProfile->GetMeasurementVector(i)[0] - min;

  const IntensityProfile::InstanceIdentifier lastIndex = intensityProfile->Size() - areaWidth;
  IntensityProfile::InstanceIdentifier centerOfMaxArea = radius;
  IntensityProfile::MeasurementType area = maxArea;

  for (IntensityProfile::InstanceIdentifier i = 1; i <= lastIndex; ++i)
  {
    area += intensityProfile->GetMeasurementVector(i + areaWidth - 1)[0] - min;
    area -= intensityProfile->GetMeasurementVector(i - 1)[0] - min;

    if (area > maxArea)
    {
      maxArea = area;
      centerOfMaxArea = i + radius; // TODO: If multiple areas in the neighborhood have the same intensity chose the middle one instead of the first one.
    }
  }

  return centerOfMaxArea;
}

std::vector<IntensityProfile::MeasurementType> mitk::CreateVectorFromIntensityProfile(IntensityProfile::ConstPointer intensityProfile)
{
  std::vector<IntensityProfile::MeasurementType> result;
  result.reserve(intensityProfile->Size());

  IntensityProfile::ConstIterator end = intensityProfile->End();

  for (IntensityProfile::ConstIterator it = intensityProfile->Begin(); it != end; ++it)
    result.push_back(it.GetMeasurementVector()[0]);

  return result;
}

IntensityProfile::Pointer mitk::CreateIntensityProfileFromVector(const std::vector<IntensityProfile::MeasurementType>& vector)
{
  const IntensityProfile::InstanceIdentifier size = vector.size();

  IntensityProfile::Pointer result = IntensityProfile::New();
  result->Resize(size);

  for (IntensityProfile::InstanceIdentifier i = 0; i < size; ++i)
    result->SetMeasurement(i, 0, vector[i]);

  return result;
}

void mitk::ComputeIntensityProfileStatistics(IntensityProfile::ConstPointer intensityProfile, ImageStatisticsContainer::ImageStatisticsObject& stats)
{
  typedef std::vector<IntensityProfile::MeasurementType> StatsVecType;

  StatsVecType statsVec = mitk::CreateVectorFromIntensityProfile( intensityProfile );

  IntensityProfile::MeasurementType min;
  IntensityProfile::MeasurementType max;
  mitk::ComputeGlobalMinimum( intensityProfile, min );
  mitk::ComputeGlobalMaximum( intensityProfile, max );
  auto numSamples = static_cast<mitk::ImageStatisticsContainer::VoxelCountType>(statsVec.size());

  double mean = 0.0;
  double rms = 0.0;
  for ( StatsVecType::const_iterator it = statsVec.begin(); it != statsVec.end(); ++it )
  {
    double val = *it;
    mean += val;
    rms += val*val;
  }
  mean /= static_cast<double>(numSamples);
  rms /= static_cast<double>(numSamples);

  double var = 0.0;
  for ( StatsVecType::const_iterator it = statsVec.begin(); it != statsVec.end(); ++it )
  {
    double diff = *it - mean;
    var += diff*diff;
  }
  var /= (static_cast<double>(numSamples) - 1 );

  rms = sqrt( rms );

  stats.AddStatistic(mitk::ImageStatisticsConstants::MINIMUM(), min);
  stats.AddStatistic(mitk::ImageStatisticsConstants::MAXIMUM(), max);
  stats.AddStatistic(mitk::ImageStatisticsConstants::NUMBEROFVOXELS(), numSamples);
  stats.AddStatistic(mitk::ImageStatisticsConstants::MEAN(), mean);
  stats.AddStatistic(mitk::ImageStatisticsConstants::STANDARDDEVIATION(), sqrt(var));
  stats.AddStatistic(mitk::ImageStatisticsConstants::VARIANCE(), var);
  stats.AddStatistic(mitk::ImageStatisticsConstants::RMS(), rms);
}
