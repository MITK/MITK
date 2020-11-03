/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYABSTRACTCONTRIBUTIONFACTORY_H_
#define BERRYABSTRACTCONTRIBUTIONFACTORY_H_

#include <berryObject.h>

#include <org_blueberry_ui_qt_Export.h>

namespace berry {

struct IServiceLocator;
struct IContributionRoot;

/**
 * ContributionFactories are used by the IMenuService to populate
 * ContributionManagers. In CreateContributionItems()
 * you fill in the additions List with {@link IContributionItem} to be inserted at this
 * factory's location. For example:
 * <p>
 *
 * <pre>
 * AbstractContributionFactory contributions = new AbstractContributionFactory(
 *    &quot;menu:org.eclipse.ui.tests.api.MenuTestHarness?after=additions&quot;) {
 *  public void CreateContributionItems(IMenuService menuService, List additions) {
 *    CommandContributionItem item = new CommandContributionItem(
 *        &quot;org.eclipse.ui.tests.menus.helloWorld&quot;,
 *        &quot;org.eclipse.ui.tests.commands.enabledHelloWorld&quot;, null, null,
 *        &quot;Say Hello&quot;, null);
 *    additions.add(item);
 *    item = new CommandContributionItem(
 *        &quot;org.eclipse.ui.tests.menus.refresh&quot;,
 *        &quot;org.eclipse.ui.tests.commands.refreshView&quot;, null, null,
 *        &quot;Refresh&quot;, null);
 *    menuService.registerVisibleWhen(item, new MyActiveContextExpression(
 *        &quot;org.eclipse.ui.tests.myview.context&quot;));
 *    additions.add(item);
 *  }
 *
 *  public void releaseContributionItems(IMenuService menuService, List items) {
 *    // we have nothing to do
 *  }
 * };
 * IMenuService service = (IMenuService) PlatformUI.getWorkbench().getService(
 *    IMenuService.class);
 * service.addContributionFactory(contributions);
 * </pre>
 *
 * </p>
 * <p>
 * Only the abstract methods may be implemented.
 * </p>
 *
 * @see IMenuService
 * @see MenuManager
 * @see ToolBarManager
 */
class BERRY_UI_QT AbstractContributionFactory: public Object
{

private:

  QString location;
  QString namespaze;

public:

  berryObjectMacro(berry::AbstractContributionFactory);

  /**
   * The contribution factories must be instantiated with their location,
   * which which specifies the contributions insertion location.
   *
   * @param location
   *            the addition location in Menu API URI format. It must not be
   *            <code>null</code>.
   * @param namespace
   *            the namespace for this contribution. May be <code>null</code>.
   * @see #getNamespace()
   */
  AbstractContributionFactory(const QString& location,
                              const QString& namespaze);

  /**
   * Return the location as a String.
   *
   * @return the location - never <code>null</code>.
   */
  QString GetLocation() const;

  /**
   * This factory should create the IContributionItems that it wants to
   * contribute, and add them to the additions list. The menu service will
   * call this method at the appropriate time. It should always return new
   * instances of its contributions in the additions list.
   * <p>
   * This method is not meant to be called by clients. It will be called by
   * the menu service at the appropriate time.
   * </p>
   *
   * @param serviceLocator
   *            a service locator that may be used in the construction of
   *            items created by this factory
   * @param additions
   *            A {@link IContributionRoot} supplied by the framework. It will
   *            never be <code>null</code>.
   * @see org.eclipse.ui.menus.CommandContributionItem
   * @see org.eclipse.jface.action.MenuManager
   */
  virtual void CreateContributionItems(IServiceLocator* serviceLocator,
                                       const SmartPointer<IContributionRoot>& additions) = 0;

  /**
   * Return the namespace for this cache. This corresponds to the plug-in that
   * is contributing this factory.
   *
   * @return the namespace the namespace of this factory
   */
  QString GetNamespace() const;

};

}

#endif /* BERRYABSTRACTCONTRIBUTIONFACTORY_H_ */
