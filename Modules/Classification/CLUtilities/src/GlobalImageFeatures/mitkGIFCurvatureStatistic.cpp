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

static void calculateLocalStatistic(vtkDataArray* scalars, std::string name, std::string featureDescriptionPrefix, mitk::GIFCurvatureStatistic::FeatureListType & featureList)
{
  int size = scalars->GetNumberOfTuples();
  double minimum = std::numeric_limits<double>::max();
  double maximum = std::numeric_limits<double>::lowest();
  double mean1 = 0;
  double mean2 = 0;
  double mean3 = 0;
  double mean4 = 0;
  double mean1p = 0;
  double mean2p = 0;
  double mean3p = 0;
  double mean4p = 0;
  double mean1n = 0;
  double mean2n = 0;
  double mean3n = 0;
  double mean4n = 0;
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
    mean4 += actualValue*actualValue*actualValue*actualValue;
    if (actualValue > 0)
    {
      mean1p += actualValue;
      mean2p += actualValue*actualValue;
      mean3p += actualValue*actualValue*actualValue;
      mean4p += actualValue*actualValue*actualValue*actualValue;
      countPositive++;
    }
    if (actualValue < 0)
    {
      mean1n += actualValue;
      mean2n += actualValue*actualValue;
      mean3n += actualValue*actualValue*actualValue;
      mean4n += actualValue*actualValue*actualValue*actualValue;
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

  featureList.push_back(std::make_pair(featureDescriptionPrefix + "Minimum " + name + " Curvature", minimum));
  featureList.push_back(std::make_pair(featureDescriptionPrefix + "Maximum " + name + " Curvature", maximum));
  featureList.push_back(std::make_pair(featureDescriptionPrefix + "Mean " + name + " Curvature", mean));
  featureList.push_back(std::make_pair(featureDescriptionPrefix + "Standard Deviation " + name + " Curvature", stddev));
  featureList.push_back(std::make_pair(featureDescriptionPrefix + "Skewness " + name + " Curvature", skewness));
  featureList.push_back(std::make_pair(featureDescriptionPrefix + "Mean Positive " + name + " Curvature", meanP));
  featureList.push_back(std::make_pair(featureDescriptionPrefix + "Standard Deviation Positive " + name + " Curvature", stddevP));
  featureList.push_back(std::make_pair(featureDescriptionPrefix + "Skewness Positive " + name + " Curvature", skewnessP));
  featureList.push_back(std::make_pair(featureDescriptionPrefix + "Mean Negative " + name + " Curvature", meanN));
  featureList.push_back(std::make_pair(featureDescriptionPrefix + "Standard Deviation Negative " + name + " Curvature", stddevN));
  featureList.push_back(std::make_pair(featureDescriptionPrefix + "Skewness Negative " + name + " Curvature", skewnessN));

}

mitk::GIFCurvatureStatistic::GIFCurvatureStatistic()
{
  SetLongName("curvature");
  SetShortName("cur");
  SetFeatureClassName("Curvature Feature");
}

mitk::GIFCurvatureStatistic::FeatureListType mitk::GIFCurvatureStatistic::CalculateFeatures(const Image::Pointer & image, const Image::Pointer &mask)
{
  FeatureListType featureList;
  if (image->GetDimension() < 3)
  {
    return featureList;
  }

  vtkSmartPointer<vtkImageMarchingCubes> mesher = vtkSmartPointer<vtkImageMarchingCubes>::New();
  vtkSmartPointer<vtkCurvatures> curvator = vtkSmartPointer<vtkCurvatures>::New();
  mesher->SetInputData(mask->GetVtkImageData());
  mesher->SetValue(0, 0.5);
  curvator->SetInputConnection(mesher->GetOutputPort());
  curvator->SetCurvatureTypeToMean();
  curvator->Update();
  vtkDataArray* scalars = curvator->GetOutput()->GetPointData()->GetScalars();
  calculateLocalStatistic(scalars, "Mean", FeatureDescriptionPrefix(), featureList);

  curvator->SetCurvatureTypeToGaussian();
  curvator->Update();
  scalars = curvator->GetOutput()->GetPointData()->GetScalars();
  calculateLocalStatistic(scalars, "Gaussian", FeatureDescriptionPrefix(), featureList);

  curvator->SetCurvatureTypeToMinimum();
  curvator->Update();
  scalars = curvator->GetOutput()->GetPointData()->GetScalars();
  calculateLocalStatistic(scalars, "Minimum", FeatureDescriptionPrefix(), featureList);

  curvator->SetCurvatureTypeToMaximum();
  curvator->Update();
  scalars = curvator->GetOutput()->GetPointData()->GetScalars();
  calculateLocalStatistic(scalars, "Maximum", FeatureDescriptionPrefix(), featureList);

  return featureList;
}

mitk::GIFCurvatureStatistic::FeatureNameListType mitk::GIFCurvatureStatistic::GetFeatureNames()
{
  FeatureNameListType featureList;
  return featureList;
}


void mitk::GIFCurvatureStatistic::AddArguments(mitkCommandLineParser &parser)
{
  std::string name = GetOptionPrefix();

  parser.addArgument(GetLongName(), name, mitkCommandLineParser::Bool, "Use Curvature of Surface as feature", "calculates shape curvature based features", us::Any());
}

void
mitk::GIFCurvatureStatistic::CalculateFeaturesUsingParameters(const Image::Pointer & feature, const Image::Pointer &mask, const Image::Pointer &, FeatureListType &featureList)
{
  auto parsedArgs = GetParameter();
  if (parsedArgs.count(GetLongName()))
  {
    MITK_INFO << "Start calculating volumetric features ....";
    auto localResults = this->CalculateFeatures(feature, mask);
    featureList.insert(featureList.end(), localResults.begin(), localResults.end());
    MITK_INFO << "Finished calculating volumetric features....";
  }
}

