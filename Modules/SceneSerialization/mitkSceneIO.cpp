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

#include "mitkSceneIO.h"

mitk::DataStorage::Pointer mitk::SceneIO::LoadScene( const std::string& filename, 
                                                     DataStorage* storage, 
                                                     bool clearStorageFirst )
{
  return NULL;
}
    
bool mitk::SceneIO::SaveScene( DataStorage* storage,
                               const std::string& filename,
                               NodePredicateBase* predicate )
{
  if (!storage) return false;

  if (predicate)
  {
    LOG_INFO << "Storing scene with " << storage->GetSubset(predicate)->size() << " objects to " << filename;
  }
  else
  {
    LOG_INFO << "Storing scene with " << storage->GetAll()->size() << " objects to " << filename;
  }

  return true;
}

