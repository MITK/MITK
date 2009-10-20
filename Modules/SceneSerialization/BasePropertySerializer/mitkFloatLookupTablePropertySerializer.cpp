/*=========================================================================
 
Program:   Medical Imaging & LookupTableeraction Toolkit
Module:    $RCSfile: mitkPropertyManager.cpp,v $
Language:  C++
Date:      $Date: 2009-10-08 17:10:12 +0200 (Do, 08. Okt 2009) $
Version:   $Revision: 1.12 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#ifndef mitkFloatLookupTablePropertySerializer_h_included
#define mitkFloatLookupTablePropertySerializer_h_included

#include "mitkBasePropertySerializer.h"

#include "mitkProperties.h"

namespace mitk
{

class SceneSerialization_EXPORT FloatLookupTablePropertySerializer : public BasePropertySerializer
{
  public:
    
    mitkClassMacro( FloatLookupTablePropertySerializer, BasePropertySerializer );
    itkNewMacro(Self);

    virtual TiXmlElement* Serialize()
    {
      const FloatLookupTableProperty* prop = dynamic_cast<const FloatLookupTableProperty*>(m_Property.GetPointer());
      if (prop == NULL)
        return NULL;
      FloatLookupTable lut = prop->GetValue();
      //if (lut.IsNull()) 
      //  return NULL; // really?
      const FloatLookupTable::LookupTableType& map = lut.GetLookupTable();

      TiXmlElement* element = new TiXmlElement("FloatLookupTableTable");
      for (FloatLookupTable::LookupTableType::const_iterator it = map.begin(); it != map.end(); ++it)
        {
          TiXmlElement* tableEntry = new TiXmlElement("LUTValue");
          tableEntry->SetAttribute("id", it->first);
          tableEntry->SetDoubleAttribute("value", static_cast<double>(it->second));
          element->LinkEndChild( tableEntry );
        }
        return element;
    }
  protected:
    FloatLookupTablePropertySerializer() {}
    virtual ~FloatLookupTablePropertySerializer() {}
};
} // namespace
// important to put this into the GLOBAL namespace (because it starts with 'namespace mitk')
MITK_REGISTER_SERIALIZER(FloatLookupTablePropertySerializer);
#endif
