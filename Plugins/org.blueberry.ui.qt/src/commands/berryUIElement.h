/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYUIELEMENT_H_
#define BERRYUIELEMENT_H_

#include <berryMacros.h>
#include <berryObject.h>

#include <org_blueberry_ui_qt_Export.h>

#include <QIcon>

namespace berry {

struct IServiceLocator;

/**
 * Allow a command or application to provide feedback to a user through updating
 * a MenuItem or ToolItem. Initially used to update properties for UI elements
 * created by the CommandContributionItem.
 * <p>
 * This class may be extended by clients.
 * </p>
 */
class BERRY_UI_QT UIElement : public Object
{

private:

  IServiceLocator* serviceLocator;

protected:

  /**
   * Construct a new instance of this class keyed off of the provided service
   * locator.
   *
   * @param serviceLocator
   *            the locator. May not be <code>null</code>.
   */
  UIElement(IServiceLocator* serviceLocator);

public:

  berryObjectMacro(berry::UIElement);

  /**
   * Update the label on this UI element.
   *
   * @param text
   *            The new label to display.
   */
  virtual void SetText(const QString& text) = 0;

  /**
   * Update the tooltip on this UI element. Tooltips are currently only valid
   * for toolbar contributions.
   *
   * @param text
   *            The new tooltip to display.
   */
  virtual void SetToolTip(const QString& text) = 0;

  /**
   * Update the icon on this UI element.
   *
   * @param icon
   *            The new icon to display.
   */
  virtual void SetIcon(const QIcon& icon) = 0;

  /**
   * Update the checked state on this UI element. For example, if this was a
   * toggle or radio button.
   *
   * @param checked
   *            true to set toggle on
   */
  virtual void SetChecked(bool checked) = 0;

  /**
   * Get the service locator scope in which this UI element resides. May not
   * be <code>null</code>.
   *
   * <p>
   * The locator may be used to obtain services that are scoped in the same
   * way as the {@link UIElement}. Such services include but are not limited
   * to {@link IWorkbench}, {@link IWorkbenchWindow}, and
   * {@link IWorkbenchPartSite}. While this method may not return
   * <code>null</code> requests for any of these particular services may
   * return <code>null</code>.
   * </p>
   *
   * @return the service locator for this element
   * @see IServiceLocator#getService(Class)
   */
  IServiceLocator* GetServiceLocator() const;

  /**
   * Set the menu contribution id to use. This is only applicable to menu
   * contributions that support a drop-down style menu. The default
   * implementation does nothing.
   * <p>
   * Example: element.setDropdownId("org.eclipse.ui.navigate.back.my.menu");
   * </p>
   *
   * @param id
   *            used to populate the dropdown menu. Must not be
   *            <code>null</code>.
   */
  virtual void SetDropDownId(const QString& id);

};

}

#endif /* BERRYUIELEMENT_H_ */
