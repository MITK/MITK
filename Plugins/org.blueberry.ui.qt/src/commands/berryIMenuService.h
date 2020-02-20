/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYIMENUSERVICE_H_
#define BERRYIMENUSERVICE_H_

#include <berryIServiceWithSources.h>

namespace berry {

class AbstractContributionFactory;
class ContributionManager;
struct IEvaluationContext;

/**
 * <p>
 * Provides services related to the menu architecture within the workbench. It
 * can be used to contribute additional items to the menu, tool bar and status
 * line.
 * </p>
 * <p>
 * This service can be acquired from your service locator:
 * <pre>
 *  IMenuService service = (IMenuService) getSite().getService(IMenuService.class);
 * </pre>
 * <ul>
 * <li>This service is available globally.</li>
 * </ul>
 * </p>
 *
 * @noimplement This interface is not intended to be implemented by clients.
 * @noextend This interface is not intended to be extended by clients.
 */
struct IMenuService : public IServiceWithSources
{

  berryObjectMacro(berry::IMenuService);

  /**
   * Contribute and initialize the contribution factory. This should only be
   * called once per factory. After the call, the factory should be treated as
   * an unmodifiable object.
   * <p>
   * <b>Note:</b> factories should be removed when no longer necessary. If
   * not, they will be removed when the IServiceLocator used to acquire this
   * service is disposed.
   * </p>
   *
   * @param factory
   *            the contribution factory. Must not be <code>null</code>
   * @see #removeContributionFactory(AbstractContributionFactory)
   */
  virtual void AddContributionFactory(const SmartPointer<AbstractContributionFactory>& factory) = 0;

  /**
   * Remove the contributed factory from the menu service. If the factory is
   * not contained by this service, this call does nothing.
   *
   * @param factory
   *            the contribution factory to remove. Must not be
   *            <code>null</code>.
   */
  virtual void RemoveContributionFactory(const SmartPointer<AbstractContributionFactory>& factory) = 0;

  /**
   * Populate a <code>ContributionManager</code> at the specified starting
   * location with a set of <code>IContributionItems</code>s. It applies
   * <code>AbstractContributionFactory</code>s that are stored against the
   * provided location.
   *
   * @param mgr
   *            The ContributionManager to populate
   * @param location
   *            The starting location to begin populating this contribution
   *            manager. The format is the Menu API URI format.
   * @see #releaseContributions(ContributionManager)
   */
  virtual void PopulateContributionManager(ContributionManager* mgr,
                                           const QString& location) = 0;

  /**
   * Before calling dispose() on a ContributionManager populated by the menu
   * service, you must inform the menu service to release its contributions.
   * This takes care of unregistering any IContributionItems that have their
   * visibleWhen clause managed by this menu service.
   *
   * @param mgr
   *            The manager that was populated by a call to
   *            {@link #populateContributionManager(ContributionManager, String)}
   */
  virtual void ReleaseContributions(ContributionManager* mgr) = 0;

  /**
   * Get the current state of eclipse as seen by the menu service.
   *
   * @return an IEvaluationContext containing state variables.
   *
   * @see org.eclipse.ui.ISources
   * @see org.eclipse.ui.services.IEvaluationService
   */
  virtual SmartPointer<IEvaluationContext> GetCurrentState() const = 0;
};

}

Q_DECLARE_INTERFACE(berry::IMenuService, "org.blueberry.ui.IMenuService")

#endif /* BERRYIMENUSERVICE_H_ */
