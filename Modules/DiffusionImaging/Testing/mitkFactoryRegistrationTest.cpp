/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-08-05 17:32:40 +0200 (Mi, 05 Aug 2009) $
Version:   $Revision: 7837 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkTestingMacros.h"

#include "mitkDiffusionImagingObjectFactory.h"
#include "mitkCoreObjectFactory.h"
#include "mitkDiffusionImage.h"

/**Documentation
 *  Test for factory registration
 */
int mitkFactoryRegistrationTest(int /* argc */, char* /*argv*/[])
{
  // always start with this!
  MITK_TEST_BEGIN("FactoryRegistrationTest");

  RegisterDiffusionImagingObjectFactory();

  bool canWrite = false;
  mitk::DiffusionImage<short>::Pointer img = mitk::DiffusionImage<short>::New();
  mitk::CoreObjectFactory::FileWriterList fileWriters = mitk::CoreObjectFactory::GetInstance()->GetFileWriters();
  for (mitk::CoreObjectFactory::FileWriterList::iterator it = fileWriters.begin() ; it != fileWriters.end() ; ++it) 
            {
              if ( (*it)->CanWriteDataType(img.GetPointer()) ) {
                canWrite = true;
                break;
              } 
            }

  MITK_TEST_CONDITION_REQUIRED(canWrite,"Testing factory registration");

  // always end with this!
  MITK_TEST_END();
}
