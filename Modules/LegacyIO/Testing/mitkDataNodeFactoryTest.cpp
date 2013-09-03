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

#include "mitkDataNodeFactory.h"
#include "mitkTestingMacros.h"

#include "mitkProperties.h"

#include <itksys/SystemTools.hxx>
#include <itksys/Directory.hxx>
/**
 *  Test for the class "DataNodeFactory".
 *
 *  argc and argv are the command line parameters which were passed to
 *  the ADD_TEST command in the CMakeLists.txt file. For the automatic
 *  tests, argv is either empty for the simple tests or contains the filename
 *  of a test image for the image tests (see CMakeLists.txt).
 */
int mitkDataNodeFactoryTest(int, char* argv[])
{
  // always start with this!
  MITK_TEST_BEGIN("DataNodeFactory")

    mitk::DataNodeFactory::Pointer factory = mitk::DataNodeFactory::New();
    MITK_TEST_OUTPUT(<< "Loading file: " << argv[1]);

    factory->SetFileName( argv[1] );
    MITK_TEST_CONDITION_REQUIRED(strcmp(factory->GetFileName(),argv[1])==0,"Test for Set/GetFileName()");

    factory->Update();
    MITK_TEST_CONDITION_REQUIRED(factory->GetNumberOfOutputs() > 0, "file loaded");

    MITK_TEST_OUTPUT(<< "Test function SetDefaultCommonProperties()");
    mitk::DataNode::Pointer node = factory->GetOutput( 0 );

    MITK_TEST_CONDITION_REQUIRED( node.IsNotNull(), "DataNodeFactory has returned a non-empty node.")

    factory->SetDefaultCommonProperties(node);
    // get file path and property
    std::string filePath = itksys::SystemTools::GetFilenamePath(factory->GetFileName());
    mitk::StringProperty::Pointer pathProp = dynamic_cast<mitk::StringProperty*>(node->GetProperty(mitk::StringProperty::PATH));

    MITK_TEST_CONDITION_REQUIRED(strcmp(pathProp->GetValue(),filePath.c_str())==0,"Test for file path");

    std::string fileName = factory->GetFileName();
    std::string fileExtension = itksys::SystemTools::GetFilenameExtension(fileName);
    if (fileName.substr(fileName.size()-3) == ".gz")
       fileName = fileName.substr( 0, fileName.length()-3 );
    fileName = fileName.substr(0,fileName.length()-fileExtension.length());
    fileName = fileName.substr(filePath.length()+1, fileName.length());
    mitk::StringProperty::Pointer nameProp = dynamic_cast<mitk::StringProperty*>(node->GetProperty("name"));
    MITK_TEST_CONDITION_REQUIRED(strcmp(nameProp->GetValue(),fileName.c_str())==0,"Test for file name");

    mitk::BoolProperty::Pointer visibleProp = dynamic_cast<mitk::BoolProperty*>(node->GetProperty("visible"));
    MITK_TEST_CONDITION_REQUIRED(visibleProp->GetValue()==true,"Test for visibility");
  // always end with this!
  MITK_TEST_END()
}
