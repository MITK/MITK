/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-10-07 16:14:59 +0200 (Mi, 07 Okt 2009) $
Version:   $Revision: 19343 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include "mitkTestingMacros.h"
#include "mitkPixelType.h"
#include <itkImage.h>
#include <mitkLogMacros.h>


//## Documentation
//## main testing method
int mitkPixelTypeTest(int /*argc*/, char* /*argv*/[])
{
  MITK_TEST_BEGIN("PixelTypeTest");

  MITK_INFO << "ptype = mitk::MakePixelType<int, int, 5>();";
  MITK_INFO << "itkPtype = mitk::MakePixelType<ItkImageType>();\n with  itk::Image<itk::FixedArray< int, 5>, 3> ItkImageType";

  mitk::PixelType ptype = mitk::MakePixelType<int, int, 5>();
  typedef itk::Image<itk::FixedArray< int, 5>, 3> ItkImageType;
  mitk::PixelType itkPtype = mitk::MakePixelType<ItkImageType>();

  MITK_TEST_CONDITION_REQUIRED( ptype.GetTypeId() == typeid(int), "GetTypeId()");
 // MITK_TEST_CONDITION( ptype.GetPixelTypeId() == typeid(ItkImageType), "GetPixelTypeId()");

  MITK_INFO << ptype.GetPixelTypeId().name();
  MITK_INFO << typeid(ItkImageType).name();

  MITK_TEST_CONDITION_REQUIRED( ptype.GetBpe() == 8*sizeof(int)*5, "[ptype] GetBpe()");
  MITK_TEST_CONDITION_REQUIRED( ptype.GetNumberOfComponents() == 5, "[ptype]GetNumberOfComponents()");
  MITK_TEST_CONDITION_REQUIRED( ptype.GetBitsPerComponent() == 8*sizeof(int), "[ptype]GetBitsPerComponent()");

  MITK_TEST_CONDITION_REQUIRED( itkPtype.GetBpe() == 8*sizeof(int)*5, "[itkPType] GetBpe()");
  MITK_TEST_CONDITION_REQUIRED( itkPtype.GetNumberOfComponents() == 5, "[itkPType] GetNumberOfComponents()");
  MITK_TEST_CONDITION_REQUIRED( itkPtype.GetBitsPerComponent() == 8*sizeof(int), "[itkPType] GetBitsPerComponent()");

  // MITK_TEST_CONDITION_REQUIRED( itkPtype == ptype, "[itkPtype = ptype]");

  //MITK_TEST_CONDITION( ptype.GetItkTypeAsString().compare("unknown") == 0, "GetItkTypeAsString()");
  {

    {
      mitk::PixelType ptype2( ptype); 
      MITK_TEST_CONDITION_REQUIRED( ptype2.GetTypeId() == typeid(int), "ptype2( ptype)- GetTypeId()");
      MITK_TEST_CONDITION( ptype2.GetPixelTypeId() == ptype.GetPixelTypeId(), "ptype2( ptype)-GetPixelTypeId(");
      MITK_TEST_CONDITION_REQUIRED( ptype2.GetBpe() == 8*sizeof(int)*5, "ptype2( ptype)-GetBpe()");
      MITK_TEST_CONDITION_REQUIRED( ptype2.GetNumberOfComponents() == 5, "ptype2( ptype)-GetNumberOfComponents()");
      MITK_TEST_CONDITION_REQUIRED( ptype2.GetBitsPerComponent() == 8*sizeof(int), "ptype2( ptype)-GetBitsPerComponent()");
 //     MITK_TEST_CONDITION_REQUIRED( ptype.GetItkTypeAsString().compare("unknown") == 0, "ptype2( ptype)-GetItkTypeAsString()");
    }

    {
      mitk::PixelType ptype2 = ptype; 
      MITK_TEST_CONDITION_REQUIRED( ptype2.GetTypeId() == typeid(int), "ptype2 = ptype- GetTypeId()");
      MITK_TEST_CONDITION( ptype2.GetPixelTypeId() == ptype.GetPixelTypeId(), "ptype2 = ptype- GetPixelTypeId(");
      MITK_TEST_CONDITION_REQUIRED( ptype2.GetBpe() == 8*sizeof(int)*5, "ptype2 = ptype- GetBpe()");
      MITK_TEST_CONDITION_REQUIRED( ptype2.GetNumberOfComponents() == 5, "ptype2 = ptype- GetNumberOfComponents()");
      MITK_TEST_CONDITION_REQUIRED( ptype2.GetBitsPerComponent() == 8*sizeof(int), "ptype2 = ptype- GetBitsPerComponent()");
 //     MITK_TEST_CONDITION_REQUIRED( ptype.GetItkTypeAsString().compare("unknown") == 0, "ptype2 = ptype- GetItkTypeAsString()");
    }

    {
      mitk::PixelType ptype2 = ptype; 
      MITK_TEST_CONDITION_REQUIRED( ptype == ptype2, "operator ==");
      //MITK_TEST_CONDITION_REQUIRED( ptype == typeid(int), "operator ==");
      //mitk::PixelType ptype3 = mitk::MakePixelType<char, char ,5>;
      //MITK_TEST_CONDITION_REQUIRED( ptype != ptype3, "operator !=");
      //MITK_TEST_CONDITION_REQUIRED( *ptype3 != typeid(int), "operator !="); 
    }  
  }

  MITK_TEST_END();
}


