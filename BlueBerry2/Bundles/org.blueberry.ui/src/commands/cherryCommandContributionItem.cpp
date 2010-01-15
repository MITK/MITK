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

#include "cherryCommandContributionItem.h"

#include "cherryIMenuItem.h"
#include "cherryIToolItem.h"
#include "cherryIMenuItemListener.h"
#include "cherryIToolItemListener.h"
#include "cherryIMenuListener.h"
#include "cherryIMenuService.h"
#include "cherryICommandService.h"
#include "cherryICommandImageService.h"
#include "../handlers/cherryIHandlerService.h"
#include "../tweaklets/cherryGuiWidgetsTweaklet.h"
#include "cherryIElementReference.h"
#include <cherryICommandListener.h>
#include <cherryCommand.h>
#include <cherryCommandEvent.h>
#include <cherryIEvaluationContext.h>

#include "../guitk/cherryGuiTkEvent.h"
#include <cherryParameterizedCommand.h>
#include <cherryCommandExceptions.h>
#include "../cherryImageDescriptor.h"
#include "../cherryDisplay.h"
#include "../cherryAsyncRunnable.h"
#include "../services/cherryIServiceLocator.h"
#include "cherryCommandContributionItemParameter.h"

#include "../internal/cherryWorkbenchPlugin.h"


