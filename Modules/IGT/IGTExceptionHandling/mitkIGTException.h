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

#ifndef IGTEXCEPTION_H_INCLUDED
#define IGTEXCEPTION_H_INCLUDED

#include <MitkIGTExports.h>
#include <itkExceptionObject.h>

namespace mitk {
  /**Documentation
  * \brief An object of this class represents an exception of the MITK-IGT module.
  *
  * \ingroup IGT
  */  class MitkIGT_EXPORT IGTException : public itk::ExceptionObject //perhaps this should later derive from mitk::Exception (if there is an exception handling for complete MITK
  {
  
  };
} // namespace mitk
#endif
