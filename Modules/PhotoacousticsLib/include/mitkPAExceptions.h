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

#ifndef MITKEXCEPTIONS_H
#define MITKEXCEPTIONS_H

#include <MitkPhotoacousticsLibExports.h>

#include <mitkCommon.h>

//std
#include <string>
#include <exception>

namespace mitk {
  namespace pa {
    /**
    *@brief this exception is thrown if an invalid input occurs
    */
    class MITKPHOTOACOUSTICSLIB_EXPORT InvalidInputException : public std::exception
    {
      std::string m_InvalidParameter;
    public:
      InvalidInputException(std::string invalidparameter)
      {
        m_InvalidParameter = invalidparameter;
        MITK_ERROR << GetFullMessage();
      }

      const std::string GetFullMessage() const throw()
      {
        return "InvalidInputException: " + m_InvalidParameter;
      }
    };

    /**
    *@brief this exception is thrown if an invalid value is supposed to be processed
    */
    class MITKPHOTOACOUSTICSLIB_EXPORT InvalidValueException : public std::exception
    {
      std::string m_ValueType;
    public:
      InvalidValueException(std::string valueType)
      {
        m_ValueType = valueType;
        MITK_ERROR << GetFullMessage();
      }

      const std::string GetFullMessage() const throw()
      {
        return "InvalidValueException: " + m_ValueType;
      }
    };
  }
}

#endif // MITKEXCEPTIONS_H
