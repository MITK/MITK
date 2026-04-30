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

#include "mitkMimeTypeProvider.h"
#include "mitkNodeSelectionService.h"
#include <mitkPlanePositionManager.h>
#include <mitkPropertyAliases.h>
#include <mitkPropertyDescriptions.h>
#include <mitkPropertyDeserialization.h>
#include <mitkPropertyExtensions.h>
#include <mitkPropertyFilters.h>
#include <mitkPropertyPersistence.h>
#include <mitkPropertyRelations.h>
#include "mitkPreferencesService.h"

// Micro Services
#include <usModuleActivator.h>
#include <usModuleEvent.h>
#include <usServiceRegistration.h>
#include <usServiceTracker.h>

#include <memory>

/**
 * \brief Module activator for the MitkCore module.
 *
 * Registers all core services when the module is loaded, including property services
 * (aliases, descriptions, extensions, filters, persistence, relations, deserialization),
 * I/O services (MIME types, ITK/VTK image readers/writers, point set and surface I/O,
 * legacy writer adapters), node selection, plane position management, and preferences.
 *
 * On unload, all registered services and file I/O instances are cleaned up.
 *
 * \sa mitk::CoreServices
 * \sa us::ModuleActivator
 */
class MitkCoreActivator : public us::ModuleActivator
{
public:
  /**
   * \brief Load and register all core services.
   *
   * Called by CppMicroServices when the MitkCore module is loaded. Registers
   * default MIME types, ITK/VTK readers and writers, point set I/O, geometry
   * data I/O, raw image reader, legacy writers, property services, preferences,
   * and MIME type provider.
   *
   * \param[in] context The module context for service registration.
   */
  void Load(us::ModuleContext *context) override;

  /**
   * \brief Unload and clean up all registered services.
   *
   * Called by CppMicroServices when the MitkCore module is unloaded. Deletes
   * all file reader, writer, and I/O instances and stops the MIME type provider.
   */
  void Unload(us::ModuleContext *) override;

private:
  /**
   * \brief Handle module lifecycle events.
   *
   * \param[in] moduleEvent The module event to handle.
   */
  void HandleModuleEvent(const us::ModuleEvent moduleEvent);

  /** \brief Register default MIME types for common file formats. */
  void RegisterDefaultMimeTypes();

  /** \brief Register ITK-based image reader/writer services. */
  void RegisterItkReaderWriter();

  /** \brief Register VTK-based image and surface reader/writer services. */
  void RegisterVtkReaderWriter();

  // mitk::RenderingManager::Pointer m_RenderingManager;
  std::unique_ptr<mitk::NodeSelectionService> m_NodeSelectionService;
  std::unique_ptr<mitk::PlanePositionManagerService> m_PlanePositionManager;
  std::unique_ptr<mitk::PropertyAliases> m_PropertyAliases;
  std::unique_ptr<mitk::PropertyDescriptions> m_PropertyDescriptions;
  std::unique_ptr<mitk::PropertyDeserialization> m_PropertyDeserialization;
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

  std::vector<mitk::CustomMimeType *> m_DefaultMimeTypes;

  us::ServiceRegistration<mitk::IMimeTypeProvider> m_MimeTypeProviderReg;

  us::ModuleContext *m_Context;
};

#endif
