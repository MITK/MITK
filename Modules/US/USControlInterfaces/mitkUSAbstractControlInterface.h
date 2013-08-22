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


#ifndef MITKUSAbstractControlInterface_H_HEADER_INCLUDED_
#define MITKUSAbstractControlInterface_H_HEADER_INCLUDED_

#include <mitkCommon.h>
#include <MitkUSExports.h>

namespace mitk {

  class MitkUS_EXPORT USAbstractControlInterface : public itk::Object
  {
  public:
    mitkClassMacro(USAbstractControlInterface, itk::Object);

    virtual void SetIsActive(bool) = 0;
    virtual bool GetIsActive() = 0;

  protected:
    USAbstractControlInterface();
    virtual ~USAbstractControlInterface();
  };

}

#endif // MITKUSAbstractControlInterface_H_HEADER_INCLUDED_