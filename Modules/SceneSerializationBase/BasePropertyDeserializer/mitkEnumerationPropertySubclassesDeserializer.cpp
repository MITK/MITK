/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: 1.12 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

// TODO this should be moved to module SceneSerializationExt

#ifndef mitkEnumerationSubclassesDeserializer_h_included
#define mitkEnumerationSubclassesDeserializer_h_included

#include "mitkBasePropertyDeserializer.h"

/*
#include "mitkGridRepresentationProperty.h"
#include "mitkGridVolumeMapperProperty.h"
#include "mitkOrganTypeProperty.h"
*/
#include "mitkPlaneDecorationProperty.h"
#include "mitkShaderEnumProperty.h"
#include "mitkVtkInterpolationProperty.h"
#include "mitkVtkRepresentationProperty.h"
#include "mitkVtkResliceInterpolationProperty.h"
#include "mitkVtkScalarModeProperty.h"
#include "mitkVtkVolumeRenderingProperty.h"
#include "mitkModalityProperty.h"

#include "SceneSerializationBaseExports.h"


#define MITK_REGISTER_ENUM_SUB_DESERIALIZER(classname) \
 \
namespace mitk \
{ \
 \
class SceneSerializationBase_EXPORT classname ## Deserializer : public BasePropertyDeserializer \
{ \
  public: \
     \
    mitkClassMacro( classname ## Deserializer, BasePropertyDeserializer ); \
    itkNewMacro(Self); \
\
    virtual BaseProperty::Pointer Deserialize(TiXmlElement* element) \
    { \
      if (!element) return NULL; \
      const char* sa( element->Attribute("value") ); \
      std::string s(sa?sa:""); \
      classname::Pointer property = classname::New(); \
      property->SetValue( s ); \
      return property.GetPointer(); \
    } \
 \
 \
  protected: \
 \
    classname ## Deserializer () {} \
    virtual ~classname ## Deserializer () {} \
}; \
 \
} \
 \
MITK_REGISTER_SERIALIZER( classname ## Deserializer );

/*
MITK_REGISTER_ENUM_SUB_DESERIALIZER(GridRepresentationProperty);
MITK_REGISTER_ENUM_SUB_DESERIALIZER(GridVolumeMapperProperty);
MITK_REGISTER_ENUM_SUB_DESERIALIZER(OrganTypeProperty);
*/
MITK_REGISTER_ENUM_SUB_DESERIALIZER(PlaneDecorationProperty);
MITK_REGISTER_ENUM_SUB_DESERIALIZER(ShaderEnumProperty);
MITK_REGISTER_ENUM_SUB_DESERIALIZER(VtkInterpolationProperty);
MITK_REGISTER_ENUM_SUB_DESERIALIZER(VtkRepresentationProperty);
MITK_REGISTER_ENUM_SUB_DESERIALIZER(VtkResliceInterpolationProperty);
MITK_REGISTER_ENUM_SUB_DESERIALIZER(VtkScalarModeProperty);
MITK_REGISTER_ENUM_SUB_DESERIALIZER(VtkVolumeRenderingProperty);
MITK_REGISTER_ENUM_SUB_DESERIALIZER(ModalityProperty);
#endif
