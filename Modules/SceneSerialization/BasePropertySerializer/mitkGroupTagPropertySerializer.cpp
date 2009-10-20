/*=========================================================================
 
Program:   Medical Imaging & GroupTageraction Toolkit
Module:    $RCSfile: mitkPropertyManager.cpp,v $
Language:  C++
Date:      $Date: 2009-07-07 15:56:37 +0200 (Di, 07. Jul 2009) $
Version:   $Revision: 1.12 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#ifndef mitkGroupTagPropertySerializer_h_included
#define mitkGroupTagPropertySerializer_h_included

#include "mitkBasePropertySerializer.h"

#include "mitkGroupTagProperty.h"

namespace mitk
{

class SceneSerialization_EXPORT GroupTagPropertySerializer : public BasePropertySerializer
{
  public:
    
    mitkClassMacro( GroupTagPropertySerializer, BasePropertySerializer );
    itkNewMacro(Self);

    virtual TiXmlElement* Serialize()
    {
      if (dynamic_cast<const GroupTagProperty*>(m_Property.GetPointer()) != NULL)
      {
        TiXmlElement* element = new TiXmlElement("GroupTag");
        return element;
      }
      else return NULL;
    }

  protected:

    GroupTagPropertySerializer() {}
    virtual ~GroupTagPropertySerializer() {}
};

} // namespace

// important to put this into the GLOBAL namespace (because it starts with 'namespace mitk')
MITK_REGISTER_SERIALIZER(GroupTagPropertySerializer);

#endif

