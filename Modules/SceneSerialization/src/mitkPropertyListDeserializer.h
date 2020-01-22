/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPropertyListDeserializer_h_included
#define mitkPropertyListDeserializer_h_included

#include "mitkPropertyList.h"

namespace mitk
{
  /**
    \brief Deserializes a mitk::PropertyList
  */
  class PropertyListDeserializer : public itk::Object
  {
  public:
    mitkClassMacroItkParent(PropertyListDeserializer, itk::Object);
    itkFactorylessNewMacro(Self) // is this needed? should never be instantiated, only subclasses should
      itkCloneMacro(Self);
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
    ~PropertyListDeserializer() override;

    std::string m_Filename;
    PropertyList::Pointer m_PropertyList;
  };

} // namespace

#endif
