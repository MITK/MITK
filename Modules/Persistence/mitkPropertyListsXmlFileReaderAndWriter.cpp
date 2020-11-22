/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkPropertyListsXmlFileReaderAndWriter.h"
#include "mitkProperties.h"
#include "mitkStandaloneDataStorage.h"
#include <itksys/SystemTools.hxx>
#include <tinyxml2.h>

namespace
{
  std::string ReadStringAttribute(const tinyxml2::XMLElement* elem, const char *name)
  {
    const char* attrib = elem->Attribute(name);

    return attrib != nullptr
      ? attrib
      : "";
  }
}

namespace mitk
{
  bool PropertyListsXmlFileReaderAndWriter::PropertyFromXmlElem(std::string &name,
                                                                mitk::BaseProperty::Pointer &prop,
                                                                const tinyxml2::XMLElement *elem) const
  {
    if (nullptr == elem)
      return false;

    std::string type = ReadStringAttribute(elem, "type");

    if (type.empty())
    {
      MITK_WARN << "type attribute not found in property";
      return false;
    }
      
    name = ReadStringAttribute(elem, "name");

    if (name.empty())
    {
      MITK_WARN << "name attribute not found in property";
      return false;
    }

    tinyxml2::XMLError err = tinyxml2::XML_SUCCESS;

    if (type == "BoolProperty")
    {
      int val = 0;
      err = elem->QueryIntAttribute("value", &val);
      if (tinyxml2::XML_SUCCESS == err)
      {
        prop = mitk::BoolProperty::New(val == 1 ? true : false);
      }
    }
    else if (type == "StringProperty")
    {
      const auto* attrib = elem->FindAttribute("value");
      
      if (nullptr == attrib)
      {
        err = tinyxml2::XML_NO_ATTRIBUTE;
      }
      else
      {
        auto val = ReadStringAttribute(elem, "value");
        prop = mitk::StringProperty::New(val);
      }
    }
    else if (type == "IntProperty")
    {
      int val = 0;
      err = elem->QueryIntAttribute("value", &val);
      if (tinyxml2::XML_SUCCESS == err)
      {
        prop = mitk::IntProperty::New(val);
      }
    }
    else if (type == "DoubleProperty")
    {
      double val = 0;
      err = elem->QueryDoubleAttribute("value", &val);
      if (tinyxml2::XML_SUCCESS == err)
      {
        prop = mitk::DoubleProperty::New(val);
      }
    }
    else if (type == "FloatProperty")
    {
      float val = 0;
      err = elem->QueryFloatAttribute("value", &val);
      if (tinyxml2::XML_SUCCESS == err)
      {
        prop = mitk::FloatProperty::New(val);
      }
    }
    else
    {
      err = tinyxml2::XML_WRONG_ATTRIBUTE_TYPE;
      MITK_WARN << "type attribute unknown. Only BoolProperty, StringProperty, IntProperty, DoubleProperty or FloatProperty allowed.";
    }

    return tinyxml2::XML_SUCCESS == err;
  }

  bool PropertyListsXmlFileReaderAndWriter::PropertyToXmlElem(const std::string &name,
                                                              const mitk::BaseProperty *prop,
                                                              tinyxml2::XMLElement *elem) const
  {
    if (nullptr == prop || nullptr == elem)
      return false;

    const mitk::IntProperty *intProp = nullptr;
    const mitk::FloatProperty *floatProp = nullptr;
    const mitk::DoubleProperty *doubleProp = nullptr;
    const mitk::BoolProperty *boolProp = nullptr;
    const mitk::StringProperty *stringProp = nullptr;
    bool writeOp = true;

    if (nullptr != (boolProp = dynamic_cast<const BoolProperty *>(prop)))
    {
      elem->SetAttribute(GetPropertyListIdElementName(), name.c_str());
      elem->SetAttribute("value", boolProp->GetValue() ? 1 : 0);
      elem->SetAttribute("type", "BoolProperty");
    }
    else if (nullptr != (stringProp = dynamic_cast<const StringProperty *>(prop)))
    {
      elem->SetAttribute(GetPropertyListIdElementName(), name.c_str());
      elem->SetAttribute("value", stringProp->GetValue());
      elem->SetAttribute("type", "StringProperty");
    }
    else if (nullptr != (intProp = dynamic_cast<const IntProperty *>(prop)))
    {
      elem->SetAttribute(GetPropertyListIdElementName(), name.c_str());
      elem->SetAttribute("value", intProp->GetValue());
      elem->SetAttribute("type", "IntProperty");
    }
    else if (nullptr != (doubleProp = dynamic_cast<const DoubleProperty *>(prop)))
    {
      elem->SetAttribute(GetPropertyListIdElementName(), name.c_str());
      elem->SetAttribute("value", doubleProp->GetValue());
      elem->SetAttribute("type", "DoubleProperty");
    }
    else if (nullptr != (floatProp = dynamic_cast<const FloatProperty *>(prop)))
    {
      elem->SetAttribute(GetPropertyListIdElementName(), name.c_str());
      elem->SetAttribute("value", floatProp->GetValue());
      elem->SetAttribute("type", "FloatProperty");
    }
    else
    {
      MITK_WARN("PropertyListImportFromXmlFile") << "Base property " << name << " is unknown";
      writeOp = false;
    }
    return writeOp;
  }

