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

#include "mitkPASpectralUnmixingFilter.h"

#include <mitkImageReadAccessor.h>
#include <mitkImageWriteAccessor.h>

mitk::pa::SpectralUnmixingFilter::SpectralUnmixingFilter()
{
  this->SetNumberOfIndexedOutputs(3);

  for (unsigned int i = 0; i<GetNumberOfIndexedOutputs(); i++)
  {
    this->SetNthOutput(i, mitk::Image::New());
  }

}

mitk::pa::SpectralUnmixingFilter::~SpectralUnmixingFilter()
{

}

void mitk::pa::SpectralUnmixingFilter::Test()
{
  MITK_INFO << "Test";
}


void mitk::pa::SpectralUnmixingFilter::AddWavelength(int wavelength)
{
  m_Wavelength.push_back(wavelength);
}


void mitk::pa::SpectralUnmixingFilter::GenerateData()
{

  //Test region:++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  //code from "old" SUF.cpp

  // now creats to identical output images lieke the input for hb and hb02

  MITK_INFO << "GENERATING DATA..";
  unsigned int numberOfInputs = GetNumberOfIndexedInputs();
  unsigned int numberOfOutputs = GetNumberOfIndexedOutputs();


  for (unsigned int outputIdx = 0; outputIdx < numberOfOutputs; outputIdx++)
  {
    GetOutput(outputIdx)->Initialize(GetInput(0));
  }

  long length = GetOutput(0)->GetDimension(0)*GetOutput(0)->GetDimension(1)*GetOutput(0)->GetDimension(2);

  for (int i = 0; i < length; i++)
  {
    for (unsigned int j = 0; j < numberOfInputs; j++)
    {
      mitk::Image::Pointer input = GetInput(j);
      mitk::ImageReadAccessor readAccess(input, input->GetVolumeData());
    }



    for (unsigned int outputIdx = 0; outputIdx < numberOfOutputs; outputIdx++)
    {
      mitk::Image::Pointer output = GetOutput(outputIdx);
      mitk::ImageWriteAccessor writeOutput(output, output->GetVolumeData());
      double* outputArray = (double *)writeOutput.GetData();
    }
  }

  MITK_INFO << "GENERATING DATA...[DONE]";

  //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++





}


//// Not correct working alternativ for wavelengths
/*void mitk::pa::SpectralUnmixingFilter::AddWavelength(int wavelength)
{
  // Das ganze klappt noch nicht so! m_wavelengths wird nicht gespeichert ... 
  if (m_Wavelengths.empty())
  {
    size = 0;
  }
  m_Wavelengths.push_back(wavelength);
  size += 1;
}*/


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// VERWORFENES ZEUG: 
/* 

void mitk::pa::SpectralUnmixingFilter::GetInputPictures()
{
  MITK_INFO << "GET INPUT PICTURES...";
  unsigned int numberOfInputs = GetNumberOfIndexedInputs();
  unsigned int numberOfOutputs = GetNumberOfIndexedOutputs();

  MITK_INFO << "Inputs: " << numberOfInputs << " Outputs: " << numberOfOutputs;

  if (m_Wavelengths.empty() || m_Wavelengths.size() != numberOfInputs || numberOfInputs < m_Chromophores.size())
  {
    std::string invalidWavelengthError = "Not enough wavelengths given for calculation.";
    MITK_ERROR << invalidWavelengthError;
    mitkThrow() << invalidWavelengthError;
  }
 MITK_INFO << "LOADING...[DONE]";
}

void mitk::pa::SpectralUnmixingFilter::AddChromophore(int chromophore)
{
  MITK_INFO << "ADD CHROMOPHORE...";
  m_Chromophores.push_back(chromophore);
}
Probably easier to implement in the Plugin like: if (slot x then do algorithm x)
void mitk::pa::SpectralUnmixingFilter::ChooseAlgorithm(int algorithm)
{
  MITK_INFO << "CHOOSE ALGORITHM...";
  m_Chromophores.push_back(algorithm);
}*/
