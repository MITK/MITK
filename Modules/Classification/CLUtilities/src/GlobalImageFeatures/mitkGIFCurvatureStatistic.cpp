/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkGIFCurvatureStatistic.h>

// MITK
#include <mitkITKImageImport.h>
#include <mitkImageCast.h>
#include <mitkImageAccessByItk.h>
#include <mitkPixelTypeMultiplex.h>
#include <mitkImagePixelReadAccessor.h>

// ITK
#include <itkLabelStatisticsImageFilter.h>
#include <itkNeighborhoodIterator.h>

// VTK
#include <vtkSmartPointer.h>
#include <vtkImageMarchingCubes.h>
#include <vtkCurvatures.h>
#include <vtkPointData.h>

// STL
#include <sstream>
#include <limits>

static void calculateLocalStatistic(vtkDataArray* scalars, const std::string& name, const mitk::FeatureID& featureID, mitk::GIFCurvatureStatistic::FeatureListType & featureList)
{
  int size = scalars->GetNumberOfTuples();
  double minimum = std::numeric_limits<double>::max();
  double maximum = std::numeric_limits<double>::lowest();
  double mean1 = 0;
  double mean2 = 0;
  double mean3 = 0;
  double mean1p = 0;
  double mean2p = 0;
  double mean3p = 0;
  double mean1n = 0;
  double mean2n = 0;
  double mean3n = 0;
  int countPositive = 0;
  int countNegative = 0;

  for (int i = 0; i < size; ++i)
  {
    double actualValue = scalars->GetComponent(i, 0);
    minimum = std::min<double>(minimum, scalars->GetComponent(i, 0));
    maximum = std::max<double>(maximum, scalars->GetComponent(i, 0));
    mean1 += actualValue;
    mean2 += actualValue*actualValue;
    mean3 += actualValue*actualValue*actualValue;
    if (actualValue > 0)
    {
      mean1p += actualValue;
      mean2p += actualValue*actualValue;
      mean3p += actualValue*actualValue*actualValue;
      countPositive++;
    }
    if (actualValue < 0)
    {
      mean1n += actualValue;
      mean2n += actualValue*actualValue;
      mean3n += actualValue*actualValue*actualValue;
      countNegative++;
    }
  }
  double mean = mean1 / size;
  double stddev = std::sqrt(mean2 / size - mean*mean);
  double skewness = ((mean3 / size) - 3 * mean*stddev*stddev - mean*mean*mean) / (stddev*stddev*stddev);

  double meanP = mean1p / countPositive;
  double stddevP = std::sqrt(mean2p / countPositive - meanP*meanP);
  double skewnessP = ((mean3p / countPositive) - 3 * meanP*stddevP*stddevP - meanP*meanP*meanP) / (stddevP*stddevP*stddevP);

  double meanN = mean1n / countNegative;
  double stddevN = std::sqrt(mean2n / countNegative - meanN*meanN);
  double skewnessN = ((mean3n / countNegative) - 3 * meanN*stddevN*stddevN - meanN*meanN*meanN) / (stddevN*stddevN*stddevN);

  featureList.push_back(std::make_pair(mitk::CreateFeatureID(featureID, "Minimum " + name + " Curvature"), minimum));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(featureID, "Maximum " + name + " Curvature"), maximum));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(featureID, "Mean " + name + " Curvature"), mean));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(featureID, "Standard Deviation " + name + " Curvature"), stddev));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(featureID, "Skewness " + name + " Curvature"), skewness));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(featureID, "Mean Positive " + name + " Curvature"), meanP));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(featureID, "Standard Deviation Positive " + name + " Curvature"), stddevP));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(featureID, "Skewness Positive " + name + " Curvature"), skewnessP));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(featureID, "Mean Negative " + name + " Curvature"), meanN));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(featureID, "Standard Deviation Negative " + name + " Curvature"), stddevN));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(featureID, "Skewness Negative " + name + " Curvature"), skewnessN));

}

mitk::GIFCurvatureStatistic::GIFCurvatureStatistic()
{
  SetLongName("curvature");
  SetShortName("cur");
  SetFeatureClassName("Curvature Feature");
}

void mitk::GIFCurvatureStatistic::AddArguments(mitkCommandLineParser &parser) const
{
  std::string name = GetOptionPrefix();

  parser.addArgument(GetLongName(), name, mitkCommandLineParser::Bool, "Use Curvature of Surface as feature", "calculates shape curvature based features", us::Any());
}

mitk::AbstractGlobalImageFeature::FeatureListType mitk::GIFCurvatureStatistic::DoCalculateFeatures(const Image* image, const Image* mask)
{
  FeatureListType featureList;

  if (image->GetDimension() < 3)
  {
    MITK_INFO << "Passed calculating volumetric features due to wrong dimensionality ....";
  }
  else
  {
    MITK_INFO << "Start calculating volumetric features ....";

    auto id = this->CreateTemplateFeatureID();
    vtkSmartPointer<vtkImageMarchingCubes> mesher = vtkSmartPointer<vtkImageMarchingCubes>::New();
    vtkSmartPointer<vtkCurvatures> curvator = vtkSmartPointer<vtkCurvatures>::New();
    auto nonconstVtkData = const_cast<vtkImageData*>(mask->GetVtkImageData());
    mesher->SetInputData(nonconstVtkData);
    mesher->SetValue(0, 0.5);
    curvator->SetInputConnection(mesher->GetOutputPort());
    curvator->SetCurvatureTypeToMean();
    curvator->Update();
    vtkDataArray* scalars = curvator->GetOutput()->GetPointData()->GetScalars();
    calculateLocalStatistic(scalars, "Mean", id, featureList);

    curvator->SetCurvatureTypeToGaussian();
    curvator->Update();
    scalars = curvator->GetOutput()->GetPointData()->GetScalars();
    calculateLocalStatistic(scalars, "Gaussian", id, featureList);

    curvator->SetCurvatureTypeToMinimum();
    curvator->Update();
    scalars = curvator->GetOutput()->GetPointData()->GetScalars();
    calculateLocalStatistic(scalars, "Minimum", id, featureList);

    curvator->SetCurvatureTypeToMaximum();
    curvator->Update();
    scalars = curvator->GetOutput()->GetPointData()->GetScalars();
    calculateLocalStatistic(scalars, "Maximum", id, featureList);

    MITK_INFO << "Finished calculating volumetric features....";
  }

  return featureList;
}

mitk::AbstractGlobalImageFeature::FeatureListType mitk::GIFCurvatureStatistic::CalculateFeatures(const Image* image, const Image* mask, const Image*)
{
  return Superclass::CalculateFeatures(image, mask);
}
