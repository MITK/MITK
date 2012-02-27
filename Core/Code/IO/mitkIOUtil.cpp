/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include "mitkIOUtil.h"

#include <mitkGetModuleContext.h>
#include <mitkModuleContext.h>
#include <mitkStandaloneDataStorage.h>
#include <mitkIDataNodeReader.h>
#include <mitkProgressBar.h>

namespace mitk {

int IOUtil::LoadFiles(const std::vector<std::string> &fileNames, DataStorage &ds)
{
  // Get the set of registered mitk::IDataNodeReader services
  ModuleContext* context = GetModuleContext();
  const std::list<ServiceReference> refs = context->GetServiceReferences<IDataNodeReader>();
  std::vector<IDataNodeReader*> services;
  services.reserve(refs.size());
  for (std::list<ServiceReference>::const_iterator i = refs.begin();
       i != refs.end(); ++i)
  {
    IDataNodeReader* s = context->GetService<IDataNodeReader>(*i);
    if (s != 0)
    {
      services.push_back(s);
    }
  }

  mitk::ProgressBar::GetInstance()->AddStepsToDo(2*fileNames.size());

  // Iterate over all file names and use the IDataNodeReader services
  // to load them.
  int nodesRead = 0;
  for (std::vector<std::string>::const_iterator i = fileNames.begin();
       i != fileNames.end(); ++i)
  {
    for (std::vector<IDataNodeReader*>::const_iterator readerIt = services.begin();
         readerIt != services.end(); ++readerIt)
    {
      try
      {
        int n = (*readerIt)->Read(*i, ds);
        nodesRead += n;
        if (n > 0) break;
      }
      catch (const std::exception& e)
      {
        MITK_WARN << e.what();
      }
    }
    mitk::ProgressBar::GetInstance()->Progress(2);
  }

  for (std::list<ServiceReference>::const_iterator i = refs.begin();
       i != refs.end(); ++i)
  {
    context->UngetService(*i);
  }

  return nodesRead;
}

DataStorage::Pointer IOUtil::LoadFiles(const std::vector<std::string>& fileNames)
{
  mitk::StandaloneDataStorage::Pointer ds = mitk::StandaloneDataStorage::New();
  LoadFiles(fileNames, *ds);
  return ds.GetPointer();
}

}