namespace cherry
{

int CommandContributionItem::MODE_FORCE_TEXT = 1;

CommandContributionItem::CommandUIElement::CommandUIElement(
    CommandContributionItem* item, SmartPointer<IServiceLocator> serviceLocator) :
  UIElement(serviceLocator), item(item)
{
}

void CommandContributionItem::CommandUIElement::SetChecked(bool checked)
{
  item->SetChecked(checked);
}

bool CommandContributionItem::CommandUIElement::GetChecked() const
{
  return item->checkedState;
}

void CommandContributionItem::CommandUIElement::SetDisabledIcon(SmartPointer<
    ImageDescriptor> desc)
{
  item->SetDisabledIcon(desc);
}

void CommandContributionItem::CommandUIElement::SetHoverIcon(SmartPointer<
    ImageDescriptor> desc)
{
  item->SetHoverIcon(desc);
}

void CommandContributionItem::CommandUIElement::SetIcon(SmartPointer<
    ImageDescriptor> desc)
{
  item->SetIcon(desc);
}

void CommandContributionItem::CommandUIElement::SetText(const std::string& text)
{
  item->SetText(text);
}

void CommandContributionItem::CommandUIElement::SetToolTip(
    const std::string& text)
{
  item->SetToolTip(text);
}

void CommandContributionItem::CommandUIElement::SetDropDownId(
    const std::string& id)
{
  item->dropDownMenuOverride = id;
}

void CommandContributionItem::CommandUIElement::SetEnabled(bool enabled)
{

}

bool CommandContributionItem::CommandUIElement::GetEnabled() const
{
  return item->IsEnabled();
}

void CommandContributionItem::CommandUIElement::Dispose()
{
  item->Dispose();
}

bool CommandContributionItem::CommandUIElement::IsDisposed() const
{
  return item->widget->IsDisposed();
}

CommandContributionItem::MyCommandListener::MyCommandListener(
    CommandContributionItem* item) :
  item(item)
{
}

void CommandContributionItem::MyCommandListener::CommandChanged(
    const SmartPointer<const CommandEvent> commandEvent)
{
  if (commandEvent->IsHandledChanged() || commandEvent->IsEnabledChanged()
      || commandEvent->IsDefinedChanged())
  {
    item->UpdateCommandProperties(commandEvent);
  }
}

CommandContributionItem::CommandUIElementListener::CommandUIElementListener(
    CommandContributionItem* item) :
  item(item)
{

}

void CommandContributionItem::CommandUIElementListener::UIElementDisposed(
    UIElement* element)
{
  item->HandleWidgetDispose(element);
}

void CommandContributionItem::CommandUIElementListener::UIElementSelected(
    SmartPointer<UIElement> element)
{
  item->HandleWidgetSelection(element);
}

CommandContributionItem::CommandContributionItem(
    const CommandContributionItemParameter& contributionParameters) :
  ContributionItem(contributionParameters.id)
{
}

CommandContributionItem::CommandContributionItem(
    const IServiceLocator::Pointer serviceLocator, const std::string& id,
    const std::string& commandId,
    const std::map<std::string, Object::Pointer>& parameters, SmartPointer<
        ImageDescriptor> icon, SmartPointer<ImageDescriptor> disabledIcon,
    SmartPointer<ImageDescriptor> hoverIcon, const std::string& label,
    const std::string& mnemonic, const std::string& tooltip, UIElement::Styles style) :
  ContributionItem(id)
{
  this->Init(CommandContributionItemParameter(serviceLocator, id, commandId,
      parameters, icon, disabledIcon, hoverIcon, label, mnemonic, tooltip,
      style, "", false));
}

void CommandContributionItem::Init(
    const CommandContributionItemParameter& contributionParameters)
{
  this->icon = contributionParameters.icon;
  this->disabledIcon = contributionParameters.disabledIcon;
  this->hoverIcon = contributionParameters.hoverIcon;
  this->label = contributionParameters.label;
  this->mnemonic = contributionParameters.mnemonic;
  this->tooltip = contributionParameters.tooltip;
  this->style = contributionParameters.style;
  this->helpContextId = contributionParameters.helpContextId;
  this->visibleEnabled = contributionParameters.visibleEnabled;
  this->mode = contributionParameters.mode;

  menuService = contributionParameters.serviceLocator ->GetService(
      IMenuService::GetManifestName()).Cast<IMenuService> ();
  commandService = contributionParameters.serviceLocator ->GetService(
      ICommandService::GetManifestName()).Cast<ICommandService> ();
  handlerService = contributionParameters.serviceLocator ->GetService(
      IHandlerService::GetManifestName()).Cast<IHandlerService> ();
  //    bindingService = (IBindingService) contributionParameters.serviceLocator
  //        .getService(IBindingService.class);

  this->CreateCommand(contributionParameters.commandId,
      contributionParameters.parameters);

  if (command)
  {
    try
    {
      UIElement::Pointer callback(new CommandUIElement(this,
          contributionParameters.serviceLocator));

      elementRef = commandService->RegisterElementForCommand(command, callback);
      command->GetCommand()->AddCommandListener(this->GetCommandListener());
      this->SetImages(contributionParameters.serviceLocator,
          contributionParameters.iconStyle);

      if (contributionParameters.helpContextId.empty())
      {
        try
        {
          this->helpContextId = commandService ->GetHelpContextId(
              contributionParameters.commandId);
        } catch (const NotDefinedException* e)
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
    } catch (const NotDefinedException* e)
    {
      WorkbenchPlugin::Log("Unable to register menu item \"" + this->GetId() //$NON-NLS-1$
          + "\", command \"" + contributionParameters.commandId
          + "\" not defined"); //$NON-NLS-1$ //$NON-NLS-2$
    }
  }
}

void CommandContributionItem::Fill(SmartPointer<IMenu> parent, int index)
{
  if (!command)
  {
    return;
  }
  if (widget || !parent)
  {
    return;
  }

  // Menus don't support the pulldown style
  UIElement::Styles tmpStyle = style;
  if (tmpStyle == UIElement::STYLE_PULLDOWN)
    tmpStyle = UIElement::STYLE_PUSH;

  IMenuItem::Pointer item;
  if (index >= 0)
  {
    item = Tweaklets::Get(GuiWidgetsTweaklet::KEY)->CreateMenuItem(parent,
        tmpStyle, index);
  }
  else
  {
    item = Tweaklets::Get(GuiWidgetsTweaklet::KEY)->CreateMenuItem(parent,
        tmpStyle);
  }
  item->SetData(Object::Pointer(this));
  //  if (workbenchHelpSystem != null)
  //  {
  //    workbenchHelpSystem.setHelp(item, helpContextId);
  //  }
  item->AddListener(this->GetItemListener());
  widget = item;

  this->Update();
  this->UpdateIcons();
}

void CommandContributionItem::Fill(SmartPointer<IToolBar> parent, int index)
{
  if (!command)
  {
    return;
  }
  if (widget || !parent)
  {
    return;
  }

  IToolItem::Pointer item;
  if (index >= 0)
  {
    item = Tweaklets::Get(GuiWidgetsTweaklet::KEY)->CreateToolItem(parent,
        style, index);
  }
  else
  {
    item = Tweaklets::Get(GuiWidgetsTweaklet::KEY)->CreateToolItem(parent,
        style);
  }

  item->SetData(Object::Pointer(this));

  item->AddListener(this->GetItemListener());
  widget = item;

  this->Update();
  this->UpdateIcons();
}

void CommandContributionItem::Update(const std::string& id)
{
  if (widget)
  {
    if (IMenuItem::Pointer item = widget.Cast<IMenuItem>())
    {
      std::string text(label);
      if (text.empty())
      {
        if (command)
        {
          try
          {
            text = command->GetCommand()->GetName();
          } catch (const NotDefinedException& e)
          {
            WorkbenchPlugin::Log("Update item failed " + this->GetId(), e);
          }
        }
      }
      text = this->UpdateMnemonic(text);

      //TODO Command key binding
      //        std::string keyBindingText;
      //        if (command) {
      //          TriggerSequence binding = bindingService
      //              .getBestActiveBindingFor(command);
      //          if (binding != null) {
      //            keyBindingText = binding.format();
      //          }
      //        }
      //        if (text != null) {
      //          if (keyBindingText == null) {
      //            item.setText(text);
      //          } else {
      //            item.setText(text + '\t' + keyBindingText);
      //          }
      //        }

      if (item->GetChecked() != checkedState)
      {
        item->SetChecked(checkedState);
      }

      bool shouldBeEnabled = this->IsEnabled();
      if (item->GetEnabled() != shouldBeEnabled)
      {
        item->SetEnabled(shouldBeEnabled);
      }
    }
    else if (IToolItem::Pointer item = widget.Cast<IToolItem>())
    {
      std::string text = label;
      if (text.empty())
      {
        if (command)
        {
          try
          {
            text = command->GetCommand()->GetName();
          } catch (const NotDefinedException& e)
          {
            WorkbenchPlugin::Log("Update item failed " //$NON-NLS-1$
                + GetId(), e);
          }
        }
      }

      if ((!icon || (mode & MODE_FORCE_TEXT) == MODE_FORCE_TEXT))
      {
        item->SetText(text);
      }

      if (!tooltip.empty())
        item->SetToolTip(tooltip);
      else
      {
        item->SetToolTip(text);
      }

      if (item->GetChecked() != checkedState)
      {
        item->SetChecked(checkedState);
      }

      bool shouldBeEnabled = this->IsEnabled();
      if (item->GetEnabled() != shouldBeEnabled)
      {
        item->SetEnabled(shouldBeEnabled);
      }
    }
  }
}

void CommandContributionItem::Dispose()
{
  if (widget)
  {
    //widget.dispose();
    widget = 0;
  }
  if (elementRef)
  {
    commandService->UnregisterElement(elementRef);
    elementRef = 0;
  }
  if (commandListener)
  {
    command->GetCommand()->RemoveCommandListener(commandListener);
    commandListener = 0;
  }
  command = 0;
  commandService = 0;
  //TODO Command key binding
  //bindingService = 0;
  menuService = 0;
  handlerService = 0;
  //this->DisposeOldImages();

  ContributionItem::Dispose();
}

CommandContributionItem::~CommandContributionItem()
{
  CommandContributionItem::Dispose();
}

bool CommandContributionItem::IsEnabled()
{
  if (command)
  {
    command->GetCommand()->SetEnabled(menuService->GetCurrentState());
    return command->GetCommand()->IsEnabled();
  }
  return false;
}

bool CommandContributionItem::IsVisible()
{
  if (visibleEnabled)
  {
    return ContributionItem::IsVisible() && this->IsEnabled();
  }
  return ContributionItem::IsVisible();
}

void CommandContributionItem::SetImages(const IServiceLocator::Pointer locator,
    const std::string& iconStyle)
{
  if (!icon)
  {
    ICommandImageService::Pointer service(locator ->GetService(
        ICommandImageService::GetManifestName()).Cast<ICommandImageService> ());
    icon = service->GetImageDescriptor(command->GetId(),
        ICommandImageService::TYPE_DEFAULT, iconStyle);
    disabledIcon = service->GetImageDescriptor(command->GetId(),
        ICommandImageService::TYPE_DISABLED, iconStyle);
    hoverIcon = service->GetImageDescriptor(command->GetId(),
        ICommandImageService::TYPE_HOVER, iconStyle);
  }
}

SmartPointer<ICommandListener> CommandContributionItem::GetCommandListener()
{
  if (!commandListener)
  {
    commandListener = new MyCommandListener(this);
  }
  return commandListener;
}

void CommandContributionItem::UpdateCommandProperties(const SmartPointer<
    const CommandEvent> commandEvent)
{
  if (commandEvent->IsHandledChanged())
  {
    dropDownMenuOverride = "";
  }
  if (!widget || widget->IsDisposed())
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
          IContributionManager::Pointer parent = this->GetParent();
          if (parent)
          {
            parent->Update(true);
          }
        }
      }
}

