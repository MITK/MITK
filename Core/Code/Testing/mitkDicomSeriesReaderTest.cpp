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

#include "mitkTestingMacros.h"

#include <iostream>

#include "mitkDicomSeriesReader.h"


int mitkDicomSeriesReaderTest(int argc, char* argv[])
{
  // always start with this!
  MITK_TEST_BEGIN("DicomSeriesReader")
  mitk::DicomSeriesReader::StringContainer files;
 
  for (int i = 1 ; i < argc ; i++ )
  {
    files.push_back(std::string(argv[i]));
  }
  if (! files.empty() ) 
  {
    for (int i = 0 ; i < 10 ; i++ ) {
      mitk::DataNode::Pointer node = mitk::DicomSeriesReader::LoadDicomSeries(files);
    MITK_TEST_CONDITION_REQUIRED(node.IsNotNull(),"Testing node") 
  }
  }

  MITK_TEST_END()
}
