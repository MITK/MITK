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


//## Documentation
//## main testing method
int mitkPixelTypeTest(int /*argc*/, char* /*argv*/[])
{
    MITK_TEST_BEGIN("PixelTypeTest");

    
      mitk::PixelType ptype( typeid(int) ,5);
      MITK_TEST_CONDITION_REQUIRED(( (*ptype.GetTypeId()) == typeid(int)), "GetTypeId()");
      MITK_TEST_CONDITION_REQUIRED(( ptype == typeid(int)), "operator ==");
      MITK_TEST_CONDITION_REQUIRED(( (ptype.GetItkTypeId()) == NULL), "GetItkTypeId(");
      std::cout<<sizeof(int)<<std::endl;
      MITK_TEST_CONDITION_REQUIRED(( (ptype.GetBpe()) == 8*sizeof(int)*5), "GetBpe()");
      MITK_TEST_CONDITION_REQUIRED(( (ptype.GetNumberOfComponents()) == 5), "GetNumberOfComponents()");
      MITK_TEST_CONDITION_REQUIRED(( (ptype.GetBitsPerComponent()) == 8*sizeof(int)), "GetBitsPerComponent()");
      MITK_TEST_CONDITION_REQUIRED(( (ptype.GetItkTypeAsString()).compare("int")), "GetItkTypeAsString()");
    {
    {
      mitk::PixelType ptype2( ptype); 
      MITK_TEST_CONDITION_REQUIRED(( (*ptype.GetTypeId()) == typeid(int)), "ptype2( ptype)- GetTypeId()");
      MITK_TEST_CONDITION_REQUIRED(( (ptype.GetItkTypeId()) == NULL), "ptype2( ptype)-GetItkTypeId(");
      MITK_TEST_CONDITION_REQUIRED(( (ptype.GetBpe()) == 8*sizeof(int)*5), "ptype2( ptype)-GetBpe()");
      MITK_TEST_CONDITION_REQUIRED(( (ptype.GetNumberOfComponents()) == 5), "ptype2( ptype)-GetNumberOfComponents()");
      MITK_TEST_CONDITION_REQUIRED(( (ptype.GetBitsPerComponent()) == 8*sizeof(int)), "ptype2( ptype)-GetBitsPerComponent()");
      MITK_TEST_CONDITION_REQUIRED(( (ptype.GetItkTypeAsString()).compare("int")), "ptype2( ptype)-GetItkTypeAsString()");
    }
    {
      mitk::PixelType ptype2 = ptype; 
      MITK_TEST_CONDITION_REQUIRED(( (*ptype.GetTypeId()) == typeid(int)), "ptype2 = ptype- GetTypeId()");
      MITK_TEST_CONDITION_REQUIRED(( (ptype.GetItkTypeId()) == NULL), "ptype2 = ptype- GetItkTypeId(");
      MITK_TEST_CONDITION_REQUIRED(( (ptype.GetBpe()) == 8*sizeof(int)*5), "ptype2 = ptype- GetBpe()");
      MITK_TEST_CONDITION_REQUIRED(( (ptype.GetNumberOfComponents()) == 5), "ptype2 = ptype- GetNumberOfComponents()");
      MITK_TEST_CONDITION_REQUIRED(( (ptype.GetBitsPerComponent()) == 8*sizeof(int)), "ptype2 = ptype- GetBitsPerComponent()");
      MITK_TEST_CONDITION_REQUIRED(( (ptype.GetItkTypeAsString()).compare("int")), "ptype2 = ptype- GetItkTypeAsString()");
    }
    {
      mitk::PixelType ptype2 = ptype; 
      MITK_TEST_CONDITION_REQUIRED(( ptype == ptype2), "operator ==");
      MITK_TEST_CONDITION_REQUIRED(( ptype == typeid(int)), "operator ==");
      mitk::PixelType ptype3(typeid(char) ,5); 
      MITK_TEST_CONDITION_REQUIRED(( ptype != ptype3), "operator !=");
      //MITK_TEST_CONDITION_REQUIRED(( (*ptype3) != typeid(int)), "operator !="); 
    }
    
    
    
  }

 

  /* Create StandaloneDataStorage */
  MITK_TEST_OUTPUT( << "Create StandaloneDataStorage : ");
 
  MITK_TEST_CONDITION_REQUIRED(true, "Testing Instatiation");

  MITK_TEST_FAILED_MSG( << "Exception during creation of StandaloneDataStorage");
  

  MITK_TEST_OUTPUT( << "Testing StandaloneDataStorage: ");

  MITK_TEST_END();
}


