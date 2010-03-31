/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
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

#ifndef mitkStringLookupTablePropertySerializer_h_included
#define mitkStringLookupTablePropertySerializer_h_included

#include "mitkBasePropertySerializer.h"

#include "mitkProperties.h"

namespace mitk
{

class SceneSerialization_EXPORT StringLookupTablePropertySerializer : public BasePropertySerializer
{
  public:
    
    mitkClassMacro( StringLookupTablePropertySerializer, BasePropertySerializer );
    itkNewMacro(Self);

    virtual TiXmlElement* Serialize()
    {
      const StringLookupTableProperty* prop = dynamic_cast<const StringLookupTableProperty*>(m_Property.GetPointer());
      if (prop == NULL)
        return NULL;
      StringLookupTable lut = prop->GetValue();
      //if (lut.IsNull()) 
      //  return NULL; // really?
      const StringLookupTable::LookupTableType& map = lut.GetLookupTable();

      TiXmlElement* element = new TiXmlElement("StringLookupTable");
      for (StringLookupTable::LookupTableType::const_iterator it = map.begin(); it != map.end(); ++it)
        {
          TiXmlElement* tableEntry = new TiXmlElement("LUTValue");
          tableEntry->SetAttribute("id", it->first);
          tableEntry->SetAttribute("value", it->second);
          element->LinkEndChild( tableEntry );
        }
        return element;
    }
  protected:
    StringLookupTablePropertySerializer() {}
    virtual ~StringLookupTablePropertySerializer() {}
};
} // namespace
// important to put this into the GLOBAL namespace (because it starts with 'namespace mitk')
MITK_REGISTER_SERIALIZER(StringLookupTablePropertySerializer);
#endif
