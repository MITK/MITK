/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkPersistenceActivator.h"

mitk::PersistenceActivator::PersistenceActivator()
{
}

void mitk::PersistenceActivator::Load(us::ModuleContext *context)
{
  MITK_DEBUG << "PersistenceActivator::Load";

  mitk::IPersistenceService *persistenceService = nullptr;
  us::ServiceReference<mitk::IPersistenceService> persistenceServiceRef =
    context->GetServiceReference<mitk::IPersistenceService>();
  if (persistenceServiceRef)
  {
    persistenceService =
      dynamic_cast<mitk::IPersistenceService *>(context->GetService<mitk::IPersistenceService>(persistenceServiceRef));
  }

  bool instanceAlreadyAdded = persistenceService != nullptr;
  if (instanceAlreadyAdded)
    instanceAlreadyAdded = dynamic_cast<mitk::PersistenceService *>(persistenceService) != nullptr;

  if (instanceAlreadyAdded == false)
  {
    // Registering PersistenceService as MicroService
    m_PersistenceService = itk::SmartPointer<mitk::PersistenceService>(new PersistenceService());
    us::ServiceProperties _PersistenceServiceProps;
    _PersistenceServiceProps["Name"] = std::string("PersistenceService");

    m_PersistenceServiceRegistration =
      context->RegisterService<mitk::IPersistenceService>(m_PersistenceService, _PersistenceServiceProps);
  }
  else
  {
    MITK_ERROR
      << "Another Persistence instance already installed. Library was loaded twice. Please check configuration!";
  }
}

void mitk::PersistenceActivator::Unload(us::ModuleContext *)
{
  if (m_PersistenceService.IsNull())
    return;

  MITK_DEBUG("PersistenceActivator") << "PersistenceActivator::Unload";
  MITK_DEBUG("PersistenceActivator") << "m_PersistenceService GetReferenceCount "
                                     << m_PersistenceService->GetReferenceCount();

  m_PersistenceServiceRegistration.Unregister();
  m_PersistenceService->Unitialize();
  m_PersistenceService->Delete();
}

mitk::PersistenceActivator::~PersistenceActivator()
{
}
