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

#ifndef mitkConnectomicsEnumerationSubclassesSerializer_h_included
#define mitkConnectomicsEnumerationSubclassesSerializer_h_included

#include "mitkEnumerationPropertySerializer.h"

#include "mitkConnectomicsRenderingEdgeColorParameterProperty.h"
#include "mitkConnectomicsRenderingEdgeFilteringProperty.h"
#include "mitkConnectomicsRenderingEdgeRadiusParameterProperty.h"
#include "mitkConnectomicsRenderingEdgeThresholdParameterProperty.h"
#include "mitkConnectomicsRenderingNodeColorParameterProperty.h"
#include "mitkConnectomicsRenderingNodeFilteringProperty.h"
#include "mitkConnectomicsRenderingNodeRadiusParameterProperty.h"
#include "mitkConnectomicsRenderingNodeThresholdParameterProperty.h"
#include "mitkConnectomicsRenderingSchemeProperty.h"

#include <MitkConnectomicsExports.h>

#define MITK_REGISTER_ENUM_SUB_SERIALIZER(classname) \
 \
namespace mitk \
{ \
 \
class MITKCONNECTOMICS_EXPORT classname ## Serializer : public EnumerationPropertySerializer \
{ \
  public: \
     \
    mitkClassMacro( classname ## Serializer, EnumerationPropertySerializer ) \
    itkFactorylessNewMacro(Self) \
    itkCloneMacro(Self) \
 \
    virtual BaseProperty::Pointer Deserialize(TiXmlElement* element) \
    { \
        if (!element) return nullptr; \
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

MITK_REGISTER_ENUM_SUB_SERIALIZER(ConnectomicsRenderingEdgeColorParameterProperty);
MITK_REGISTER_ENUM_SUB_SERIALIZER(ConnectomicsRenderingEdgeFilteringProperty);
MITK_REGISTER_ENUM_SUB_SERIALIZER(ConnectomicsRenderingEdgeRadiusParameterProperty);
MITK_REGISTER_ENUM_SUB_SERIALIZER(ConnectomicsRenderingEdgeThresholdParameterProperty);
MITK_REGISTER_ENUM_SUB_SERIALIZER(ConnectomicsRenderingNodeColorParameterProperty);
MITK_REGISTER_ENUM_SUB_SERIALIZER(ConnectomicsRenderingNodeFilteringProperty);
MITK_REGISTER_ENUM_SUB_SERIALIZER(ConnectomicsRenderingNodeRadiusParameterProperty);
MITK_REGISTER_ENUM_SUB_SERIALIZER(ConnectomicsRenderingNodeThresholdParameterProperty);
MITK_REGISTER_ENUM_SUB_SERIALIZER(ConnectomicsRenderingSchemeProperty);

#endif
