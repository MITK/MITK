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

#ifndef mitkLookupTablePropertySerializer_h_included
#define mitkLookupTablePropertySerializer_h_included

#include "mitkBasePropertySerializer.h"

#include "mitkLookupTableProperty.h"

namespace mitk
{

class LookupTablePropertySerializer : public BasePropertySerializer
{
  public:

    mitkClassMacro( LookupTablePropertySerializer, BasePropertySerializer );
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    virtual TiXmlElement* Serialize() override
    {
      if (const LookupTableProperty* prop = dynamic_cast<const LookupTableProperty*>(m_Property.GetPointer()))
      {
        LookupTable::Pointer mitkLut = const_cast<LookupTableProperty*>(prop)->GetLookupTable();
        if (mitkLut.IsNull()) return nullptr; // really?

        vtkLookupTable* lut = mitkLut->GetVtkLookupTable();
        if (!lut) return nullptr;

        auto  element = new TiXmlElement("LookupTable");

        double*  range;
        double*  rgba;

        element->SetAttribute("NumberOfColors", lut->GetNumberOfTableValues());
        element->SetAttribute("Scale", lut->GetScale());
        element->SetAttribute("Ramp", lut->GetRamp());

        range = lut->GetHueRange();
        auto  child = new TiXmlElement("HueRange");
        element->LinkEndChild( child );
          child->SetDoubleAttribute("min", range[0]);
          child->SetDoubleAttribute("max", range[1]);

        range = lut->GetValueRange();
                      child = new TiXmlElement("ValueRange");
        element->LinkEndChild( child );
          child->SetDoubleAttribute("min", range[0]);
          child->SetDoubleAttribute("max", range[1]);

        range = lut->GetSaturationRange();
                      child = new TiXmlElement("SaturationRange");
        element->LinkEndChild( child );
          child->SetDoubleAttribute("min", range[0]);
          child->SetDoubleAttribute("max", range[1]);

        range = lut->GetAlphaRange();
                      child = new TiXmlElement("AlphaRange");
        element->LinkEndChild( child );
          child->SetDoubleAttribute("min", range[0]);
          child->SetDoubleAttribute("max", range[1]);

        range = lut->GetTableRange();
                      child = new TiXmlElement("TableRange");
        element->LinkEndChild( child );
          child->SetDoubleAttribute("min", range[0]);
          child->SetDoubleAttribute("max", range[1]);

        child = new TiXmlElement("Table");
        element->LinkEndChild( child );
        for ( int index = 0; index < lut->GetNumberOfTableValues(); ++index)
        {
          auto  grandChildNinife = new TiXmlElement("RgbaColor");
          rgba = lut->GetTableValue(index);
          grandChildNinife->SetDoubleAttribute("R", rgba[0]);
          grandChildNinife->SetDoubleAttribute("G", rgba[1]);
          grandChildNinife->SetDoubleAttribute("B", rgba[2]);
          grandChildNinife->SetDoubleAttribute("A", rgba[3]);
          child->LinkEndChild( grandChildNinife );
        }
        return element;
      }
      else return nullptr;
    }

