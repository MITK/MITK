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


#ifndef BERRYCOMMANDCONTRIBUTIONITEM_H_
#define BERRYCOMMANDCONTRIBUTIONITEM_H_

#include "berryContributionItem.h"

#include <QIcon>

namespace berry {

struct IMenuService;
struct ICommandService;
struct ICommandListener;
struct IHandlerService;
struct IHandler;
struct IElementReference;
struct IServiceLocator;

class CommandEvent;
class ParameterizedCommand;
class CommandContributionItemParameter;
class UIElement;

/**
 * A contribution item which delegates to a command. It can be used in {@link
 * AbstractContributionFactory#CreateContributionItems(IServiceLocator,
 * IContributionRoot)}.
 * <p>
 * It currently supports placement in menus and toolbars.
 * </p>
 * <p>
 * This class may be instantiated; it is not intended to be subclassed.
 * </p>
 */
class BERRY_UI_QT CommandContributionItem : public QObject, public ContributionItem
{
  Q_OBJECT

public:

  /**
   * Mode bit: Show text on tool items or buttons, even if an image is
   * present. If this mode bit is not set, text is only shown on tool items if
   * there is no image present.
   */
  static Modes modes;

private:

  //LocalResourceManager localResourceManager;

  //Listener menuItemListener;

  QAction* action;

  IMenuService* menuService;

  ICommandService* commandService;

  IHandlerService* handlerService;

  //IBindingService bindingService;

  SmartPointer<ParameterizedCommand> command;

  QIcon icon;

  QString label;

  QString tooltip;

  QChar mnemonic;

  SmartPointer<IElementReference> elementRef;

  bool checkedState;

  Style style;

  QScopedPointer<ICommandListener> commandListener;

  QString dropDownMenuOverride;

  //IWorkbenchHelpSystem workbenchHelpSystem;

  QString helpContextId;

  Modes mode;

  /**
   * This is <code>true</code> when the menu contribution's visibleWhen
   * checkEnabled attribute is <code>true</code>.
   */
  bool visibleEnabled;

  // items contributed
  QString contributedLabel;

  QIcon contributedIcon;

  SmartPointer<IServiceLocator> serviceLocator;

public:

  /**
   * Create a CommandContributionItem to place in a ContributionManager.
   *
   * @param contributionParameters
   *    parameters necessary to render this contribution item.
   */
  CommandContributionItem(
      const SmartPointer<CommandContributionItemParameter>& contributionParameters);

  ~CommandContributionItem();

  using ContributionItem::Fill;

  QAction* Fill(QMenu* parent, QAction* before);

  QAction* Fill(QToolBar* parent, QAction* before);

  void Update();

  void Update(const QString& id);

  bool IsEnabled() const;

  bool IsVisible() const;

  void UpdateCommandPropertiesInUI(const SmartPointer<const CommandEvent>& commandEvent);


private:

  void SetImages(IServiceLocator* locator, const QString &iconStyle);

  ICommandListener *GetCommandListener();

  void UpdateMenuItem();

  void UpdateToolItem();

  void UpdateCommandProperties(const SmartPointer<const CommandEvent> commandEvent);

  bool ShouldRestoreAppearance(const SmartPointer<IHandler>& handler);

  SmartPointer<ParameterizedCommand> GetCommand() const;

  void CreateCommand(const QString& commandId, const QHash<QString, Object::Pointer>& parameters);

  QString GetToolTipText(const QString& text) const;

  QString UpdateMnemonic(const QString& s);

//  void disposeOldImages() {
//    if (localResourceManager != null) {
//      localResourceManager.dispose();
//      localResourceManager = null;
//    }
//  }

  //SmartPointer<IUIElementListener> GetItemListener();

#if (QT_VERSION < QT_VERSION_CHECK(5,0,0))
  void connectNotify(const char *signal);
  void disconnectNotify(const char *signal);
#else
  void connectNotify(const QMetaMethod& signal);
  void disconnectNotify(const QMetaMethod& signal);
#endif
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

  void SetIcon(const QIcon& icon);

  void UpdateIcons();

  void SetText(const QString& text);

  void SetChecked(bool checked);

  void SetToolTip(const QString& text);

private slots:

  void HandleWidgetSelection();

};

}


#endif /* BERRYCOMMANDCONTRIBUTIONITEM_H_ */
