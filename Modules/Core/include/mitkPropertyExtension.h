/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPropertyExtension_h
#define mitkPropertyExtension_h

#include <MitkCoreExports.h>
#include <itkObjectFactory.h>
#include <mitkCommon.h>

namespace mitk
{
  /** \brief Base class for all property extensions.
    */
  class MITKCORE_EXPORT PropertyExtension : public itk::LightObject
  {
  public:
    mitkClassMacroItkParent(PropertyExtension, itk::LightObject);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

      protected : PropertyExtension();
    ~PropertyExtension() override;

  private:
    PropertyExtension(const Self &other);
    Self &operator=(const Self &other);
  };
}

#endif
