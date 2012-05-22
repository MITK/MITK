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

#ifndef MITKEXCEPTION_H_INCLUDED
#define MITKEXCEPTION_H_INCLUDED

#include <MitkExports.h>
#include <itkExceptionObject.h>

namespace mitk {
  /**Documentation
  * \brief   An object of this class represents an exception of MITK.
  *          Please don't instantiate exceptions manually, but use the
  *          exception macros (file mitkExceptionMacro.h) instead.
  *          Simple use in your code is:
  *
  *                  mitkThrow() << "optional exception message";
  *
  *          You can also define specialized exceptions which must inherit
  *          from this class. Please always use the mitkExceptionClassMacro
  *          when implementing specialized exceptions. A simple implementation
  *          can look like:
  *
  *          class MyException : public mitk::Exception
  *             {
  *             public:
  *             mitkExceptionClassMacro(MyException,mitk::Exception);
  *             };
  *
  *          You can then throw your specialized exceptions by using the macro
  *
  *                 mitkThrowException(MyException) << "optional exception message";
  */  
  class MITK_CORE_EXPORT Exception : public itk::ExceptionObject 
  {
  
  public:
    Exception(const char *file, unsigned int lineNumber=0,
                  const char *desc="None", const char *loc="Unknown") :
    itk::ExceptionObject(file,lineNumber,desc,loc){}
    
    virtual ~Exception() throw() {}

    itkTypeMacro(ClassName, SuperClassName);

    /** \brief Definition of the bit shift operator for this class.*/
    template <class T> inline Exception& operator<<(const T& data)
    {
      std::stringstream ss;
      ss << this->GetDescription() << data;
      this->SetDescription(ss.str());
      return *this;
    }

    /** \brief Definition of the bit shift operator for this class (for non const data).*/
    template <class T> inline Exception& operator<<(T& data)
    {
      std::stringstream ss;
      ss << this->GetDescription() << data;
      this->SetDescription(ss.str());
      return *this;
    }

    /** \brief Definition of the bit shift operator for this class (for functions).*/
    inline Exception& operator<<(std::ostream& (*func)(std::ostream&))
    {
      std::stringstream ss;
      ss << this->GetDescription() << func;
      this->SetDescription(ss.str());
      return *this;
    }
   
  };
} // namespace mitk
#endif
