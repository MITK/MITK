/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date: 2009-05-28 17:19:30 +0200 (Do, 28 Mai 2009) $
Version:   $Revision $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkNavigationToolStorageDeserializer.h"

mitk::NavigationToolStorageDeserializer::NavigationToolStorageDeserializer()
  {

  }

mitk::NavigationToolStorageDeserializer::~NavigationToolStorageDeserializer()
  {

  }

mitk::NavigationToolStorage::Pointer mitk::NavigationToolStorageDeserializer::Deserialize(std::string filename)
  {
  m_ErrorMessage = "Error, NavigationToolStorageDeserializer is not implemented yet!";
  return NULL;
  }