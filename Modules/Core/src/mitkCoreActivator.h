/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkCoreActivator_h
#define mitkCoreActivator_h

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
#include <mitkPreferencesService.h>

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
  std::unique_ptr<mitk::PreferencesService> m_PreferencesService;

  // File IO
  std::vector<mitk::IFileReader *> m_FileReaders;
  std::vector<mitk::IFileWriter *> m_FileWriters;
  std::vector<mitk::AbstractFileIO *> m_FileIOs;
  std::vector<mitk::IFileWriter *> m_LegacyWriters;

  std::vector<mitk::CustomMimeType *> m_DefaultMimeTypes;

  us::ServiceRegistration<mitk::IMimeTypeProvider> m_MimeTypeProviderReg;

  us::ModuleContext *m_Context;
};

#endif
