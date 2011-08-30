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


#ifndef MITKMODULEPROPSINTERFACE_H
#define MITKMODULEPROPSINTERFACE_H

#include <mitkServiceInterface.h>
#include <mitkServiceProperties.h>

namespace mitk {

struct ModulePropsInterface
{
  typedef ServiceProperties Properties;

  virtual ~ModulePropsInterface() {}

  virtual const Properties& GetProperties() const = 0;
};

}

MITK_DECLARE_SERVICE_INTERFACE(mitk::ModulePropsInterface, "org.mitk.testing.ModulePropsInterface")

#endif // MITKMODULEPROPSINTERFACE_H
