/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef MITK_EXCEPTIONMACRO_H_DEFINED
#define MITK_EXCEPTIONMACRO_H_DEFINED

#include "mitkException.h"
#include <itkMacro.h>

/** The exception macro is used to print error information (i.e., usually
 * a condition that results in program failure). Example usage looks like:
 * mitkExceptionMacro(<< "this is error info" << this->SomeVariable); */
#define mitkExceptionMacro(x) \
  { \
  ::itk::OStringStream message; \
  message << "mitk::EXCEPTION: " << this->GetNameOfClass() \
          << "(" << this << "): " x; \
  mitk::Exception e_(__FILE__, __LINE__, message.str().c_str(),ITK_LOCATION); \
  throw e_; /* Explicit naming to work around Intel compiler bug.  */ \
  }
  
#endif