/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPropertyListSerializer_h_included
#define mitkPropertyListSerializer_h_included

#include <MitkSceneSerializationBaseExports.h>

#include "mitkPropertyList.h"

#include <itkObjectFactoryBase.h>

namespace tinyxml2
{
  class XMLDocument;
  class XMLElement;
}

namespace mitk
{
  /**
    \brief Serializes a mitk::PropertyList
  */
  class MITKSCENESERIALIZATIONBASE_EXPORT PropertyListSerializer : public itk::Object
  {
  public:
    mitkClassMacroItkParent(PropertyListSerializer, itk::Object);
    itkFactorylessNewMacro(Self) // is this needed? should never be instantiated, only subclasses should
      itkCloneMacro(Self);
      itkSetStringMacro(FilenameHint);
    itkGetStringMacro(FilenameHint);

    itkSetStringMacro(WorkingDirectory);
    itkGetStringMacro(WorkingDirectory);

    itkSetObjectMacro(PropertyList, PropertyList);

    /**
      \brief Serializes given PropertyList object.
      \return the filename of the newly created file.
      */
    virtual std::string Serialize();

    PropertyList *GetFailedProperties();

  protected:
    PropertyListSerializer();
    ~PropertyListSerializer() override;

    tinyxml2::XMLElement *SerializeOneProperty(tinyxml2::XMLDocument &doc, const std::string &key, const BaseProperty *property);

    std::string m_FilenameHint;
    std::string m_WorkingDirectory;
    PropertyList::Pointer m_PropertyList;

    PropertyList::Pointer m_FailedProperties;
  };

} // namespace

#endif
