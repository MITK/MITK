/*=========================================================================

 Program:   openCherry Platform
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


#ifndef CHERRYCOMMANDCONTRIBUTIONITEM_H_
#define CHERRYCOMMANDCONTRIBUTIONITEM_H_

#include "../commands/cherryContributionItem.h"
#include "cherryUIElement.h"
#include "cherryICommandListener.h"

#include <map>

namespace cherry {

namespace GuiTk { class Event; }

struct IMenuListener;
struct IMenuItem;
struct IMenuService;
struct IUIElementListener;
struct ICommandService;
struct IHandlerService;
struct IElementReference;

class CommandEvent;
class ParameterizedCommand;
class ImageDescriptor;
class CommandContributionItemParameter;

/**
 * A contribution item which delegates to a command. It can be used in {@link
 * AbstractContributionFactory#createContributionItems(IServiceLocator,
 * IContributionRoot)}.
 * <p>
 * It currently supports placement in menus and toolbars.
 * </p>
 * <p>
 * This class may be instantiated; it is not intended to be subclassed.
 * </p>
 *
 * @since 3.3
 */
class CHERRY_UI CommandContributionItem : public ContributionItem {

public:

  /**
   * Mode bit: Show text on tool items or buttons, even if an image is
   * present. If this mode bit is not set, text is only shown on tool items if
   * there is no image present.
   *
   */
  static int MODE_FORCE_TEXT; // = 1;


private:

  //LocalResourceManager localResourceManager;

  SmartPointer<IUIElementListener> itemListener;

  UIElement::Pointer widget;

  SmartPointer<IMenuService> menuService;

  SmartPointer<ICommandService> commandService;

  SmartPointer<IHandlerService> handlerService;

  //IBindingService bindingService;

  SmartPointer<ParameterizedCommand> command;

  SmartPointer<ImageDescriptor> icon;

  std::string label;

  std::string tooltip;

  SmartPointer<ImageDescriptor> disabledIcon;

  SmartPointer<ImageDescriptor> hoverIcon;

  std::string mnemonic;

  SmartPointer<IElementReference> elementRef;

  bool checkedState;

  UIElement::Styles style;

  SmartPointer<ICommandListener> commandListener;

  std::string dropDownMenuOverride;

  //IWorkbenchHelpSystem workbenchHelpSystem;

  std::string helpContextId;

  int mode;

  /**
   * This is <code>true</code> when the menu contribution's visibleWhen
   * checkEnabled attribute is <code>true</code>.
   */
  bool visibleEnabled;

  class CommandUIElement : public UIElement
  {

  private:
    CommandContributionItem* item;

  public:

    CommandUIElement(CommandContributionItem* item,
        SmartPointer<IServiceLocator> serviceLocator);

          void SetChecked(bool checked);
          bool GetChecked() const;
          void SetDisabledIcon(SmartPointer<ImageDescriptor> desc);
          void SetHoverIcon(SmartPointer<ImageDescriptor> desc);
          void SetIcon(SmartPointer<ImageDescriptor> desc);
          void SetText(const std::string& text);
          void SetToolTip(const std::string& text);
          void SetDropDownId(const std::string& id);
          void SetEnabled(bool enabled);
          bool GetEnabled() const;
          void Dispose();
          bool IsDisposed() const;
        };

  class MyCommandListener : public ICommandListener
  {

  private:
    CommandContributionItem* item;

  public:

    MyCommandListener(CommandContributionItem* item);

    void CommandChanged(const SmartPointer<const CommandEvent> commandEvent);

  };

  class CommandUIElementListener : public IUIElementListener
  {
  private:
    CommandContributionItem* item;

    public:

      CommandUIElementListener(CommandContributionItem* item);

      void UIElementDisposed(UIElement* item);

  void UIElementSelected(SmartPointer<UIElement> item);
  };

public:

  /**
   * Create a CommandContributionItem to place in a ContributionManager.
   *
   * @param contributionParameters
   *    parameters necessary to render this contribution item.
   * @since 3.4
   */
  CommandContributionItem(
      const CommandContributionItemParameter& contributionParameters);

