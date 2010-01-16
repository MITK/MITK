/*=========================================================================

 Program:   BlueBerry Platform
 Language:  C++
 Date:      $Date$
 Version:   $Revision$

 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/


#ifndef BERRYUIELEMENT_H_
#define BERRYUIELEMENT_H_

#include <osgi/framework/Object.h>
#include <osgi/framework/Macros.h>
#include <osgi/framework/Flags.h>

#include "../berryUiDll.h"

#include "../services/berryIServiceLocator.h"
#include "berryIUIElementListener.h"

#include <Poco/Exception.h>

namespace berry {

using namespace osgi::framework;

class ImageDescriptor;

/**
 * Allow a command or application to provide feedback to a user through updating
 * a MenuItem or ToolItem. Initially used to update properties for UI elements
 * created by the CommandContributionItem.
 * <p>
 * This class may be extended by clients.
 * </p>
 *
 * @since 3.3
 */
class BERRY_UI UIElement : public Object {

private:

  SmartPointer<IServiceLocator> serviceLocator;
  Object::Pointer data;

  IUIElementListener::Events events;

protected:

  /**
   * Construct a new instance of this class keyed off of the provided service
   * locator.
   *
   * @param serviceLocator
   *            the locator. May not be <code>null</code>.
   */
  UIElement(SmartPointer<IServiceLocator> serviceLocator)
      throw(Poco::InvalidArgumentException);

public:

  osgiObjectMacro(UIElement)

  enum Style {
    STYLE_NONE = 0x0,
    STYLE_PUSH = 0x1,
    STYLE_CHECK = 0x2,
    STYLE_RADIO = 0x4,
    STYLE_TOGGLE = 0x8,
    STYLE_PULLDOWN = 0x10,
    STYLE_DROPDOWN = 0x20,
    STYLE_CASCADE = 0x30,
    STYLE_SEPARATOR = 0x100
  };

  OSGI_DECLARE_FLAGS(Styles, Style)

  virtual void AddListener(SmartPointer<IUIElementListener> l);
  virtual void RemoveListener(SmartPointer<IUIElementListener> l);

  virtual void SetData(Object::Pointer data);
  virtual Object::Pointer GetData() const;

  /**
   * Update the label on this UI element.
   *
   * @param text
   *            The new label to display.
   */
  virtual void SetText(const std::string& text) = 0;

  /**
   * Update the tooltip on this UI element. Tooltips are currently only valid
   * for toolbar contributions.
   *
   * @param text
   *            The new tooltip to display.
   */
  virtual void SetToolTip(const std::string& text) = 0;

  /**
   * Update the icon on this UI element.
   *
   * @param desc
   *            The descriptor for the new icon to display.
   */
  virtual void SetIcon(SmartPointer<ImageDescriptor> desc) = 0;

  /**
   * Update the disabled icon on this UI element.
   *
   * @param desc
   *            The descriptor for the new icon to display.
   */
  virtual void SetDisabledIcon(SmartPointer<ImageDescriptor> desc) = 0;

  /**
   * Update the hover icon on this UI element.
   *
   * @param desc
   *            The descriptor for the new icon to display.
   */
  virtual void SetHoverIcon(SmartPointer<ImageDescriptor> desc) = 0;

  /**
   * Update the checked state on this UI element. For example, if this was a
   * toggle or radio button.
   *
   * @param checked
   *            true to set toggle on
   */
  virtual void SetChecked(bool checked) = 0;
  virtual bool GetChecked() const = 0;

  virtual void Dispose() = 0;
  virtual bool IsDisposed() const = 0;

  virtual void SetEnabled(bool enabled) = 0;
  virtual bool GetEnabled() const = 0;

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
  SmartPointer<IServiceLocator> GetServiceLocator() const;

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
  virtual void SetDropDownId(const std::string& id);

};

}

OSGI_DECLARE_OPERATORS_FOR_FLAGS(berry::UIElement::Styles)

#endif /* BERRYUIELEMENT_H_ */
