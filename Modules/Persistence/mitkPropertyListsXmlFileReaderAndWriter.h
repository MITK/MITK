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

#ifndef mitkXmlSceneIO_h_included
#define mitkXmlSceneIO_h_included

#include "mitkDataStorage.h"

class TiXmlElement;

namespace mitk
{
class PropertyListsXmlFileReaderAndWriter;

class PropertyListsXmlFileReaderAndWriter : public itk::Object
{
  public:
    static const char* GetPropertyListIdElementName();

    mitkClassMacro( PropertyListsXmlFileReaderAndWriter, itk::Object );
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    bool WriteLists( const std::string& fileName, const std::map<std::string, mitk::PropertyList::Pointer>& _PropertyLists ) const;
    bool ReadLists( const std::string& fileName, std::map<std::string, mitk::PropertyList::Pointer>& _PropertyLists ) const;

  protected:

    PropertyListsXmlFileReaderAndWriter();
    virtual ~PropertyListsXmlFileReaderAndWriter();

    bool PropertyFromXmlElem(std::string& name, mitk::BaseProperty::Pointer& prop, TiXmlElement* elem) const;
    bool PropertyToXmlElem(const std::string& name, const mitk::BaseProperty* prop, TiXmlElement* elem) const;
};

}

#endif

