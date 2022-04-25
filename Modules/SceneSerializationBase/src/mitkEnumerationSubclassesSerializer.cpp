/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkEnumerationSubclassesSerializer_h_included
#define mitkEnumerationSubclassesSerializer_h_included

#include "mitkEnumerationPropertySerializer.h"

#include "mitkModalityProperty.h"
#include "mitkPlaneOrientationProperty.h"
#include "mitkPointSetShapeProperty.h"
#include "mitkRenderingModeProperty.h"
#include "mitkVtkInterpolationProperty.h"
#include "mitkVtkRepresentationProperty.h"
#include "mitkVtkResliceInterpolationProperty.h"
#include "mitkVtkScalarModeProperty.h"

#include <tinyxml2.h>

#define MITK_REGISTER_ENUM_SUB_SERIALIZER(classname)                                        \
                                                                                            \
namespace mitk                                                                              \
{                                                                                           \
  class classname##Serializer : public EnumerationPropertySerializer                        \
  {                                                                                         \
  public:                                                                                   \
    mitkClassMacro(classname##Serializer, EnumerationPropertySerializer)                    \
    itkFactorylessNewMacro(Self)                                                            \
    itkCloneMacro(Self)                                                                     \
                                                                                            \
    virtual BaseProperty::Pointer Deserialize(const tinyxml2::XMLElement *element) override \
    {                                                                                       \
      if (nullptr == element)                                                               \
        return nullptr;                                                                     \
                                                                                            \
      classname::Pointer property = classname::New();                                       \
      property->SetValue(element->Attribute("value"));                                      \
                                                                                            \
      return property.GetPointer();                                                         \
  }                                                                                         \
                                                                                            \
  protected:                                                                                \
    classname##Serializer() {}                                                              \
    virtual ~classname##Serializer() {}                                                     \
  };                                                                                        \
}                                                                                           \
                                                                                            \
MITK_REGISTER_SERIALIZER(classname##Serializer);

MITK_REGISTER_ENUM_SUB_SERIALIZER(PlaneOrientationProperty);
MITK_REGISTER_ENUM_SUB_SERIALIZER(VtkInterpolationProperty);
MITK_REGISTER_ENUM_SUB_SERIALIZER(VtkRepresentationProperty);
MITK_REGISTER_ENUM_SUB_SERIALIZER(VtkResliceInterpolationProperty);
MITK_REGISTER_ENUM_SUB_SERIALIZER(VtkScalarModeProperty);
MITK_REGISTER_ENUM_SUB_SERIALIZER(ModalityProperty);
MITK_REGISTER_ENUM_SUB_SERIALIZER(RenderingModeProperty);
MITK_REGISTER_ENUM_SUB_SERIALIZER(PointSetShapeProperty);

#endif
