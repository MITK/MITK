/*=========================================================================
 
Program:   Medical Imaging & Enumerationeraction Toolkit
Module:    $RCSfile: mitkPropertyManager.cpp,v $
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

#ifndef mitkEnumerationSubclassesSerializer_h_included
#define mitkEnumerationSubclassesSerializer_h_included

#include "mitkEnumerationPropertySerializer.h"

#define MITK_REGISTER_ENUM_SUB_SERIALIZER(classname) \
 \
namespace mitk \
{ \
 \
class SceneSerialization_EXPORT classname ## Serializer : public EnumerationPropertySerializer \
{ \
  public: \
     \
    mitkClassMacro( classname ## Serializer, EnumerationPropertySerializer ); \
    itkNewMacro(Self); \
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

MITK_REGISTER_ENUM_SUB_SERIALIZER(GridRepresentationProperty);
MITK_REGISTER_ENUM_SUB_SERIALIZER(GridVolumeMapperProperty);
MITK_REGISTER_ENUM_SUB_SERIALIZER(OrganTypeProperty);
MITK_REGISTER_ENUM_SUB_SERIALIZER(PlaneDecorationProperty);
MITK_REGISTER_ENUM_SUB_SERIALIZER(VtkInterpolationProperty);
MITK_REGISTER_ENUM_SUB_SERIALIZER(VtkRepresentationProperty);
MITK_REGISTER_ENUM_SUB_SERIALIZER(VtkResliceInterpolationProperty);
MITK_REGISTER_ENUM_SUB_SERIALIZER(VtkScalarModeProperty);
MITK_REGISTER_ENUM_SUB_SERIALIZER(VtkVolumeRenderingProperty);

#endif

