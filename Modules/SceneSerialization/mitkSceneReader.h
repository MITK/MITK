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

#include "SceneSerializationExports.h"

#include <tinyxml.h>

#include <itkObjectFactory.h>

#include "mitkDataStorage.h"

namespace mitk
{

class SceneSerialization_EXPORT SceneReader : public itk::Object
{
  public:
    
    mitkClassMacro( SceneReader, itk::Object );
    itkNewMacro( Self );
      
    virtual bool LoadScene( TiXmlDocument& document, const std::string& workingDirectory, DataStorage* storage );
};

}

