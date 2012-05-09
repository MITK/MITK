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


#include <itkMacro.h>
#include <mitkLogMacros.h>
#include <sstream>
#include "mitkException.h"

/** Class macro for MITK exception classes.
  * All MITK exception classes should derive from MITK::Exception.
  */
#define mitkExceptionClassMacro(ClassName,SuperClassName) \
    ClassName(const char *file, unsigned int lineNumber, const char *desc, const char *loc) :\
    SuperClassName(file,lineNumber,desc,loc){}\
    itkTypeMacro(ClassName, SuperClassName);\
  
/**
 * TODO
 */
#define mitkThrow() mitk::ExceptionMessagePseudoStream(__FILE__,__LINE__,ITK_LOCATION,mitk::Exception(__FILE__,__LINE__,"",ITK_LOCATION))

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

namespace mitk {
/** @Documentation: TODO
 *
 *
 */
 class MITK_CORE_EXPORT ExceptionMessagePseudoStream {

   protected:

      std::stringstream ss;

      const char* m_filePath;
      int m_lineNumber;
      const char* m_itkLocation;
      mitk::Exception m_exceptionToThrow;
      
    
   public:

   inline ExceptionMessagePseudoStream(const char* filePath,
                                int lineNumber,
                                const char* itkLocation,mitk::Exception exception)
                                : ss(std::stringstream::out), 
                                  m_filePath(filePath), 
                                  m_lineNumber(lineNumber), 
                                  m_itkLocation(itkLocation),
                                  m_exceptionToThrow(exception)

      {
    
      }

	    /** \brief TODO */
      inline ~ExceptionMessagePseudoStream()
      {
        m_exceptionToThrow.SetDescription(ss.str());
        throw m_exceptionToThrow;
      }

	  /** \brief Definition of the bit shift operator for this class.*/
      template <class T> inline ExceptionMessagePseudoStream& operator<<(const T& data)
      {
        std::locale C("C");
        std::locale originalLocale = ss.getloc();
        ss.imbue(C);
        ss << data;
        ss.imbue( originalLocale );
        return *this;
      }

	  /** \brief Definition of the bit shift operator for this class (for non const data).*/
      template <class T> inline ExceptionMessagePseudoStream& operator<<(T& data)
      {
        std::locale C("C");
        std::locale originalLocale = ss.getloc();
        ss.imbue(C);
        ss << data;
        ss.imbue( originalLocale );
        return *this;
      }

	  /** \brief Definition of the bit shift operator for this class (for functions).*/
      inline ExceptionMessagePseudoStream& operator<<(std::ostream& (*func)(std::ostream&))
      {
        std::locale C("C");
        std::locale originalLocale = ss.getloc();
        ss.imbue(C);
        ss << func;
        ss.imbue( originalLocale );
        return *this;
      }
  };
}

#endif