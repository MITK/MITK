/*=========================================================================
 
Program:   Medical Imaging & LookupTableeraction Toolkit
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

#ifndef mitkLookupTablePropertyDeserializer_h_included
#define mitkLookupTablePropertyDeserializer_h_included

#include "mitkBasePropertyDeserializer.h"

#include "mitkLookupTableProperty.h"

namespace mitk
{

class SceneSerialization_EXPORT LookupTablePropertyDeserializer : public BasePropertyDeserializer
{
  public:
    
    mitkClassMacro( LookupTablePropertyDeserializer, BasePropertyDeserializer );
    itkNewMacro(Self);

    virtual BaseProperty::Pointer Deserialize(TiXmlElement* element)
    {
      if (!element) return NULL;

#if ( (VTK_MAJOR_VERSION < 5) && (VTK_MINOR_VERSION < 4) )
      typedef float OUR_VTK_FLOAT_TYPE;
      float range[2];
      float  rgba[4];
#else
      typedef double OUR_VTK_FLOAT_TYPE;
      double range[2];
      double  rgba[4];
#endif

      double d;  // bec. of tinyXML's interface that takes a pointer to float or double...

      vtkLookupTable* lut = vtkLookupTable::New();

      int numberOfColors;
      int scale;
      int ramp; // hope the int values don't change betw. vtk versions... 
      if ( element->QueryIntAttribute( "NumberOfColors", &numberOfColors ) == TIXML_SUCCESS )
      {
        lut->SetNumberOfTableValues( numberOfColors );
      }
      if ( element->QueryIntAttribute( "Scale", &scale ) == TIXML_SUCCESS )
      {
        lut->SetScale( scale );
      }
      if ( element->QueryIntAttribute( "Ramp", &ramp ) == TIXML_SUCCESS )
      {
        lut->SetRamp( ramp );
      }

      TiXmlElement* child = element->FirstChildElement("HueRange");
      if (child) 
      {
        if ( child->QueryDoubleAttribute( "min", &d ) != TIXML_SUCCESS ) return NULL; range[0] = static_cast<OUR_VTK_FLOAT_TYPE>(d);
        if ( child->QueryDoubleAttribute( "max", &d ) != TIXML_SUCCESS ) return NULL; range[1] = static_cast<OUR_VTK_FLOAT_TYPE>(d);
        lut->SetHueRange( range );
      }

                    child = element->FirstChildElement("ValueRange");
      if (child)
      {
        if ( child->QueryDoubleAttribute( "min", &d ) != TIXML_SUCCESS ) return NULL; range[0] = static_cast<OUR_VTK_FLOAT_TYPE>(d);
        if ( child->QueryDoubleAttribute( "max", &d ) != TIXML_SUCCESS ) return NULL; range[1] = static_cast<OUR_VTK_FLOAT_TYPE>(d);
        lut->SetValueRange( range );
      }

                    child = element->FirstChildElement("SaturationRange");
      if (child)
      {
        if ( child->QueryDoubleAttribute( "min", &d ) != TIXML_SUCCESS ) return NULL; range[0] = static_cast<OUR_VTK_FLOAT_TYPE>(d);
        if ( child->QueryDoubleAttribute( "max", &d ) != TIXML_SUCCESS ) return NULL; range[1] = static_cast<OUR_VTK_FLOAT_TYPE>(d);
        lut->SetSaturationRange( range );
      }

                    child = element->FirstChildElement("AlphaRange");
      if (child)
      {
        if ( child->QueryDoubleAttribute( "min", &d ) != TIXML_SUCCESS ) return NULL; range[0] = static_cast<OUR_VTK_FLOAT_TYPE>(d);
        if ( child->QueryDoubleAttribute( "max", &d ) != TIXML_SUCCESS ) return NULL; range[1] = static_cast<OUR_VTK_FLOAT_TYPE>(d);
        lut->SetAlphaRange( range );
      }

                    child = element->FirstChildElement("TableRange");
      if (child)
      {
        if ( child->QueryDoubleAttribute( "min", &d ) != TIXML_SUCCESS ) return NULL; range[0] = static_cast<OUR_VTK_FLOAT_TYPE>(d);
        if ( child->QueryDoubleAttribute( "max", &d ) != TIXML_SUCCESS ) return NULL; range[1] = static_cast<OUR_VTK_FLOAT_TYPE>(d);
        lut->SetTableRange( range );
      }
                     
      child = element->FirstChildElement("Table");
      if (child) 
      {
        unsigned int index(0);
        for( TiXmlElement* grandChild = child->FirstChildElement("RgbaColor"); grandChild; grandChild = grandChild->NextSiblingElement("RgbaColor"))
        {
          if ( grandChild->QueryDoubleAttribute("R", &d) != TIXML_SUCCESS ) return NULL; rgba[0] = static_cast<OUR_VTK_FLOAT_TYPE>(d);
          if ( grandChild->QueryDoubleAttribute("G", &d) != TIXML_SUCCESS ) return NULL; rgba[1] = static_cast<OUR_VTK_FLOAT_TYPE>(d);
          if ( grandChild->QueryDoubleAttribute("B", &d) != TIXML_SUCCESS ) return NULL; rgba[2] = static_cast<OUR_VTK_FLOAT_TYPE>(d);
          if ( grandChild->QueryDoubleAttribute("A", &d) != TIXML_SUCCESS ) return NULL; rgba[3] = static_cast<OUR_VTK_FLOAT_TYPE>(d);

          lut->SetTableValue( index, rgba );
          ++index;
        }
      }

      LookupTable::Pointer mitkLut = LookupTable::New();
      mitkLut->SetVtkLookupTable( lut );

      lut->Delete(); 

      return LookupTableProperty::New(mitkLut).GetPointer();
    }

  protected:

    LookupTablePropertyDeserializer() {}
    virtual ~LookupTablePropertyDeserializer() {}
};

} // namespace

// important to put this into the GLOBAL namespace (because it starts with 'namespace mitk')
MITK_REGISTER_SERIALIZER(LookupTablePropertyDeserializer);

#endif

