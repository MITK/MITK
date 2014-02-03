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
        PersistenceActivator()
        {
        }

        void Load(us::ModuleContext* context)
        {
          MITK_DEBUG << "PersistenceActivator::Load";

          mitk::IPersistenceService* persistenceService = 0;
          us::ServiceReference<mitk::IPersistenceService> persistenceServiceRef
            = context->GetServiceReference<mitk::IPersistenceService>();
          if( persistenceServiceRef )
          {
            persistenceService = dynamic_cast<mitk::IPersistenceService*> ( context->GetService<mitk::IPersistenceService>(persistenceServiceRef) );
          }

          bool instanceAlreadyAdded = persistenceService != 0;
          if( instanceAlreadyAdded )
            instanceAlreadyAdded = dynamic_cast<mitk::PersistenceService*>(persistenceService) != 0;

          if( instanceAlreadyAdded == false )
          {
            // Registering PersistenceService as MicroService
            m_PersistenceService = itk::SmartPointer<mitk::PersistenceService>(new PersistenceService());
            us::ServiceProperties _PersistenceServiceProps;
            _PersistenceServiceProps["Name"] = std::string("PersistenceService");

            m_PersistenceServiceRegistration = context->RegisterService<mitk::IPersistenceService>(m_PersistenceService, _PersistenceServiceProps);

          }
          else
          {
            MITK_ERROR << "Another Persistence instance already installed. Library was loaded twice. Please check configuration!";
          }
        }

        void Unload(us::ModuleContext* context)
        {
          if( m_PersistenceService.IsNull() )
            return;

          MITK_DEBUG("PersistenceActivator") << "PersistenceActivator::Unload";
          MITK_DEBUG("PersistenceActivator") << "m_PersistenceService GetReferenceCount " << m_PersistenceService->GetReferenceCount();

          m_PersistenceServiceRegistration.Unregister();
          m_PersistenceService->Unitialize();
          m_PersistenceService->Delete();
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
