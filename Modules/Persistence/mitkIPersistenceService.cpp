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

#include "mitkIPersistenceService.h"
#include "Poco\Path.h"


std::string mitk::IPersistenceService::m_DefaultPersistenceFile(CreateDefaultFileName());

mitk::IPersistenceService::~IPersistenceService()
{
}

void mitk::IPersistenceService::SetDefaultPersistenceFile( const std::string& defaultPersistenceFile )
{
    m_DefaultPersistenceFile = defaultPersistenceFile;

}

std::string mitk::IPersistenceService::GetDefaultPersistenceFile()
{
    return m_DefaultPersistenceFile;
}

std::string mitk::IPersistenceService::CreateDefaultFileName()
{
    std::string homeDir = Poco::Path::home();
    std::string file = /*homeDir +*/ "PersistentData.mitk";
    return file;
}
