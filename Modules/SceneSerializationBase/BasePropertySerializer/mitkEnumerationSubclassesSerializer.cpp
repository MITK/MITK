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

#ifndef mitkEnumerationSubclassesSerializer_h_included
#define mitkEnumerationSubclassesSerializer_h_included

#include "mitkEnumerationPropertySerializer.h"

#include "mitkPlaneOrientationProperty.h"
#include "mitkShaderProperty.h"
#include "mitkVtkInterpolationProperty.h"
#include "mitkVtkRepresentationProperty.h"
#include "mitkVtkResliceInterpolationProperty.h"
#include "mitkVtkScalarModeProperty.h"
#include "mitkVtkVolumeRenderingProperty.h"
#include "mitkModalityProperty.h"

#define MITK_REGISTER_ENUM_SUB_SERIALIZER(classname) \
 \
namespace mitk \
{ \
 \
class SceneSerializationBase_EXPORT classname ## Serializer : public EnumerationPropertySerializer \
{ \
  public: \
     \
    mitkClassMacro( classname ## Serializer, EnumerationPropertySerializer ); \
    itkNewMacro(Self); \
 \
    virtual BaseProperty::Pointer Deserialize(TiXmlElement* element) \
    { \
        if (!element) return NULL; \
            const char* sa( element->Attribute("value") ); \
         \
        std::string s(sa?sa:""); \
        classname::Pointer property = classname::New(); \
        property->SetValue( s ); \
         \
        return property.GetPointer(); \
    } \
 \
  protected: \
 \
    classname ## Serializer () {} \
    virtual ~classname ## Serializer () {} \
}; \
 \
} \
 \
MITK_REGISTER_SERIALIZER( classname ## Serializer );

MITK_REGISTER_ENUM_SUB_SERIALIZER(PlaneOrientationProperty);
MITK_REGISTER_ENUM_SUB_SERIALIZER(ShaderProperty);
MITK_REGISTER_ENUM_SUB_SERIALIZER(VtkInterpolationProperty);
MITK_REGISTER_ENUM_SUB_SERIALIZER(VtkRepresentationProperty);
MITK_REGISTER_ENUM_SUB_SERIALIZER(VtkResliceInterpolationProperty);
MITK_REGISTER_ENUM_SUB_SERIALIZER(VtkScalarModeProperty);
MITK_REGISTER_ENUM_SUB_SERIALIZER(VtkVolumeRenderingProperty);
MITK_REGISTER_ENUM_SUB_SERIALIZER(ModalityProperty);

#endif

