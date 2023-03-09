/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef mitkNavigationDataReaderInterface_h
#define mitkNavigationDataReaderInterface_h

#include "itkObject.h"
#include "mitkCommon.h"
#include "mitkNavigationDataSet.h"
#include "MitkIGTExports.h"

namespace mitk {
  class MITKIGT_EXPORT NavigationDataReaderInterface : public itk::Object
  {
  public:
    mitkClassMacroItkParent(NavigationDataReaderInterface, itk::Object);

    virtual mitk::NavigationDataSet::Pointer Read(std::string filename) = 0;

  protected:
    NavigationDataReaderInterface();
    ~NavigationDataReaderInterface() override;
  };
}

#endif
