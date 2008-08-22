/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile: mitkPropertyManager.cpp,v $
Language:  C++
Date:      $Date$
Version:   $Revision: 1.12 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#include <iostream>
#include <string>

#include <stdlib.h> // for system() call

int mitkExternalToolsTest(int argc, char* argv[])
{
  std::cout << "Got " << argc << " parameters" << std::endl;
  if ( argc == 4 )
  {
    // "parse" commandline
    std::string cmakeBinary( argv[1] );
    std::string mitkBinaryDirectory( argv[2] );
    std::string sourceDirectory( argv[3] );

    // try to configure MITK external project
    std::cout << "Caling CMake as '" << cmakeBinary << "'" << std::endl;
    std::cout << "MITK was compiled in '" << mitkBinaryDirectory << "'" << std::endl;
    std::cout << "Configuring project in '" << sourceDirectory << "'" << std::endl;

    std::string commandline(cmakeBinary);
    
	commandline += " -DMITK_DIR:PATH=";

	commandline += """";
    commandline += mitkBinaryDirectory;
	commandline += """";
    commandline += " ";
	commandline += """";
    commandline += sourceDirectory;
	commandline += """";

    std::cout << "Calling system() with '" 
              << commandline
              << "'" 
              << std::endl;

    int returnCode = system(commandline.c_str());

    std::cout << "system() returned " << returnCode << std::endl;

    if (returnCode == EXIT_FAILURE)
    {
      std::cerr << "Configure FAILED. See output above." << std::endl;
      return EXIT_FAILURE;
    }

    // try to build MITK external project

#ifndef WIN32
    commandline = "make";
    returnCode = system(commandline.c_str());
#endif
    // 
    // TODO extend test here to support windows...
    //
    if (returnCode == EXIT_FAILURE)
    {
      std::cerr << "Building the project FAILED. See output above." << std::endl;
      return EXIT_FAILURE;
    }

    return returnCode;
  }
  else
  {
    std::cout << "Invoke this test with three parameters:" << std::endl;
    std::cout << "   1. CMake binary including necessary path" << std::endl;
    std::cout << "   2. MITK binary path (top-level directory)" << std::endl;
    std::cout << "   3. Source directory containing CMakeLists.txt" << std::endl;
    return EXIT_FAILURE;
  }
}

