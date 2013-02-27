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

#ifndef MITKSERVICEBASEOBJECT_H
#define MITKSERVICEBASEOBJECT_H

#include <itkLightObject.h>

#include <mitkServiceInterface.h>

/** Documentation
 *  @brief This template is used to test the C++ micro services in MITK.
 */
template<> inline const char* us_service_impl_name(itk::LightObject* impl)
{ return impl->GetNameOfClass(); }

#endif // USERVICESBASEOBJECT_H