  /**
   * Create a CommandContributionItem to place in a ContributionManager.
   *
   * @param serviceLocator
   *    a service locator that is most appropriate for this contribution.
   *    Typically the local {@link IWorkbenchWindow} or {@link
   *    IWorkbenchPartSite} will be sufficient.
   * @param id
   *    The id for this item. May be <code>null</code>. Items without an id
   *    cannot be referenced later.
   * @param commandId
   *    A command id for a defined command. Must not be <code>null</code>.
   * @param parameters
   *    A map of strings to strings which represent parameter names to
   *    values. The parameter names must match those in the command
   *    definition.
   * @param icon
   *    An icon for this item. May be <code>null</code>.
   * @param disabledIcon
   *    A disabled icon for this item. May be <code>null</code>.
   * @param hoverIcon
   *    A hover icon for this item. May be <code>null</code>.
   * @param label
   *    A label for this item. May be <code>null</code>.
   * @param mnemonic
   *    A mnemonic for this item to be applied to the label. May be
   *    <code>null</code>.
   * @param tooltip
   *    A tooltip for this item. May be <code>null</code>. Tooltips are
   *    currently only valid for toolbar contributions.
   * @param style
   *    The style of this menu contribution. See the STYLE_* contants.
   * @deprecated create the {@link CommandContributionItemParameter}
   */
  CommandContributionItem(
      const SmartPointer<IServiceLocator> serviceLocator,
      const std::string& id,
      const std::string& commandId,
      const std::map<std::string, Object::Pointer>& parameters,
      SmartPointer<ImageDescriptor> icon,
      SmartPointer<ImageDescriptor> disabledIcon,
      SmartPointer<ImageDescriptor> hoverIcon,
      const std::string& label,
      const std::string& mnemonic,
      const std::string& tooltip,
      UIElement::Styles style);

  ~CommandContributionItem();

  /*
   * (non-Javadoc)
   *
   * @see
   * org.eclipse.jface.action.ContributionItem#fill(org.eclipse.swt.widgets
   * .Menu, int)
   */
  void Fill(SmartPointer<IMenu> parent, int index);

  /*
   * (non-Javadoc)
   *
   * @see
   * org.eclipse.jface.action.ContributionItem#fill(org.eclipse.swt.widgets
   * .ToolBar, int)
   */
  void Fill(SmartPointer<IToolBar> parent, int index);

  /*
   * (non-Javadoc)
   *
   * @see org.eclipse.jface.action.ContributionItem#update(java.lang.String)
   */
  void Update(const std::string& id = "");

  /*
   * (non-Javadoc)
   *
   * @see org.eclipse.jface.action.ContributionItem#dispose()
   */
  void Dispose();

  /*
   * (non-Javadoc)
   *
   * @see org.eclipse.jface.action.ContributionItem#isEnabled()
   */
  bool IsEnabled();

  /**
   * @since 3.4
   */
  bool IsVisible();

  void UpdateCommandPropertiesInUI(const SmartPointer<
    const CommandEvent>& commandEvent);


private:

  void Init(const CommandContributionItemParameter& p);

  void SetImages(const SmartPointer<IServiceLocator> locator, const std::string& iconStyle);

  SmartPointer<ICommandListener> GetCommandListener();

  void UpdateCommandProperties(const SmartPointer<const CommandEvent> commandEvent);

  SmartPointer<ParameterizedCommand> GetCommand();

  void CreateCommand(const std::string& commandId, const std::map<std::string, Object::Pointer>& parameters);

  std::string UpdateMnemonic(const std::string& s);

  void HandleWidgetDispose(UIElement* widget);

//  void disposeOldImages() {
//    if (localResourceManager != null) {
//      localResourceManager.dispose();
//      localResourceManager = null;
//    }
//  }

  SmartPointer<IUIElementListener> GetItemListener();

  void HandleWidgetSelection(SmartPointer<UIElement> element);

  /**
   * Determines if the selection was on the dropdown affordance and, if so,
   * opens the drop down menu (populated using the same id as this item...
   *
   * @param event
   *    The <code>SWT.Selection</code> event to be tested
   *
   * @return <code>true</code> iff a drop down menu was opened
   */
  //TODO Tool item drop down menu contributions
  //bool OpenDropDownMenu(SmartPointer<GuiTk::Event> event);

  void SetIcon(SmartPointer<ImageDescriptor> desc);

  void UpdateIcons();

  void SetText(const std::string& text);

  void SetChecked(bool checked);

  void SetToolTip(const std::string& text);

  void SetDisabledIcon(SmartPointer<ImageDescriptor> desc);

  void SetHoverIcon(SmartPointer<ImageDescriptor> desc);


};

}


#endif /* CHERRYCOMMANDCONTRIBUTIONITEM_H_ */
