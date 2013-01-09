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

  MITK_INFO << "Starting Factory registration test.";

  RegisterDiffusionImagingObjectFactory();

  MITK_INFO << "Factory has been registered.";

  bool canWrite = false;
  mitk::DiffusionImage<short>::Pointer img = mitk::DiffusionImage<short>::New();
  mitk::CoreObjectFactory::FileWriterList fileWriters = mitk::CoreObjectFactory::GetInstance()->GetFileWriters();
  MITK_INFO << "Looking for diffusion image writer.";
  for (mitk::CoreObjectFactory::FileWriterList::iterator it = fileWriters.begin() ; it != fileWriters.end() ; ++it)
  {
    if ( (*it)->CanWriteBaseDataType(img.GetPointer()) )
    {
      MITK_INFO << "Found diffusion image writer.";
      canWrite = true;
      break;
    }
  }

  MITK_TEST_CONDITION_REQUIRED(canWrite,"Testing factory registration");

  // always end with this!
  MITK_TEST_END();
}
