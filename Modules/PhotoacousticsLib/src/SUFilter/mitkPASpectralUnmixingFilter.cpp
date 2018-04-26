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

// Includes for AddEnmemberMatrix
#include "mitkPAPropertyCalculator.h"
#include <eigen3/Eigen/Dense>

mitk::pa::SpectralUnmixingFilter::SpectralUnmixingFilter()
{
  this->SetNumberOfIndexedOutputs(2);

  for (unsigned int i = 0; i<GetNumberOfIndexedOutputs(); i++)
  {
    this->SetNthOutput(i, mitk::Image::New());
  }

  m_PropertyCalculator = mitk::pa::PropertyCalculator::New();
}

mitk::pa::SpectralUnmixingFilter::~SpectralUnmixingFilter()
{

}

void mitk::pa::SpectralUnmixingFilter::AddWavelength(int wavelength)
{
  m_Wavelength.push_back(wavelength);
}

void mitk::pa::SpectralUnmixingFilter::SetChromophores(ChromophoreType chromophore)
{
  m_Chromophore.push_back(chromophore);
}

void mitk::pa::SpectralUnmixingFilter::GenerateData()
{
  //code recreaction from "old" SUF.cpp

  // now creats to identical output images like the input for hb and hb02

  MITK_INFO << "GENERATING DATA..";
  numberOfInputs = GetNumberOfIndexedInputs();
  numberOfOutputs = GetNumberOfIndexedOutputs();
  
  for (unsigned int outputIdx = 0; outputIdx < numberOfOutputs; outputIdx++)
  {
    GetOutput(outputIdx)->Initialize(GetInput(0));
  }

  length = GetOutput(0)->GetDimension(0)*GetOutput(0)->GetDimension(1)*GetOutput(0)->GetDimension(2);
    
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
  
  AddEndmemberMatrix();
}


//Void creats Matrix with #chromophores rows and #wavelengths columns
//so Matrix Element (i,j) contains the Absorbtion of chromophore j @ wavelength i
void mitk::pa::SpectralUnmixingFilter::AddEndmemberMatrix()
{
  numberofchromophores = m_Chromophore.size();
  numberofwavelengths = m_Wavelength.size();

  Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic> EndmemberMatrix(numberofchromophores, numberofwavelengths);

  //loop over j rows (Chromophores)
  for(unsigned int j =0; j < numberofchromophores; ++j)
  {
    //loop over i columns (Wavelength)
    for (unsigned int i = 0; i < numberofwavelengths; ++i)
    {
      //writes @ Matrix element (i,j) the absorbtion wavelength of the propertycalculator.cpp
      EndmemberMatrix(j,i)= m_PropertyCalculator->GetAbsorptionForWavelength(
      static_cast<mitk::pa::PropertyCalculator::MapType>(m_Chromophore[j]), m_Wavelength[i]);
      if (EndmemberMatrix(i, j) == 0)
      {
        m_Wavelength.clear();
        MITK_INFO << "size: "<< m_Wavelength.size();
        mitkThrow() << "WAVELENGTH NOT SUPPORTED!";
      }
      /* Test to see what gets written in the Matrix:
      MITK_INFO << "map type: " << static_cast<mitk::pa::PropertyCalculator::MapType>(m_Chromophore[j]);
      MITK_INFO << "wavelength: " << m_Wavelength[i];
      MITK_INFO << "Matrixelement: (" << j << ", " << i << ") Absorbtion: " << EndmemberMatrix(j, i);/**/
    }
  }
  MITK_INFO << "GENERATING ENMEMBERMATRIX SUCCESSFUL!";
}
