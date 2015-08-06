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

#ifndef mitkPropertyPersistenceInfo_h
#define mitkPropertyPersistenceInfo_h

#include <mitkCommon.h>
#include <itkObjectFactory.h>
#include <MitkCoreExports.h>

namespace mitk
{
  /** \brief Property persistence info.
    */
  class MITKCORE_EXPORT PropertyPersistenceInfo : public itk::LightObject
  {
  public:
    mitkClassMacroItkParent(PropertyPersistenceInfo, itk::LightObject);
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)
    mitkNewMacro1Param(Self, const std::string&);

    std::string GetKey() const;
    void SetKey(const std::string& key);

  protected:
    /** \brief Constructor.
      *
      * \param[in] key Key used to save the property value as key value pair. If empty, the property name is used.
      */
    PropertyPersistenceInfo(const std::string& key = "");

    virtual ~PropertyPersistenceInfo();

  private:
    PropertyPersistenceInfo(const Self& other);
    Self& operator=(const Self& other);

    struct Impl;
    Impl* m_Impl;
  };
}

#endif
