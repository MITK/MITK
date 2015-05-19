/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef BERRYSOURCEPROVIDERSERVICE_H
#define BERRYSOURCEPROVIDERSERVICE_H

#include "berryISourceProviderService.h"
#include "berryIDisposable.h"

namespace berry {

struct IServiceLocator;

/**
 * <p>
 * A service holding all of the registered source providers.
 * </p>
 * <p>
 * This class is not intended for use outside of the
 * <code>org.eclipse.ui.workbench</code> plug-in.
 * </p>
 */
class SourceProviderService : public ISourceProviderService, public IDisposable
{

private:

  /**
   * The source providers registered with this service. This value is never
   * <code>null</code>. This is a map of the source name ({@link String})
   * to the source provider ({@link ISourceProvider}).
   */
  QHash<QString,SmartPointer<ISourceProvider> > sourceProvidersByName;

  /**
   * All of the source providers registered with this service. This value is
   * never <code>null</code>.
   */
  QSet<SmartPointer<ISourceProvider> > sourceProviders;

  IServiceLocator* locator;

public:

  berryObjectMacro(berry::SourceProviderService)

  SourceProviderService(IServiceLocator* locator);

  void Dispose() override;

  SmartPointer<ISourceProvider> GetSourceProvider(const QString& sourceName) const override;

  QList<SmartPointer<ISourceProvider> > GetSourceProviders() const override;

  void RegisterProvider(const SmartPointer<ISourceProvider>& sourceProvider);

  void UnregisterProvider(const SmartPointer<ISourceProvider>& sourceProvider);

  void ReadRegistry();
};

}

#endif // BERRYSOURCEPROVIDERSERVICE_H
