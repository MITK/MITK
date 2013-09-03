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
  class NavigationDataReaderInterface : public itk::Object
  {
  public:
    mitkClassMacro(NavigationDataReaderInterface, itk::Object);

    itkSetMacro(FileName, std::string);
    itkGetMacro(FileName, std::string);

    virtual mitk::NavigationDataSet::Pointer Read() = 0;

  protected:
    NavigationDataReaderInterface();
    virtual ~NavigationDataReaderInterface();

    std::string m_FileName;
  };
}

#endif // MITKNavigationDataReaderInterface_H_HEADER_INCLUDED_
