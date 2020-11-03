/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkAterialInputFunctionGenerator.h"

#include "mitkMaskedDynamicImageStatisticsGenerator.h"
#include "mitkImageTimeSelector.h"
#include "mitkImageCast.h"
#include "itkArray2D.h"
#include "mitkImageAccessByItk.h"
#include "mitkExtractTimeGrid.h"
#include "mitkAIFBasedModelBase.h"
#include "mitkImageCast.h"

#include <iostream>
#include <fstream>

const double mitk::AterialInputFunctionGenerator::DEFAULT_HEMATOCRIT_LEVEL = 0.45;

mitk::AIFBasedModelBase::AterialInputFunctionType  mitk::AterialInputFunctionGenerator::GetAterialInputFunction()
{
    if(this->HasOutdatedResults())
    {
        CheckValidInputs();
        CalculateAIFAndGetResult();
    }
    return this->m_AIFValues;

}

mitk::ModelBase::TimeGridType mitk::AterialInputFunctionGenerator::GetAterialInputFunctionTimeGrid()
{
    if(this->HasOutdatedResults()){
        CheckValidInputs();
        CalculateAIFAndGetResult();
    }
    return this->m_AIFTimeGrid;

}




void mitk::AterialInputFunctionGenerator::CalculateAIFAndGetResult()
{

   mitk::MaskedDynamicImageStatisticsGenerator::Pointer signalGenerator = mitk::MaskedDynamicImageStatisticsGenerator::New();



    m_AIFValues.SetSize(this->m_DynamicImage->GetTimeSteps());
    m_AIFValues.Fill(0.);
    m_AIFTimeGrid.SetSize(this->m_DynamicImage->GetTimeSteps());
    m_AIFTimeGrid.Fill(0.);


    // = not defined for array???
    mitk::ModelBase::TimeGridType timeGrid = ExtractTimeGrid(m_DynamicImage);
    m_AIFTimeGrid = timeGrid;


    signalGenerator->SetDynamicImage(m_DynamicImage);
    signalGenerator->SetMask(m_Mask);
    signalGenerator->Generate();


    //Convert from aterial curve Ca to Plasma curve Cp
  //  m_AIFValues = signalGenerator->GetMean()/(1-this->m_HCL);
    mitk::MaskedDynamicImageStatisticsGenerator::ResultType temp = signalGenerator->GetMean();


    mitk::AIFBasedModelBase::AterialInputFunctionType::iterator aif = this->m_AIFValues.begin();
    for( mitk::MaskedDynamicImageStatisticsGenerator::ResultType::const_iterator pos = temp.begin(); pos != temp.end(); ++pos, ++aif)
    {
        *aif = *pos /(1-this->m_HCL);

    }


    this->m_GenerationTimeStamp.Modified();

}



void
  mitk::AterialInputFunctionGenerator::CheckValidInputs() const
{

  if (m_DynamicImage.IsNull())
  {
    mitkThrow() << "Cannot generate Aterial Input Function. Input dynamic image is not set.";
  }
  if (m_Mask.IsNull())
  {
    mitkThrow() << "Cannot generate Aterial Input Function. ROI is not set.";
  }
}

bool mitk::AterialInputFunctionGenerator::HasOutdatedResults()
{
    bool result = false;

    if (m_DynamicImage.IsNull())
    {
        result = true;
    }
    else
    {
        if (m_DynamicImage->GetMTime() > this->m_GenerationTimeStamp)
    {
    result = true;
    }
    }

    if (m_Mask.IsNull())
    {
        result = true;
    }
    else
    {
        if (m_Mask->GetMTime() > this->m_GenerationTimeStamp)
        {
            result = true;
        }
    }
    return result;
};