SmartPointer<ParameterizedCommand> CommandContributionItem::GetCommand()
{
  return command;
}

void CommandContributionItem::CreateCommand(const std::string& commandId,
    const std::map<std::string, Object::Pointer>& parameters)
{
  if (commandId.empty())
  {
    WorkbenchPlugin::Log("Unable to create menu item \"" + this->GetId() //$NON-NLS-1$
        + "\", no command id"); //$NON-NLS-1$
    return;
  }
  Command::Pointer cmd = commandService->GetCommand(commandId);
  if (!cmd->IsDefined())
  {
    WorkbenchPlugin::Log("Unable to create menu item \"" + this->GetId() //$NON-NLS-1$
        + "\", command \"" + commandId + "\" not defined"); //$NON-NLS-1$ //$NON-NLS-2$
    return;
  }
  command = ParameterizedCommand::GenerateCommand(cmd, parameters);
}

std::string CommandContributionItem::UpdateMnemonic(const std::string& s)
{
  if (mnemonic.empty() || s.empty())
  {
    return s;
  }
  std::string::size_type idx = s.find_first_of(mnemonic);
  if (idx == std::string::npos)
  {
    return s;
  }

  return s.substr(0, idx) + '&' + s.substr(idx);
}

void CommandContributionItem::HandleWidgetDispose(UIElement* widget)
{
  if (this->widget == widget)
  {
    widget->RemoveListener(this->GetItemListener());
    widget = 0;
    //disposeOldImages();
  }
}

