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

#include "mitkModelFitPlotDataHelper.h"

#include "mitkExceptionMacro.h"
#include "mitkImage.h"
#include "mitkModelFitParameterValueExtraction.h"
#include "mitkModelGenerator.h"

#include "mitkFormulaParser.h"

mitk::PlotDataCurve::PlotDataCurve(const PlotDataValues& values) : values(values)
{
  time.Modified();
};

mitk::PlotDataCurve::PlotDataCurve(const PlotDataCurve& other) : values(other.values), time(other.time)
{
};

mitk::PlotDataCurve::PlotDataCurve(PlotDataCurve&& other) : values(std::move(other.values))
{
  this->time = other.time;
  other.time = itk::TimeStamp();
};

mitk::PlotDataCurve & mitk::PlotDataCurve::operator=(const PlotDataCurve& rhs)
{
  this->values = rhs.values;
  this->time = rhs.time;
  return *this;
};

mitk::PlotDataCurve& mitk::PlotDataCurve::operator=(PlotDataCurve&& rhs) noexcept
{
  this->values = std::move(rhs.values);
  this->time = rhs.time;
  rhs.time = itk::TimeStamp();

  return *this;
};

void mitk::PlotDataCurve::Reset()
{
  this->values.clear();
  this->time = itk::TimeStamp();
}

/** Helper function that generates the curve based on a stored and on the fly parsed function string.*/
mitk::PlotDataCurve
CalcSignalFromFunction(const mitk::Point3D& position, const mitk::modelFit::ModelFitInfo* fitInfo, const mitk::ModelBase::TimeGridType& timeGrid)
{
  if (!fitInfo)
  {
    mitkThrow() << "Cannot calc model curve from function for given fit. Passed ModelFitInfo instance is nullptr.";
  }

  mitk::Image::Pointer inputImage = fitInfo->inputImage;
  assert(inputImage.IsNotNull());

  mitk::PlotDataCurve result;

  // Calculate index
  ::itk::Index<3> index;
  mitk::BaseGeometry::Pointer geometry = inputImage->GetTimeGeometry()->GetGeometryForTimeStep(0);

  geometry->WorldToIndex(position, index);

  mitk::ParameterValueMapType parameterMap = mitk::ExtractParameterValueMapFromModelFit(fitInfo, position);

  mitk::FormulaParser parser(&parameterMap);

  for (unsigned int t = 0; t < timeGrid.size(); ++t)
  {
    // Set up static parameters
    for (const auto& var : fitInfo->staticParamMap)
    {
      const auto& list = var.second;

      if (list.size() == 1)
      {
        parameterMap[var.first] = list.front();
      }
      else
      {
        parameterMap[var.first] = list.at(t);
      }
    }

    // Calculate curve data
    double x = timeGrid[t];
    parameterMap[fitInfo->x] = x;

    double y = parser.parse(fitInfo->function);
    result.values.push_back(std::make_pair(x, y));
  }

  result.time.Modified();
  return result;
}

/** Helper function that generates the curve based on the model specified by the fit info.*/
mitk::PlotDataCurve
CalcSignalFromModel(const mitk::Point3D& position, const mitk::modelFit::ModelFitInfo* fitInfo, const mitk::ModelParameterizerBase* parameterizer = nullptr)
{
  assert(fitInfo);

  if (!parameterizer)
  {
    parameterizer = mitk::ModelGenerator::GenerateModelParameterizer(*fitInfo);
  }

  mitk::Image::Pointer inputImage = fitInfo->inputImage;
  assert(inputImage.IsNotNull());

  // Calculate index
  ::itk::Index<3> index;
  mitk::BaseGeometry::Pointer geometry = inputImage->GetTimeGeometry()->GetGeometryForTimeStep(0);

  geometry->WorldToIndex(position, index);

  //model generation
  mitk::ModelBase::Pointer model = parameterizer->GenerateParameterizedModel(index);

  mitk::ParameterValueMapType parameterMap = mitk::ExtractParameterValueMapFromModelFit(fitInfo, position);

  mitk::ModelBase::ParametersType paramArray = mitk::ConvertParameterMapToParameterVector(parameterMap, model);

  mitk::ModelBase::ModelResultType curveDataY = model->GetSignal(paramArray);
  mitk::PlotDataCurve result;
  mitk::ModelBase::TimeGridType timeGrid = model->GetTimeGrid();

  for (unsigned int t = 0; t < timeGrid.size(); ++t)
  {
    double x = timeGrid[t];
    double y = curveDataY[t];
    result.values.push_back(std::make_pair(x, y));
  }

  result.time.Modified();
  return result;
}

mitk::PlotDataCurve
mitk::GenerateModelSignalPlotData(const mitk::Point3D& position, const mitk::modelFit::ModelFitInfo* fitInfo, const mitk::ModelBase::TimeGridType& timeGrid, mitk::ModelParameterizerBase* parameterizer)
{
  if (!fitInfo)
  {
    mitkThrow() << "Cannot calc model curve from function for given fit. Passed ModelFitInfo instance is nullptr.";
  }

  PlotDataCurve result;

  if (!parameterizer)
  {
    parameterizer = ModelGenerator::GenerateModelParameterizer(*fitInfo);
  }

  if (parameterizer)
  {
    // Use model instead of formula parser
    parameterizer->SetDefaultTimeGrid(timeGrid);
    result = CalcSignalFromModel(position, fitInfo, parameterizer);
  }
  else
  {
    // Use formula parser to parse function string
    try
    {
      result = CalcSignalFromFunction(position, fitInfo, timeGrid);
    }
    catch (const mitk::FormulaParserException& e)
    {
      MITK_ERROR << "Error while parsing modelfit function: " << e.what();
    }
  }

  return result;
}

mitk::PlotDataCurveCollection
mitk::GenerateAdditionalModelFitPlotData(const mitk::Point3D& position, const mitk::modelFit::ModelFitInfo* fitInfo, const mitk::ModelBase::TimeGridType& timeGrid)
{
  if (!fitInfo)
  {
    mitkThrow() << "Cannot calc model curve from function for given fit. Passed ModelFitInfo instance is nullptr.";
  }

  mitk::PlotDataCurveCollection result;

  for (const auto& additionalInput : fitInfo->inputData.GetLookupTable())
  {
    if (additionalInput.second.size() != timeGrid.size())
    {
      MITK_ERROR <<
        "Error while refreshing input data for visualization. Size of data and input image time grid differ. Invalid data name: "
        << additionalInput.first;
    }
    else
    {
      mitk::PlotDataCurve pointData;

      for (unsigned int t = 0; t < timeGrid.size(); ++t)
      {
        const double x = timeGrid[t];
        const double y = additionalInput.second[t];
        pointData.values.push_back(std::make_pair(x, y));
      }
      pointData.time.Modified();
      result.emplace(additionalInput.first, std::move(pointData));
    }
  }

  return result;
}

mitk::PlotDataCurve
mitk::GenerateImageSamplePlotData(const mitk::Point3D& position, const mitk::Image* image, const mitk::ModelBase::TimeGridType& timeGrid)
{
  if (!image)
  {
    mitkThrow() << "Cannot generate sample plot data. Passed image instance is nullptr.";
  }

  mitk::PlotDataCurve result;

  for (unsigned int t = 0; t < timeGrid.size(); ++t)
  {
    const double x = timeGrid[t];
    const double y = ReadVoxel(image, position, t);
    result.values.push_back(std::make_pair(x, y));
  }

  result.time.Modified();
  return result;
}
