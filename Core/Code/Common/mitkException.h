/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef MITKEXCEPTION_H_INCLUDED
#define MITKEXCEPTION_H_INCLUDED

#include <MitkExports.h>
#include <itkExceptionObject.h>
#include "mitkExceptionMacro.h"

namespace mitk {
  /**Documentation
  * \brief An object of this class represents an exception of the MITK.
  *
  */  
  class MITK_CORE_EXPORT Exception : itk::ExceptionObject 
  {
  
  public:
    Exception(const char *file, unsigned int lineNumber=0,
                  const char *desc="None", const char *loc="Unknown") :
    itk::ExceptionObject(file,lineNumber,desc,loc){}
    
    virtual ~Exception() throw() {}

    itkTypeMacro(ClassName, SuperClassName);
   
  };
} // namespace mitk
#endif
