/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile: mitkPropertyManager.cpp,v $
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

#ifndef mitkPropertyListDeserializerV1_h_included
#define mitkPropertyListDeserializerV1_h_included

#include "SceneSerializationExports.h"
#include <itkObjectFactory.h>

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

