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

#ifndef BERRYISOURCEPROVIDERSERVICE_H
#define BERRYISOURCEPROVIDERSERVICE_H

#include "org_blueberry_ui_qt_Export.h"

#include "berryObject.h"

#include <QList>

namespace berry {

template<class T> class SmartPointer;

struct ISourceProvider;

/**
 * <p>
 * A service from which all of the source providers can be retrieved.
 * </p>
 * <p>
 * This service can be acquired from your service locator:
 * <pre>
 *   ISourceProviderService* service = GetSite()->GetService<ISourceProviderService>();
 * </pre>
 * <ul>
 * <li>This service is available globally.</li>
 * </ul>
 * </p>
 *
 * @noimplement This interface is not intended to be implemented by clients.
 * @noextend This interface is not intended to be extended by clients.
 *
 * @see IEvaluationService
 */
struct BERRY_UI_QT ISourceProviderService : public virtual Object
{

  berryObjectMacro(berry::ISourceProviderService)

  virtual ~ISourceProviderService();

  /**
   * Retrieves a source provider providing the given source. This is used by
   * clients who only need specific sources.
   *
   * @param sourceName
   *            The name of the source; must not be <code>null</code>.
   * @return A source provider which provides the request source, or
   *         <code>null</code> if no such source exists.
   * @see ISources
   */
  virtual SmartPointer<ISourceProvider> GetSourceProvider(const QString& sourceName) const = 0;

  /**
   * Retrieves all of the source providers registered with this service at the
   * time of this call.
   * <p>
   * <code>IEvaluationService</code> can be used
   * to receive notifications about source variable changes and to
   * evaluate core expressions against source providers.
   * </p>
   *
   * @return The source providers registered with this service. This value may be empty.
   */
  virtual QList<SmartPointer<ISourceProvider> > GetSourceProviders() const = 0;

};

}

Q_DECLARE_INTERFACE(berry::ISourceProviderService, "org.blueberry.ui.ISourceProviderService")

#endif // BERRYISOURCEPROVIDERSERVICE_H
