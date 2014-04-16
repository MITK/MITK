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


#ifndef MITKNavigationDataReaderInterface_H_HEADER_INCLUDED_
#define MITKNavigationDataReaderInterface_H_HEADER_INCLUDED_

#include "itkObject.h"
#include "mitkCommon.h"
#include "mitkNavigationDataSet.h"

namespace mitk {
  class MitkIGT_EXPORT NavigationDataReaderInterface : public itk::Object
  {
  public:
    mitkClassMacro(NavigationDataReaderInterface, itk::Object);

    virtual mitk::NavigationDataSet::Pointer Read(std::string filename) = 0;

  protected:
    NavigationDataReaderInterface();
    virtual ~NavigationDataReaderInterface();
  };
}

#endif // MITKNavigationDataReaderInterface_H_HEADER_INCLUDED_
