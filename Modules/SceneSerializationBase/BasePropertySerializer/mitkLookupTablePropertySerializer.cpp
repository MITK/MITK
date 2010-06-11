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

#ifndef mitkLookupTablePropertySerializer_h_included
#define mitkLookupTablePropertySerializer_h_included

#include "mitkBasePropertySerializer.h"

#include "mitkLookupTableProperty.h"

#include "SceneSerializationBaseExports.h"

namespace mitk
{

class SceneSerializationBase_EXPORT LookupTablePropertySerializer : public BasePropertySerializer
{
  public:
    
    mitkClassMacro( LookupTablePropertySerializer, BasePropertySerializer );
    itkNewMacro(Self);

    virtual TiXmlElement* Serialize()
    {
      if (const LookupTableProperty* prop = dynamic_cast<const LookupTableProperty*>(m_Property.GetPointer()))
      {
        LookupTable::Pointer mitkLut = const_cast<LookupTableProperty*>(prop)->GetLookupTable();
        if (mitkLut.IsNull()) return NULL; // really?

        vtkLookupTable* lut = mitkLut->GetVtkLookupTable();
        if (!lut) return NULL;

        TiXmlElement* element = new TiXmlElement("LookupTable");

#if ( (VTK_MAJOR_VERSION < 5) && (VTK_MINOR_VERSION < 4) )
        float*  range;
        float*  rgba;
#else
        double*  range;
        double*  rgba;
#endif

        element->SetAttribute("NumberOfColors", lut->GetNumberOfTableValues());
        element->SetAttribute("Scale", lut->GetScale());
        element->SetAttribute("Ramp", lut->GetRamp());

        range = lut->GetHueRange();
        TiXmlElement* child = new TiXmlElement("HueRange");
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
          TiXmlElement* grandChildNinife = new TiXmlElement("RgbaColor");
          rgba = lut->GetTableValue(index);
          grandChildNinife->SetDoubleAttribute("R", rgba[0]);
          grandChildNinife->SetDoubleAttribute("G", rgba[1]);
          grandChildNinife->SetDoubleAttribute("B", rgba[2]);
          grandChildNinife->SetDoubleAttribute("A", rgba[3]);
          child->LinkEndChild( grandChildNinife );
        }
        return element;
      }
      else return NULL;
    }

  protected:

    LookupTablePropertySerializer() {}
    virtual ~LookupTablePropertySerializer() {}
};

} // namespace

// important to put this into the GLOBAL namespace (because it starts with 'namespace mitk')
MITK_REGISTER_SERIALIZER(LookupTablePropertySerializer);

#endif

