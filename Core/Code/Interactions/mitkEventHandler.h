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

#ifndef MITKEVENTHANDLER_H_
#define MITKEVENTHANDLER_H_

#include "itkObject.h"
#include "itkObjectFactory.h"
#include "mitkEvent.h"
#include "mitkCommon.h"
#include <MitkExports.h>


namespace mitk
{

  class MITK_CORE_EXPORT EventHandler : public itk::Object {

  public:
    mitkClassMacro(EventHandler, itk::Object);
    itkNewMacro(Self);

  protected:
    EventHandler();
    virtual ~EventHandler();


  };

} /* namespace mitk */
#endif /* MITKEVENTHANDLER_H_ */
