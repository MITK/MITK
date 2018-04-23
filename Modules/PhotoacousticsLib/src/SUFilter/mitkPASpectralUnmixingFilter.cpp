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
  
  //MITK_INFO << "m_w" << m_Wavelengths[size];

  /*if (m_Wavelengths.empty())
  {
    size = 0;
  }*/
  //MITK_INFO << "size after if empty:  " << size;


  //MITK_INFO << "ADD WAVELENGTH...";


  MITK_INFO << "begin size:  " << size;
  m_Wavelengths.push_back(wavelength);
  MITK_INFO << "inhalt vector size:  " << m_Wavelengths[size];
  MITK_INFO << "inhalt vector 0:  " << m_Wavelengths[0]; 
  MITK_INFO << "inhalt vector 10:  " << m_Wavelengths[10];


  int foo = size;
  size = foo + 1;
  MITK_INFO << "end size:  " << size;

}


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
