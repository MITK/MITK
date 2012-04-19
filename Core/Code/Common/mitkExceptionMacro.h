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
#include <mitkLogMacros.h>

/** The exception macro is used to print error information / throw an exception 
 *  (i.e., usually a condition that results in program failure). 
 *
 *  Example usage looks like:
 *  mitkExceptionMacro("this is error info");
 */
#define mitkExceptionMacro(messagetext) {mitkSpecializedExceptionMacro(messagetext,mitk::Exception);}

/** The specialized exception macro is used to print error information / throw exceptions 
  * in cases of specialized errors. This means the second parameter must be a class which 
  * inherits from mitk::Exception. An object of this exception is thrown when using the macro.
  * Thus, more differentiated excaptions can be thrown, when needed.
  *
  * Example usage:
  * mitkSpecializedExceptionMacro("this is error info",mitk::MySpecializedException); 
  */
#define mitkSpecializedExceptionMacro(messagetext,classname) \
  { \
  ::itk::OStringStream message; \
  message << "mitk::EXCEPTION: " \
  << "(Object:" << this->GetNameOfClass() << "(" << this << ")" << "/File:" << __FILE__ << "/Line:" << __LINE__ << "): " messagetext; \
  MITK_DEBUG << message.str().c_str(); /* Print a MITK debug message to log exceptions in debug mode. */ \
  throw classname(__FILE__, __LINE__, message.str().c_str(),ITK_LOCATION); /* Explicit naming to work around Intel compiler bug.  */ \
  }
  
#endif