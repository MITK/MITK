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


#include "mitkReportGenerator.h"
#include <itksys/SystemTools.hxx>

int mitkReportGeneratorTest(int argc, char* argv[])
{
  mitk::ReportGenerator::Pointer generator;
  std::cout << "Testing mitk::ReportGenerator::New(): ";
  generator = mitk::ReportGenerator::New();
  if (generator.IsNull()) {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  else {
    std::cout<<"[PASSED]"<<std::endl;
  }
  std::string path = argv[1];
  itksys::SystemTools::ConvertToUnixSlashes(path);
  std::string fileIn = path + "/reportTemplate.txt";
  std::string fileOut = path + "/report.txt";

  
  fileIn = itksys::SystemTools::ConvertToOutputPath(fileIn.c_str());
  fileOut = itksys::SystemTools::ConvertToOutputPath(fileOut.c_str());
  std::cout<<"Eingabe Datei: "<<fileIn<<std::endl;
  std::cout<<"Ausgabe Datei: "<<fileOut<<std::endl;
 
  generator->SetTemplateFileName(fileIn);
  generator->SetOutputFileName(fileOut);
  generator->AddKey("%KEY_A%","VALUE_A");
  generator->GetKeyValueMap().insert(mitk::ReportGenerator::KeyValueMapType::value_type("%KEY_B%","VALUE_B"));
  generator->Generate();
  
  return EXIT_SUCCESS;
}
