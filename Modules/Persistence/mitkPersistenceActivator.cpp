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
#ifndef mitkPersistenceActivator_h
#define mitkPersistenceActivator_h

// Microservices
#include <usModuleActivator.h>
#include "mitkPersistenceService.h"
#include <usServiceRegistration.h>

namespace mitk
{
    ///
    /// installs the PersistenceService
    /// runs all initial commands (setting env paths etc)
    ///
    class PersistenceActivator : public us::ModuleActivator
    {
    public:

        void Load(us::ModuleContext* context)
        {
          MITK_DEBUG << "PersistenceActivator::Load";
          // Registering PersistenceService as MicroService
          m_PersistenceService = itk::SmartPointer<mitk::PersistenceService>(new PersistenceService());
          us::ServiceProperties _PersistenceServiceProps;
          _PersistenceServiceProps["Name"] = std::string("PersistenceService");

          m_PersistenceServiceRegistration = context->RegisterService<mitk::IPersistenceService>(m_PersistenceService, _PersistenceServiceProps);

          // Load Default File in any case
          m_PersistenceService->Load();
          std::string id = mitk::PersistenceService::PERSISTENCE_PROPERTYLIST_NAME;
          mitk::PropertyList::Pointer propList = m_PersistenceService->GetPropertyList( id );
          bool autoLoadAndSave = true;
          propList->GetBoolProperty("m_AutoLoadAndSave", autoLoadAndSave);

          if( autoLoadAndSave == false )
          {
              MITK_DEBUG("mitk::PersistenceService") << "autoloading was not wished. clearing data we got so far.";
              m_PersistenceService->SetAutoLoadAndSave(false);
              m_PersistenceService->Clear();
          }

        }

        void Unload(us::ModuleContext* context)
        {
          MITK_DEBUG("PersistenceActivator") << "PersistenceActivator::Unload";
          MITK_DEBUG("PersistenceActivator") << "m_PersistenceService GetReferenceCount " << m_PersistenceService->GetReferenceCount();

          if(m_PersistenceService->GetAutoLoadAndSave())
              m_PersistenceService->Save();

          m_PersistenceServiceRegistration.Unregister();
          m_PersistenceService->Delete();
          MITK_DEBUG("PersistenceActivator") << "m_PersistenceService GetReferenceCount " << m_PersistenceService->GetReferenceCount();
        }

        virtual ~PersistenceActivator()
        {
        }

    private:
        itk::SmartPointer<mitk::PersistenceService> m_PersistenceService;
        us::ServiceRegistration<mitk::IPersistenceService> m_PersistenceServiceRegistration;
    };
}

US_EXPORT_MODULE_ACTIVATOR(Persistence, mitk::PersistenceActivator)
#endif
