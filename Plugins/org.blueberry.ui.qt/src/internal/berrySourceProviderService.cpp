/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berrySourceProviderService.h"

#include "berryAbstractSourceProvider.h"
#include "berryWorkbenchServiceRegistry.h"

namespace berry {

SourceProviderService::SourceProviderService(IServiceLocator *locator)
  : locator(locator)
{
}

void SourceProviderService::Dispose()
{
  sourceProvidersByName.clear();
}

SmartPointer<ISourceProvider> SourceProviderService::GetSourceProvider(const QString& sourceName) const
{
  return sourceProvidersByName.value(sourceName);
}

QList<SmartPointer<ISourceProvider> > SourceProviderService::GetSourceProviders() const
{
  return sourceProviders.toList();
}

void SourceProviderService::RegisterProvider(const SmartPointer<ISourceProvider>& sourceProvider)
{
  if (sourceProvider.IsNull())
  {
    throw ctkInvalidArgumentException("The source provider cannot be null");
  }

  const QList<QString> sourceNames = sourceProvider->GetProvidedSourceNames();
  for (int i = 0; i < sourceNames.size(); i++)
  {
    const QString sourceName = sourceNames[i];
    sourceProvidersByName.insert(sourceName, sourceProvider);
  }
  sourceProviders.insert(sourceProvider);
}

void SourceProviderService::UnregisterProvider(const SmartPointer<ISourceProvider>& sourceProvider)
{
  if (sourceProvider.IsNull())
  {
    throw ctkInvalidArgumentException("The source provider cannot be null");
  }

  const QList<QString> sourceNames = sourceProvider->GetProvidedSourceNames();
  for (int i = 0; i < sourceNames.size(); i++)
  {
    sourceProvidersByName.remove(sourceNames[i]);
  }
  sourceProviders.remove(sourceProvider);
}

void SourceProviderService::ReadRegistry()
{
  QList<AbstractSourceProvider::Pointer> sp = WorkbenchServiceRegistry::GetRegistry()->GetSourceProviders();
  for (int i = 0; i < sp.size(); i++)
  {
    sp[i]->Initialize(locator);
    RegisterProvider(sp[i]);
  }
}

}
