/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: 1.12 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#ifndef mitkPropertyListSerializer_h_included
#define mitkPropertyListSerializer_h_included

#include "SceneSerializationExports.h"

#include "mitkPropertyList.h"

#include <itkObjectFactoryBase.h>

class TiXmlElement;

namespace mitk
{

/**
  \brief Serializes a mitk::PropertyList
*/
class SceneSerialization_EXPORT PropertyListSerializer : public itk::Object
{
  public:
    
    mitkClassMacro( PropertyListSerializer, itk::Object );
    itkNewMacro(Self); // is this needed? should never be instantiated, only subclasses should

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

    PropertyList* GetFailedProperties();

  protected:

    PropertyListSerializer();
    virtual ~PropertyListSerializer();

    TiXmlElement* SerializeOneProperty( const std::string& key, const BaseProperty* property );
    
    std::string m_FilenameHint;
    std::string m_WorkingDirectory;
    PropertyList::Pointer m_PropertyList;
    
    PropertyList::Pointer m_FailedProperties;
};

} // namespace

#endif

