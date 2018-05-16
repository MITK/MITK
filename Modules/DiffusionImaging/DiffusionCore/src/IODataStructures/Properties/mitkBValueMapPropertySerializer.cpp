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

#ifndef mitkBValueMapPropertySerializer_h_included
#define mitkBValueMapPropertySerializer_h_included

#include "mitkBasePropertySerializer.h"

#include "mitkBValueMapProperty.h"

#include <MitkDiffusionCoreExports.h>

namespace mitk
{

class MITKDIFFUSIONCORE_EXPORT BValueMapPropertySerializer : public BasePropertySerializer
{

protected:


  void split(const std::string &s, char delim, std::vector<unsigned int> &elems) {
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
      elems.push_back(std::atoi(item.c_str()));
    }
  }

  std::vector<unsigned int> split(const std::string &s, char delim) {
    std::vector<unsigned int> elems;
    split(s, delim, elems);
    return elems;
  }

public:

  mitkClassMacro( BValueMapPropertySerializer, BasePropertySerializer )
  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

  TiXmlElement* Serialize() override
  {
    if (const BValueMapProperty* prop = dynamic_cast<const BValueMapProperty*>(m_Property.GetPointer()))
    {

      BValueMapProperty::BValueMap map = prop->GetBValueMap();

      if(map.empty()) return nullptr;

      BValueMapProperty::BValueMap::const_iterator it = map.begin();
      BValueMapProperty::BValueMap::const_iterator end = map.end();

      auto  element = new TiXmlElement("bvaluemap");



      while (it != end) {
        auto  child = new TiXmlElement("entry");
        {
          std::stringstream ss;
          ss << it->first;
          child->SetAttribute("key", ss.str());
        }

        {
          std::stringstream ss;
          for(unsigned int i = 0 ; i < it->second.size(); i++)
          {

            ss << it->second[i] << ",";
          }
          child->SetAttribute("value", ss.str());
        }
        element->InsertEndChild(*child);
        ++it;
      }

      return element;
    }
    else return nullptr;
  }


  BaseProperty::Pointer Deserialize(TiXmlElement* element) override
  {
    if (!element) return nullptr;

    BValueMapProperty::BValueMap map;

    TiXmlElement* entry = element->FirstChildElement( "entry" )->ToElement();
    while(entry != nullptr){

      std::string key, value;
      entry->QueryStringAttribute("key",&key);
      entry->QueryStringAttribute("value",&value);

      std::vector<unsigned int> indices = split(value.c_str(), ',');

      map[std::atoi(key.c_str())] = indices;
      entry = entry->NextSiblingElement( "entry" );
    }

    return BValueMapProperty::New(map).GetPointer();
  }

protected:

  BValueMapPropertySerializer(){}
  ~BValueMapPropertySerializer() override {}
};

} // namespace

// important to put this into the GLOBAL namespace (because it starts with 'namespace mitk')
MITK_REGISTER_SERIALIZER(BValueMapPropertySerializer)

#endif
