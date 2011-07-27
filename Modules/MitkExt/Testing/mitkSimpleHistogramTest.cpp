/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include <mitkSimpleHistogram.h>
#include <mitkTestingMacros.h>
#include <mitkSurface.h>

int mitkSimpleHistogramTest(int /*argc*/, char* /*argv*/[])
{
  MITK_TEST_BEGIN("mitkSimpleHistogram");

  mitk::SimpleImageHistogram* myTestSimpleImageHistogram = new mitk::SimpleImageHistogram();

  MITK_TEST_CONDITION_REQUIRED(myTestSimpleImageHistogram!=NULL,"Testing instanciation.");
  MITK_TEST_CONDITION_REQUIRED(myTestSimpleImageHistogram->GetMax()==1,"Testing GetMax().");
  MITK_TEST_CONDITION_REQUIRED(myTestSimpleImageHistogram->GetMin()==0,"Testing GetMin().");
  MITK_TEST_CONDITION_REQUIRED(myTestSimpleImageHistogram->GetRelativeBin(1.0,5.0) ==0,"Testing GetRelativeBin().");
  bool success = true;
  try
    {
    myTestSimpleImageHistogram->ComputeFromBaseData(NULL); 
    myTestSimpleImageHistogram->ComputeFromBaseData(mitk::Image::New()); //an empty image
    myTestSimpleImageHistogram->ComputeFromBaseData(mitk::Surface::New()); //an invalid value
    }
  catch(...)
    {
    success = false;
    }
  MITK_TEST_CONDITION_REQUIRED(success,"Testing ComputeFromBaseData() with invalid input values.");
  MITK_TEST_CONDITION_REQUIRED(!myTestSimpleImageHistogram->GetValid(),"Testing if histogram is invalid after invalid input.");

  MITK_TEST_END();
}