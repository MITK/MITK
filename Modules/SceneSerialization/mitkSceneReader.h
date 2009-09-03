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

#include <tinyxml.h>

#include <itkObjectFactory.h>

#include "mitkDataStorage.h"

namespace mitk
{

class SceneReader : public itk::Object
{
  public:
    
    mitkClassMacro( SceneReader, itk::Object );
      
    virtual bool LoadScene( TiXmlDocument& document, const std::string& workingDirectory, DataStorage* storage ) = 0;
};

}

