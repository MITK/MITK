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

#ifndef MITKCOREACTIVATOR_H_
#define MITKCOREACTIVATOR_H_

// File IO
#include <mitkAbstractFileIO.h>
#include <mitkIFileReader.h>
#include <mitkIFileWriter.h>

#include <mitkMimeTypeProvider.h>
#include <mitkPlanePositionManager.h>
#include <mitkPropertyAliases.h>
#include <mitkPropertyDescriptions.h>
#include <mitkPropertyExtensions.h>
#include <mitkPropertyFilters.h>
#include <mitkPropertyPersistence.h>
#include <mitkPropertyRelations.h>

// Micro Services
#include <usModuleActivator.h>
#include <usModuleEvent.h>
#include <usServiceRegistration.h>
#include <usServiceTracker.h>

#include <memory>

/*
 * This is the module activator for the "Mitk" module. It registers core services
 * like ...
 */
class MitkCoreActivator : public us::ModuleActivator
{
public:
  void Load(us::ModuleContext *context) override;
  void Unload(us::ModuleContext *) override;

private:
  void HandleModuleEvent(const us::ModuleEvent moduleEvent);

  void RegisterDefaultMimeTypes();
  void RegisterItkReaderWriter();
  void RegisterVtkReaderWriter();

  void RegisterLegacyWriter();

  // mitk::RenderingManager::Pointer m_RenderingManager;
  std::unique_ptr<mitk::PlanePositionManagerService> m_PlanePositionManager;
  std::unique_ptr<mitk::PropertyAliases> m_PropertyAliases;
  std::unique_ptr<mitk::PropertyDescriptions> m_PropertyDescriptions;
  std::unique_ptr<mitk::PropertyExtensions> m_PropertyExtensions;
  std::unique_ptr<mitk::PropertyFilters> m_PropertyFilters;
  std::unique_ptr<mitk::PropertyPersistence> m_PropertyPersistence;
  std::unique_ptr<mitk::PropertyRelations> m_PropertyRelations;
  std::unique_ptr<mitk::MimeTypeProvider> m_MimeTypeProvider;

  // File IO
  std::vector<mitk::IFileReader *> m_FileReaders;
  std::vector<mitk::IFileWriter *> m_FileWriters;
  std::vector<mitk::AbstractFileIO *> m_FileIOs;
  std::vector<mitk::IFileWriter *> m_LegacyWriters;

  std::vector<mitk::CustomMimeType *> m_DefaultMimeTypes;

  us::ServiceRegistration<mitk::IMimeTypeProvider> m_MimeTypeProviderReg;

  us::ModuleContext *m_Context;
};

#endif // MITKCOREACTIVATOR_H_
