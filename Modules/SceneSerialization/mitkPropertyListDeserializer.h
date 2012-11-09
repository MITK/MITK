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

#ifndef mitkPropertyListDeserializer_h_included
#define mitkPropertyListDeserializer_h_included

#include "SceneSerializationExports.h"

#include "mitkPropertyList.h"

namespace mitk
{

/**
  \brief Deserializes a mitk::PropertyList
*/
class SceneSerialization_EXPORT PropertyListDeserializer : public itk::Object
{
  public:

    mitkClassMacro( PropertyListDeserializer, itk::Object );
    itkNewMacro(Self); // is this needed? should never be instantiated, only subclasses should

    itkSetStringMacro(Filename);
    itkGetStringMacro(Filename);

    /**
      \brief Reads a propertylist from file
      \return success of deserialization
      */
    virtual bool Deserialize();

    virtual PropertyList::Pointer GetOutput();

  protected:

    PropertyListDeserializer();
    virtual ~PropertyListDeserializer();

    std::string m_Filename;
    PropertyList::Pointer m_PropertyList;
};

} // namespace

#endif

