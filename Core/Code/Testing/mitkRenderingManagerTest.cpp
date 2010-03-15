/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2008-02-25 17:27:17 +0100 (Mo, 25 Feb 2008) $
Version:   $Revision: 7837 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkRenderingManager.h"
#include "mitkProperties.h"

#include "mitkTestingMacros.h"


//Propertylist Test



/**
 *  Simple example for a test for the class "RenderingManager".
 *  
 *  argc and argv are the command line parameters which were passed to 
 *  the ADD_TEST command in the CMakeLists.txt file. For the automatic
 *  tests, argv is either empty for the simple tests or contains the filename
 *  of a test image for the image tests (see CMakeLists.txt).
 */
class mitkRenderingManagerTestClass { public:



static void TestPropertyList(mitk::RenderingManager::Pointer renderingManager)
{

  mitk::PropertyList::Pointer propertyList =  renderingManager->GetPropertyList();
   
  MITK_TEST_CONDITION(renderingManager->GetPropertyList().IsNotNull(), "Testing if the constructor set the propertylist" )

  //check if the default properties are set
  renderingManager->SetProperty("booltest", mitk::BoolProperty::New(true));

  mitk::BoolProperty* prop = dynamic_cast<mitk::BoolProperty*>( renderingManager->GetProperty("booltest") );

  MITK_TEST_CONDITION(prop->GetValue(), "Testing if getting the bool property" )
  
  MITK_TEST_CONDITION(propertyList == renderingManager->GetPropertyList(), "Testing if the propertylist has changed during the last tests" )
}

}; //mitkDataNodeTestClass
int mitkRenderingManagerTest(int /* argc */, char* /*argv*/[])
{
  // always start with this!
  MITK_TEST_BEGIN("RenderingManager")


  mitk::RenderingManager::Pointer myRenderingManager = mitk::RenderingManager::New();

  MITK_TEST_CONDITION_REQUIRED(myRenderingManager.IsNotNull(),"Testing instantiation") 


  mitkRenderingManagerTestClass::TestPropertyList(myRenderingManager);


  // write your own tests here and use the macros from mitkTestingMacros.h !!!
  // do not write to std::cout and do not return from this function yourself!
  
  // always end with this!
  MITK_TEST_END()
}
