/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef mitkPersistenceActivator_h
#define mitkPersistenceActivator_h

// Microservices
#include <mitkPersistenceService.h>
#include <usModuleActivator.h>
#include <usServiceRegistration.h>

namespace mitk
{
  /**
   * \brief Module activator that installs the PersistenceService.
   *
   * Creates and registers the PersistenceService as a micro-service
   * when the module is loaded.
   */
  class PersistenceActivator : public us::ModuleActivator
  {
  public:
    PersistenceActivator();

    /** \brief Create and register the PersistenceService. */
    void Load(us::ModuleContext *context) override;

    /** \brief Unregister the PersistenceService. */
    void Unload(us::ModuleContext *context) override;
    ~PersistenceActivator() override;

  private:
    itk::SmartPointer<mitk::PersistenceService> m_PersistenceService;
    us::ServiceRegistration<mitk::IPersistenceService> m_PersistenceServiceRegistration;
  };
}

US_EXPORT_MODULE_ACTIVATOR(mitk::PersistenceActivator)
#endif
