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


#include "mitkImage.h"
#include "mitkImageDataItem.h"

#include <fstream>
int mitkImageDataItemTest(int /*argc*/, char* /*argv*/[])
{
  unsigned long *pixels = new unsigned long [100];
  void * data = pixels;

  std::cout << "Testing pseudo-type independent deleting: ";
  delete [] (unsigned char*) data;
  std::cout<<"[PASSED]"<<std::endl;

  std::cout<<"[TEST DONE]"<<std::endl;
  return EXIT_SUCCESS;
}