    virtual BaseProperty::Pointer Deserialize(TiXmlElement* element) override
    {
      if (!element) return nullptr;

      typedef double OUR_VTK_FLOAT_TYPE;
      double range[2];
      double  rgba[4];

      double d;  // bec. of tinyXML's interface that takes a pointer to float or double...

      vtkSmartPointer<vtkLookupTable> lut = vtkSmartPointer<vtkLookupTable>::New();

      int numberOfColors;
      int scale;
      int ramp; // hope the int values don't change betw. vtk versions...
      if ( element->QueryIntAttribute( "NumberOfColors", &numberOfColors ) == TIXML_SUCCESS )
      {
        lut->SetNumberOfTableValues( numberOfColors );
      }
      else
        return nullptr;
      if ( element->QueryIntAttribute( "Scale", &scale ) == TIXML_SUCCESS )
      {
        lut->SetScale( scale );
      }
      else
        return nullptr;
      if ( element->QueryIntAttribute( "Ramp", &ramp ) == TIXML_SUCCESS )
      {
        lut->SetRamp( ramp );
      }
      else
        return nullptr;

      TiXmlElement* child = element->FirstChildElement("HueRange");
      if (child)
      {
        if ( child->QueryDoubleAttribute( "min", &d ) != TIXML_SUCCESS )
          return nullptr;
        range[0] = static_cast<OUR_VTK_FLOAT_TYPE>(d);
        if ( child->QueryDoubleAttribute( "max", &d ) != TIXML_SUCCESS )
          return nullptr;
        range[1] = static_cast<OUR_VTK_FLOAT_TYPE>(d);
        lut->SetHueRange( range );
      }

                    child = element->FirstChildElement("ValueRange");
      if (child)
      {
        if ( child->QueryDoubleAttribute( "min", &d ) != TIXML_SUCCESS ) return nullptr; range[0] = static_cast<OUR_VTK_FLOAT_TYPE>(d);
        if ( child->QueryDoubleAttribute( "max", &d ) != TIXML_SUCCESS ) return nullptr; range[1] = static_cast<OUR_VTK_FLOAT_TYPE>(d);
        lut->SetValueRange( range );
      }

                    child = element->FirstChildElement("SaturationRange");
      if (child)
      {
        if ( child->QueryDoubleAttribute( "min", &d ) != TIXML_SUCCESS ) return nullptr; range[0] = static_cast<OUR_VTK_FLOAT_TYPE>(d);
        if ( child->QueryDoubleAttribute( "max", &d ) != TIXML_SUCCESS ) return nullptr; range[1] = static_cast<OUR_VTK_FLOAT_TYPE>(d);
        lut->SetSaturationRange( range );
      }

                    child = element->FirstChildElement("AlphaRange");
      if (child)
      {
        if ( child->QueryDoubleAttribute( "min", &d ) != TIXML_SUCCESS ) return nullptr; range[0] = static_cast<OUR_VTK_FLOAT_TYPE>(d);
        if ( child->QueryDoubleAttribute( "max", &d ) != TIXML_SUCCESS ) return nullptr; range[1] = static_cast<OUR_VTK_FLOAT_TYPE>(d);
        lut->SetAlphaRange( range );
      }

                    child = element->FirstChildElement("TableRange");
      if (child)
      {
        if ( child->QueryDoubleAttribute( "min", &d ) != TIXML_SUCCESS ) return nullptr; range[0] = static_cast<OUR_VTK_FLOAT_TYPE>(d);
        if ( child->QueryDoubleAttribute( "max", &d ) != TIXML_SUCCESS ) return nullptr; range[1] = static_cast<OUR_VTK_FLOAT_TYPE>(d);
        lut->SetTableRange( range );
      }

      child = element->FirstChildElement("Table");
      if (child)
      {
        unsigned int index(0);
        for( TiXmlElement* grandChild = child->FirstChildElement("RgbaColor"); grandChild; grandChild = grandChild->NextSiblingElement("RgbaColor"))
        {
          if ( grandChild->QueryDoubleAttribute("R", &d) != TIXML_SUCCESS ) return nullptr; rgba[0] = static_cast<OUR_VTK_FLOAT_TYPE>(d);
          if ( grandChild->QueryDoubleAttribute("G", &d) != TIXML_SUCCESS ) return nullptr; rgba[1] = static_cast<OUR_VTK_FLOAT_TYPE>(d);
          if ( grandChild->QueryDoubleAttribute("B", &d) != TIXML_SUCCESS ) return nullptr; rgba[2] = static_cast<OUR_VTK_FLOAT_TYPE>(d);
          if ( grandChild->QueryDoubleAttribute("A", &d) != TIXML_SUCCESS ) return nullptr; rgba[3] = static_cast<OUR_VTK_FLOAT_TYPE>(d);

          lut->SetTableValue( index, rgba );
          ++index;
        }
      }

      LookupTable::Pointer mitkLut = LookupTable::New();
      mitkLut->SetVtkLookupTable( lut );

      return LookupTableProperty::New(mitkLut).GetPointer();
    }

  protected:

    LookupTablePropertySerializer() {}
    virtual ~LookupTablePropertySerializer() {}
};

} // namespace

// important to put this into the GLOBAL namespace (because it starts with 'namespace mitk')
MITK_REGISTER_SERIALIZER(LookupTablePropertySerializer);

#endif

