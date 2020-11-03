/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryCommandContributionItem.h"

#include "berryIMenuService.h"
#include "berryICommandService.h"
#include "berryICommandImageService.h"
#include "berryIContributionManager.h"
#include "berryIElementReference.h"
#include "berryIElementUpdater.h"
#include "berryUIElement.h"

#include <berryICommandListener.h>
#include <berryIEvaluationContext.h>
#include <berryIHandler.h>
#include <berryCommand.h>
#include <berryCommandEvent.h>
#include <berryParameterizedCommand.h>
#include <berryCommandExceptions.h>
#include <berryCommandContributionItemParameter.h>

#include "../berryDisplay.h"
#include "../berryAsyncRunnable.h"

#include "../handlers/berryIHandlerService.h"
#include "../services/berryIServiceLocator.h"

#include "../berryWorkbenchPlugin.h"

#include <QMenu>
#include <QMenuBar>
#include <QToolBar>
#include <QApplication>

#include <QMetaMethod>

namespace berry
{

ContributionItem::Modes CommandContributionItem::modes = ContributionItem::MODE_FORCE_TEXT;


//class CommandUIElementListener : public IUIElementListener
//{
//private:
//  CommandContributionItem* item;

//  public:

//    CommandUIElementListener(CommandContributionItem* item);

//    void UIElementDisposed(UIElement* item);

//void UIElementSelected(SmartPointer<UIElement> item);
//};


CommandContributionItem::CommandContributionItem(
    const SmartPointer<CommandContributionItemParameter>& contributionParameters)
  : ContributionItem(contributionParameters->id)
  , action(nullptr)
  , checkedState(false)
{
  this->icon = contributionParameters->icon;
  this->label = contributionParameters->label;
  this->mnemonic = contributionParameters->mnemonic;
  this->shortcut = contributionParameters->shortcut;
  this->tooltip = contributionParameters->tooltip;
  this->style = contributionParameters->style;
  this->helpContextId = contributionParameters->helpContextId;
  this->visibleEnabled = contributionParameters->visibleEnabled;
  this->mode = contributionParameters->mode;

  menuService = contributionParameters->serviceLocator->GetService<IMenuService>();
  commandService = contributionParameters->serviceLocator->GetService<ICommandService>();
  handlerService = contributionParameters->serviceLocator->GetService<IHandlerService>();
  //    bindingService = (IBindingService) contributionParameters.serviceLocator
  //        .getService(IBindingService.class);

  this->CreateCommand(contributionParameters->commandId,
                      contributionParameters->parameters);

  if (command)
  {
    try
    {
      class CommandUIElement : public UIElement
      {

      private:
        CommandContributionItem* item;

      public:

        CommandUIElement(CommandContributionItem* item, IServiceLocator* serviceLocator)
          : UIElement(serviceLocator), item(item) {}

        void SetText(const QString& text) override
        {
          item->SetText(text);
        }

        void SetToolTip(const QString& text) override
        {
          item->SetToolTip(text);
        }

        void SetIcon(const QIcon& icon) override
        {
          item->SetIcon(icon);
        }

        void SetChecked(bool checked) override
        {
          item->SetChecked(checked);
        }

        void SetDropDownId(const QString& id) override
        {
          item->dropDownMenuOverride = id;
        }

      };

      UIElement::Pointer callback(new CommandUIElement(this,
                                                       contributionParameters->serviceLocator));

      elementRef = commandService->RegisterElementForCommand(command, callback);
      command->GetCommand()->AddCommandListener(this->GetCommandListener());
      this->SetImages(contributionParameters->serviceLocator,
                      contributionParameters->iconStyle);

      if (contributionParameters->helpContextId.isEmpty())
      {
        try
        {
          this->helpContextId = commandService->GetHelpContextId(
                contributionParameters->commandId);
        }
        catch (const NotDefinedException& /*e*/)
        {
          // it's OK to not have a helpContextId
        }
      }
      //        IWorkbenchLocationService::Pointer wls = contributionParameters.serviceLocator
      //            ->GetService(IWorkbenchLocationService::GetManifestName()).Cast<IWorkbenchLocationService>();
      //        const IWorkbench* workbench = wls->GetWorkbench();;
      //        if (workbench != 0 && !helpContextId.empty()) {
      //          this->workbenchHelpSystem = workbench->GetHelpSystem();
      //        }
    }
    catch (const NotDefinedException& /*e*/)
    {
      WorkbenchPlugin::Log(QString("Unable to register menu item \"") + this->GetId()
                           + "\", command \"" + contributionParameters->commandId
                           + "\" not defined");
    }
  }
}

void CommandContributionItem::Fill(QMenu* parent, QAction* before)
{
  if (!command || action || parent == nullptr)
  {
    return;
  }

  // Menus don't support the pulldown style
  Style tmpStyle = style;
  if (tmpStyle == STYLE_PULLDOWN)
    tmpStyle = STYLE_PUSH;

  QAction* item = nullptr;
  if (before)
  {
    item = new QAction(icon, label, parent);
    parent->insertAction(before, item);
  }
  else
  {
    item = parent->addAction(icon, label);
  }

  // Remove this when key binding support is fully implemented
  if (!shortcut.isEmpty())
  {
    item->setShortcut(shortcut);
  }
  item->setData(QVariant::fromValue(Object::Pointer(this)));
  item->setProperty("contributionItem", QVariant::fromValue(Object::Pointer(this)));

  //  if (workbenchHelpSystem != null)
  //  {
  //    workbenchHelpSystem.setHelp(item, helpContextId);
  //  }

  connect(item, SIGNAL(triggered()), SLOT(HandleWidgetSelection()));
  connect(item, SIGNAL(destroyed()), SLOT(HandleActionDestroyed()));
  action = item;

  this->Update();
  this->UpdateIcons();

  //bindingService.addBindingManagerListener(bindingManagerListener);
}

void CommandContributionItem::Fill(QToolBar *parent, QAction *before)
{
  if (!command || action || parent == nullptr)
  {
    return;
  }

  QAction* item = nullptr;
  if (before)
  {
    item = parent->addAction(icon, label);
  }
  else
  {
    item = new QAction(icon, label, parent);
    parent->insertAction(before, item);
  }
  item->setData(QVariant::fromValue(Object::Pointer(this)));
  item->setProperty("contributionItem", QVariant::fromValue(Object::Pointer(this)));

  connect(item, SIGNAL(triggered()), SLOT(HandleWidgetSelection()));
  connect(item, SIGNAL(destroyed()), SLOT(HandleActionDestroyed()));
  action = item;

  this->Update();
  this->UpdateIcons();

  //bindingService.addBindingManagerListener(bindingManagerListener);
}

void CommandContributionItem::Update()
{
  this->Update(QString::null);
}

void CommandContributionItem::Update(const QString& /*id*/)
{
  if (action)
  {
    QWidget* parent = action->parentWidget();
    if(qobject_cast<QMenu*>(parent))
    {
      this->UpdateMenuItem();
    }
    else if (qobject_cast<QMenuBar*>(parent))
    {
      this->UpdateMenuItem();
    }
    else if (qobject_cast<QToolBar*>(parent))
    {
      this->UpdateToolItem();
    }
  }

}

void CommandContributionItem::UpdateMenuItem()
{
  QString text = label;
  if (text.isEmpty())
  {
    if (command.IsNotNull())
    {
      try
      {
        text = command->GetCommand()->GetName();
      }
      catch (const NotDefinedException& e)
      {
//        StatusManager.getManager().handle(
//              StatusUtil.newStatus(IStatus.ERROR,
//                                   "Update item failed "
//                                   + getId(), e));
        BERRY_ERROR << "Update item failed " << GetId() << e.what();
      }
    }
  }
  text = UpdateMnemonic(text);

//  String keyBindingText = null;
//  if (command != null)
//  {
//    TriggerSequence binding = bindingService
//        .getBestActiveBindingFor(command);
//    if (binding != null)
//    {
//      keyBindingText = binding.format();
//    }
//  }
//  if (text != null)
//  {
//    if (keyBindingText == null)
//    {
//      item.setText(text);
//    }
//    else
//    {
//      item.setText(text + '\t' + keyBindingText);
//    }
//  }

  if (action->isChecked() != checkedState)
  {
    action->setChecked(checkedState);
  }

  // allow the handler update its enablement
  bool shouldBeEnabled = IsEnabled();
  if (action->isEnabled() != shouldBeEnabled)
  {
    action->setEnabled(shouldBeEnabled);
  }
}

void CommandContributionItem::UpdateToolItem()
{
  QString text = label;
  QString tooltip = label;
  if (text.isNull())
  {
    if (command.IsNotNull())
    {
      try
      {
        text = command->GetCommand()->GetName();
        tooltip = command->GetCommand()->GetDescription();
        if (tooltip.trimmed().isEmpty())
        {
          tooltip = text;
        }
      }
      catch (const NotDefinedException& e)
      {
//        StatusManager.getManager().handle(
//              StatusUtil.newStatus(IStatus.ERROR,
//                                   "Update item failed "
//                                   + getId(), e));
        BERRY_ERROR << "Update item failed " << GetId() << e.what();
      }
    }
  }

  if ((icon.isNull() || (mode & MODE_FORCE_TEXT) == MODE_FORCE_TEXT)
      && !text.isNull())
  {
    action->setText(text);
  }

  QString toolTipText = GetToolTipText(tooltip);
  action->setToolTip(toolTipText);

  if (action->isChecked() != checkedState)
  {
    action->setChecked(checkedState);
  }

  // allow the handler update its enablement
  bool shouldBeEnabled = IsEnabled();
  if (action->isEnabled() != shouldBeEnabled)
  {
    action->setEnabled(shouldBeEnabled);
  }
}

CommandContributionItem::~CommandContributionItem()
{
  if (elementRef)
  {
    commandService->UnregisterElement(elementRef);
  }
  if (commandListener)
  {
    command->GetCommand()->RemoveCommandListener(commandListener.data());
  }
}

bool CommandContributionItem::IsEnabled() const
{
  if (command)
  {
    command->GetCommand()->SetEnabled(menuService->GetCurrentState());
    return command->GetCommand()->IsEnabled();
  }
  return false;
}

bool CommandContributionItem::IsVisible() const
{
  if (visibleEnabled)
  {
    return ContributionItem::IsVisible() && this->IsEnabled();
  }
  return ContributionItem::IsVisible();
}

void CommandContributionItem::SetImages(IServiceLocator* locator,
                                        const QString& iconStyle)
{
  if (icon.isNull())
  {
    ICommandImageService* service = locator->GetService<ICommandImageService>();
    if (service)
    {
      icon = service->GetImage(command->GetId(), iconStyle);
    }
  }
}

ICommandListener* CommandContributionItem::GetCommandListener()
{
  if (!commandListener)
  {
    class MyCommandListener : public ICommandListener
    {

    private:
      CommandContributionItem* item;

    public:

      MyCommandListener(CommandContributionItem* item)
        : item(item)
      {}

      void CommandChanged(const SmartPointer<const CommandEvent>& commandEvent) override
      {
        if (commandEvent->IsHandledChanged() || commandEvent->IsEnabledChanged()
            || commandEvent->IsDefinedChanged())
        {
          item->UpdateCommandProperties(commandEvent);
        }
      }

    };

    commandListener.reset(new MyCommandListener(this));
  }
  return commandListener.data();
}

void CommandContributionItem::UpdateCommandProperties(const SmartPointer<
    const CommandEvent> commandEvent)
{
  if (commandEvent->IsHandledChanged())
  {
    dropDownMenuOverride = "";
  }
  if (!action)
  {
    return;
  }
  Display* display = Display::GetDefault();
  typedef AsyncRunnable<SmartPointer<const CommandEvent>, CommandContributionItem > UpdateRunnable;
  Poco::Runnable* update = new UpdateRunnable(this, &CommandContributionItem::UpdateCommandPropertiesInUI, commandEvent);
  if (display->InDisplayThread())
  {
    update->run();
  }
  else
  {
    display->AsyncExec(update);
  }
}

void CommandContributionItem::UpdateCommandPropertiesInUI(const SmartPointer<
    const CommandEvent>& commandEvent)
{
   if (commandEvent->GetCommand()->IsDefined())
   {
     this->Update();
   }
   if (commandEvent->IsEnabledChanged()
       || commandEvent->IsHandledChanged())
   {
     if (visibleEnabled)
     {
       IContributionManager* parent = this->GetParent();
       if (parent)
       {
         parent->Update(true);
       }
     }
   }
}

void CommandContributionItem::HandleActionDestroyed()
{
  this->action = nullptr;
}

bool CommandContributionItem::ShouldRestoreAppearance(const SmartPointer<IHandler>& handler)
{
  // if no handler or handler doesn't implement IElementUpdater,
  // restore the contributed elements
  if (handler.IsNull())
    return true;

  if (!(handler.Cast<IElementUpdater>()))
    return true;

  // special case, if its HandlerProxy, then check the actual handler
//  if (handler instanceof HandlerProxy) {
//    HandlerProxy handlerProxy = (HandlerProxy) handler;
//    IHandler actualHandler = handlerProxy.getHandler();
//    return shouldRestoreAppearance(actualHandler);
//  }
  return false;
}

SmartPointer<ParameterizedCommand> CommandContributionItem::GetCommand() const
{
  return command;
}

void CommandContributionItem::CreateCommand(const QString &commandId,
    const QHash<QString,Object::Pointer> &parameters)
{
  if (commandId.isEmpty())
  {
//    StatusManager.getManager().handle(StatusUtil.newStatus(IStatus.ERROR,
//                                                           "Unable to create menu item \"" + getId()
//                                                           + "\", no command id", null));
    BERRY_ERROR << "Unable to create menu item \"" << this->GetId().toStdString()
                << "\", no command id";
    return;
  }
  Command::Pointer cmd = commandService->GetCommand(commandId);
  if (!cmd->IsDefined())
  {
//    StatusManager.getManager().handle(StatusUtil.newStatus(
//                                        IStatus.ERROR, "Unable to create menu item \"" + getId()
//                                        + "\", command \"" + commandId + "\" not defined", null));
    BERRY_ERROR << "Unable to create menu item \"" << this->GetId().toStdString()
                << "\", command \"" << commandId.toStdString() << "\" not defined";
    return;
  }

  command = ParameterizedCommand::GenerateCommand(cmd, parameters);
}

QString CommandContributionItem::GetToolTipText(const QString& text) const
{
  QString tooltipText = tooltip;
  if (tooltip.isNull())
  {
    if (!text.isNull())
    {
      tooltipText = text;
    }
    else
    {
      tooltipText = "";
    }
  }

//  TriggerSequence activeBinding = bindingService
//      .getBestActiveBindingFor(command);
//  if (activeBinding != null && !activeBinding.isEmpty())
//  {
//    String acceleratorText = activeBinding.format();
//    if (acceleratorText != null
//        && acceleratorText.length() != 0)
//    {
//      tooltipText = NLS.bind(CommandMessages.Tooltip_Accelerator,
//                             tooltipText, acceleratorText);
//    }
//  }

  return tooltipText;
}

QString CommandContributionItem::UpdateMnemonic(const QString &s)
{
  if (mnemonic.isNull() || s.isEmpty())
  {
    return s;
  }

  int idx = s.indexOf(mnemonic);
  if (idx == -1)
  {
    return s;
  }

  return s.left(idx) + '&' + s.mid(idx);
}

//SmartPointer<IUIElementListener> CommandContributionItem::GetItemListener()
//{
//  if (!itemListener)
//  {
//    itemListener = new CommandUIElementListener(this);
//  }
//  return itemListener;
//}

void CommandContributionItem::HandleWidgetSelection()
{
//  // Special check for ToolBar dropdowns...
//  if (this->OpenDropDownMenu(event))
//  //return;

  if ((style & STYLE_CHECK) != 0)
  {
    checkedState = action->isChecked();
  }

  try
  {
    handlerService->ExecuteCommand(command, UIElement::Pointer(nullptr));
  }
  catch (const ExecutionException& e)
  {
    WorkbenchPlugin::Log("Failed to execute item " + GetId(), e);
  }
  catch (const NotDefinedException& e)
  {
    WorkbenchPlugin::Log("Failed to execute item " + GetId(), e);
  }
  catch (const NotEnabledException& e)
  {
    WorkbenchPlugin::Log("Failed to execute item " + GetId(), e);
  }
  catch (const NotHandledException& e)
  {
    WorkbenchPlugin::Log("Failed to execute item " + GetId(), e);
  }
}

//TODO Tool item drop down menu contributions
//bool CommandContributionItem::OpenDropDownMenu(SmartPointer<GuiTk::Event> event)
//{
//Widget item = event.widget;
//if (item != null)
//{
//  int style = item.getStyle();
//  if ((style & SWT.DROP_DOWN) != 0)
//  {
//    if (event.detail == 4)
//    { // on drop-down button
//      ToolItem ti = (ToolItem) item;
//
//      final MenuManager menuManager = new MenuManager();
//      Menu menu = menuManager.createContextMenu(ti.getParent());
//      if (workbenchHelpSystem != null)
//      {
//        workbenchHelpSystem.setHelp(menu, helpContextId);
//      }
//      menuManager.addMenuListener(new IMenuListener()
//          {
//          public void menuAboutToShow(IMenuManager manager)
//            {
//              String id = getId();
//              if (dropDownMenuOverride != null)
//              {
//                id = dropDownMenuOverride;
//              }
//              menuService.populateContributionManager(
//                  menuManager, "menu:" + id); //$NON-NLS-1$
//            }
//          });
//
//      // position the menu below the drop down item
//      Point point = ti.getParent().toDisplay(
//          new Point(event.x, event.y));
//      menu.setLocation(point.x, point.y); // waiting for SWT
//      // 0.42
//      menu.setVisible(true);
//      return true; // we don't fire the action
//    }
//  }
//}
//
//return false;
//}

void CommandContributionItem::SetIcon(const QIcon &icon)
{
  this->icon = icon;
  this->UpdateIcons();
}

void CommandContributionItem::UpdateIcons()
{
  action->setIcon(icon);
}

void CommandContributionItem::SetText(const QString &text)
{
  label = text;
  this->Update();
}

void CommandContributionItem::SetChecked(bool checked)
{
  if (checkedState == checked)
  {
    return;
  }
  checkedState = checked;
  action->setChecked(checkedState);
}

void CommandContributionItem::SetToolTip(const QString &text)
{
  tooltip = text;
  action->setToolTip(text);
}

}
