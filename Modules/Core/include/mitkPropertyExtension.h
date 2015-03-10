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

#ifndef mitkPropertyExtension_h
#define mitkPropertyExtension_h

#include <mitkCommon.h>
#include <itkObjectFactory.h>
#include <MitkCoreExports.h>

namespace mitk
{
  /** \brief Base class for all property extensions.
    */
  class MITKCORE_EXPORT PropertyExtension : public itk::LightObject
  {
  public:
    mitkClassMacro(PropertyExtension, itk::LightObject);
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

  protected:
    PropertyExtension();
    virtual ~PropertyExtension();

  private:
    PropertyExtension(const Self& other);
    Self& operator=(const Self& other);
  };
}

#endif