  bool PropertyListsXmlFileReaderAndWriter::WriteLists(const std::string &fileName,
                                                       const std::map<std::string, mitk::PropertyList::Pointer> &_PropertyLists) const
  {
    tinyxml2::XMLDocument doc;
    doc.InsertEndChild(doc.NewDeclaration());

    // create root
    auto *propertyListsElem = doc.NewElement("PropertyLists");

    bool allPropsConverted = true;
    auto it = _PropertyLists.begin();
    while (it != _PropertyLists.end())
    {
      const std::string &id = (*it).first;
      const PropertyList *propList = (*it).second;
      auto *propertyListElem = doc.NewElement("PropertyList");
      propertyListElem->SetAttribute(GetPropertyListIdElementName(), id.c_str());

      const std::map<std::string, BaseProperty::Pointer> *propMap = propList->GetMap();
      auto propMapIt = propMap->begin();
      while (propMapIt != propMap->end())
      {
        const std::string &propName = (*propMapIt).first;
        const BaseProperty *prop = (*propMapIt).second;
        auto *propertyElem = doc.NewElement("Property");

        if (!this->PropertyToXmlElem(propName, prop, propertyElem))
          allPropsConverted = false;

        propertyListElem->InsertEndChild(propertyElem);
        ++propMapIt;
      }

      propertyListsElem->InsertEndChild(propertyListElem);
      ++it;
    }

    doc.InsertEndChild(propertyListsElem);

    return (allPropsConverted && tinyxml2::XML_SUCCESS == doc.SaveFile(fileName.c_str()));
  }

  bool PropertyListsXmlFileReaderAndWriter::ReadLists(const std::string &fileName,
                                                      std::map<std::string, mitk::PropertyList::Pointer> &_PropertyLists) const
  {
    // reread
    tinyxml2::XMLDocument doc;
    doc.LoadFile(fileName.c_str());

    tinyxml2::XMLHandle docHandle(&doc);
    auto *elem = docHandle.FirstChildElement("PropertyLists").FirstChildElement("PropertyList").ToElement();

    if (nullptr == elem)
    {
      MITK_WARN("PropertyListFromXml") << "Cannot find a PropertyList element (inside a PropertyLists element)";
      return false;
    }

    bool opRead = false;
    while (nullptr != elem)
    {
      std::string propListId = ReadStringAttribute(elem, GetPropertyListIdElementName());
      if (propListId.empty())
        break;

      opRead = true;

      auto propList = mitk::PropertyList::New();

      auto *propElem = elem->FirstChildElement("Property");

      while (nullptr != propElem)
      {
        std::string name;
        mitk::BaseProperty::Pointer prop;
        opRead = this->PropertyFromXmlElem(name, prop, propElem);
        if (!opRead)
          break;
        propList->SetProperty(name, prop);
        propElem = propElem->NextSiblingElement("Property");
      }

      if (!opRead)
        break;
      _PropertyLists[propListId] = propList;
      elem = elem->NextSiblingElement("PropertyList");
    }

    return opRead;
  }

  PropertyListsXmlFileReaderAndWriter::PropertyListsXmlFileReaderAndWriter() {}
  PropertyListsXmlFileReaderAndWriter::~PropertyListsXmlFileReaderAndWriter() {}
  const char *PropertyListsXmlFileReaderAndWriter::GetPropertyListIdElementName() { return "name"; }
}