SmartPointer<IUIElementListener> CommandContributionItem::GetItemListener()
{
  if (!itemListener)
  {
    itemListener = new CommandUIElementListener(this);
  }
  return itemListener;
}

void CommandContributionItem::HandleWidgetSelection(UIElement::Pointer element)
{
  // Special check for ToolBar dropdowns...
  //TODO Tool item drop down menu contributions
  //if (this->OpenDropDownMenu(event))
  //return;

  if ((style & (UIElement::STYLE_TOGGLE | UIElement::STYLE_CHECK)) != 0)
  {
    checkedState = element->GetChecked();
  }

  try
  {
    handlerService->ExecuteCommand(command, element);
  } catch (const ExecutionException& e)
  {
    WorkbenchPlugin::Log("Failed to execute item " //$NON-NLS-1$
        + GetId(), e);
  } catch (const NotDefinedException& e)
  {
    WorkbenchPlugin::Log("Failed to execute item " //$NON-NLS-1$
        + GetId(), e);
  } catch (const NotEnabledException& e)
  {
    WorkbenchPlugin::Log("Failed to execute item " //$NON-NLS-1$
        + GetId(), e);
  } catch (const NotHandledException& e)
  {
    WorkbenchPlugin::Log("Failed to execute item " //$NON-NLS-1$
        + GetId(), e);
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

void CommandContributionItem::SetIcon(SmartPointer<ImageDescriptor> desc)
{
  icon = desc;
  this->UpdateIcons();
}

void CommandContributionItem::UpdateIcons()
{
  widget->SetDisabledIcon(disabledIcon);
  widget->SetHoverIcon(hoverIcon);
  widget->SetIcon(icon);
}

void CommandContributionItem::SetText(const std::string& text)
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
  widget->SetChecked(checkedState);

}

void CommandContributionItem::SetToolTip(const std::string& text)
{
  tooltip = text;
  widget->SetToolTip(text);
}

void CommandContributionItem::SetDisabledIcon(
    SmartPointer<ImageDescriptor> desc)
{
  disabledIcon = desc;
  this->UpdateIcons();
}

void CommandContributionItem::SetHoverIcon(SmartPointer<ImageDescriptor> desc)
{
  hoverIcon = desc;
  this->UpdateIcons();
}

}
