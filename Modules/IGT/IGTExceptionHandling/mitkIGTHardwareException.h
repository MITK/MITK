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

#ifndef IGTHARDWAREEXCEPTION_H_INCLUDED
#define IGTHARDWAREEXCEPTION_H_INCLUDED

#include "mitkIGTException.h"
#include "mitkExceptionMacro.h"

namespace mitk {
  /**Documentation
  * \brief An object of this class represents an exception of the MITK-IGT module which are releated to the hardware (e.g. connection problems, etc.).
  *
  * \ingroup IGT
  */  class MitkIGT_EXPORT IGTHardwareException : public mitk::IGTException
  {
  public:
    mitkExceptionClassMacro(IGTHardwareException,mitk::IGTException);
  };
} // namespace mitk
#endif
