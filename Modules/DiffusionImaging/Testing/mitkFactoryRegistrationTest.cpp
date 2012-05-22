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
              if ( (*it)->CanWriteBaseDataType(img.GetPointer()) ) {
                canWrite = true;
                break;
              } 
            }

  MITK_TEST_CONDITION_REQUIRED(canWrite,"Testing factory registration");

  // always end with this!
  MITK_TEST_END();
}
