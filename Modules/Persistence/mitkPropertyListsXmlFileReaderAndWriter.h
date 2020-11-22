/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkXmlSceneIO_h_included
#define mitkXmlSceneIO_h_included

#include "mitkDataStorage.h"

namespace tinyxml2
{
  class XMLElement;
}

namespace mitk
{
  class PropertyListsXmlFileReaderAndWriter;

  class PropertyListsXmlFileReaderAndWriter : public itk::Object
  {
  public:
    static const char *GetPropertyListIdElementName();

    mitkClassMacroItkParent(PropertyListsXmlFileReaderAndWriter, itk::Object);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

      bool WriteLists(const std::string &fileName,
                      const std::map<std::string, mitk::PropertyList::Pointer> &_PropertyLists) const;
    bool ReadLists(const std::string &fileName,
                   std::map<std::string, mitk::PropertyList::Pointer> &_PropertyLists) const;

  protected:
    PropertyListsXmlFileReaderAndWriter();
    ~PropertyListsXmlFileReaderAndWriter() override;

    bool PropertyFromXmlElem(std::string &name, mitk::BaseProperty::Pointer &prop, const tinyxml2::XMLElement *elem) const;
    bool PropertyToXmlElem(const std::string &name, const mitk::BaseProperty *prop, tinyxml2::XMLElement *elem) const;
  };
}

#endif
