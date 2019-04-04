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

#include "berryWorkbenchSourceProvider.h"

#include "berryIContextService.h"
#include "berryIEditorSite.h"
#include "berryIEvaluationContext.h"
#include "berryIPartService.h"
#include "berryISelectionService.h"
#include "berryIShowInSource.h"
#include "berryISources.h"
#include "berryIWorkbench.h"
#include "berryIWorkbenchLocationService.h"
#include "berryIWorkbenchPartConstants.h"

#include "berryDetachedWindow.h"
#include "berryDisplay.h"
#include "berryObjectString.h"
#include "berryObjects.h"
#include "berryShowInContext.h"
#include "berryUiUtil.h"
#include "berryWorkbench.h"
#include "berryWorkbenchWindow.h"

#include <QApplication>
#include <QWidget>

namespace berry {

const QList<QString> WorkbenchSourceProvider::PROVIDED_SOURCE_NAMES = QList<QString>()
    << ISources::ACTIVE_CURRENT_SELECTION_NAME()
    << ISources::ACTIVE_EDITOR_ID_NAME()
    << ISources::ACTIVE_EDITOR_NAME()
    << ISources::ACTIVE_PART_ID_NAME()
    << ISources::ACTIVE_PART_NAME()
    << ISources::ACTIVE_SITE_NAME()
    << ISources::SHOW_IN_SELECTION()
    << ISources::SHOW_IN_INPUT()
    << ISources::ACTIVE_SHELL_NAME()
    << ISources::ACTIVE_WORKBENCH_WINDOW_NAME()
    << ISources::ACTIVE_WORKBENCH_WINDOW_SHELL_NAME()
    << ISources::ACTIVE_WORKBENCH_WINDOW_IS_TOOLBAR_VISIBLE_NAME()
    << ISources::ACTIVE_WORKBENCH_WINDOW_IS_PERSPECTIVEBAR_VISIBLE_NAME()
    << ISources::ACTIVE_WORKBENCH_WINDOW_IS_STATUS_LINE_VISIBLE_NAME()
    << ISources::ACTIVE_WORKBENCH_WINDOW_ACTIVE_PERSPECTIVE_NAME();

WorkbenchSourceProvider::WorkbenchSourceProvider()
  : workbench(nullptr)
  , lastActiveEditor(nullptr)
  , lastActivePart(nullptr)
  , lastActivePartSite(nullptr)
  , lastShowInInput(nullptr)
  , display(nullptr)
{
}


void WorkbenchSourceProvider::Initialize(IServiceLocator* locator)
{
  AbstractSourceProvider::Initialize(locator);
//    this.locator = locator;
  IWorkbenchLocationService* wls = locator->GetService<IWorkbenchLocationService>();
  workbench = wls->GetWorkbench();
  workbench->AddWindowListener(this);
  lastWindow = workbench->GetActiveWorkbenchWindow();
  display = workbench->GetDisplay();
  qApp->installEventFilter(this);
}

WorkbenchSourceProvider::~WorkbenchSourceProvider()
{
  if (!lastWindow.Expired())
  {
    lastWindow.Lock()->GetSelectionService()->RemoveSelectionListener(this);
  }
  workbench->RemoveWindowListener(this);
  qApp->removeEventFilter(this);
  HookListener(const_cast<WorkbenchWindow*>(lastActiveWorkbenchWindow.Lock().GetPointer()), nullptr);
  lastActiveWorkbenchWindow.Reset();
  lastActiveWorkbenchWindowShell.Reset();
  lastActiveShell.Reset();
  lastWindow.Reset();
}

QList<QString> WorkbenchSourceProvider::GetProvidedSourceNames() const
{
  return PROVIDED_SOURCE_NAMES;
}

ISourceProvider::StateMapType WorkbenchSourceProvider::GetCurrentState() const
{
  ISourceProvider::StateMapType currentState;

  UpdateActiveShell(currentState);
  UpdateActivePart(currentState);
  UpdateSelection(currentState);

  return currentState;
}

void WorkbenchSourceProvider::SelectionChanged(const SmartPointer<IWorkbenchPart>& /*part*/,
                                               const SmartPointer<const ISelection>& newSelection)
{

  if (selection == newSelection)
    return; // we have already handled the change

  selection = newSelection;

  LogDebuggingInfo(QString("Selection changed to ") + (selection ? selection->ToString() : QString("nullptr")));

  FireSourceChanged(ISources::ACTIVE_CURRENT_SELECTION(),
                    ISources::ACTIVE_CURRENT_SELECTION_NAME(), selection);
}

int WorkbenchSourceProvider::UpdateSelection(ISourceProvider::StateMapType& currentState) const
{
  int sources = 0;
  currentState.insert(ISources::ACTIVE_CURRENT_SELECTION_NAME(),
                      IEvaluationContext::UNDEFINED_VARIABLE);
  Object::ConstPointer object = currentState.value(ISources::ACTIVE_PART_NAME());
  if (IWorkbenchPart::ConstPointer part = object.Cast<const IWorkbenchPart>())
  {
    if (part->GetSite().IsNotNull() && part->GetSite()->GetSelectionProvider().IsNotNull())
    {
      sources = ISources::ACTIVE_CURRENT_SELECTION();
      ISelection::ConstPointer currentSelection = part->GetSite()
          ->GetSelectionProvider()->GetSelection();
      currentState.insert(ISources::ACTIVE_CURRENT_SELECTION_NAME(), currentSelection);
    }
  }
  return sources;
}

void WorkbenchSourceProvider::UpdateWindows(IWorkbenchWindow* newWindow)
{
  if (lastWindow == newWindow)
  {
    return;
  }

  ISelection::ConstPointer selection;
  if (!lastWindow.Expired())
  {
    lastWindow.Lock()->GetSelectionService()->RemoveSelectionListener(this);
  }
  if (newWindow != nullptr)
  {
    newWindow->GetSelectionService()->AddSelectionListener(this);
    selection = newWindow->GetSelectionService()->GetSelection();
  }
  SelectionChanged(IWorkbenchPart::Pointer(nullptr), selection);
  lastWindow = IWorkbenchWindow::Pointer(newWindow);
}

void WorkbenchSourceProvider::PartActivated(const SmartPointer<IWorkbenchPartReference>& )
{
  CheckActivePart();
}

void WorkbenchSourceProvider::PartBroughtToTop(const SmartPointer<IWorkbenchPartReference>& )
{
  CheckActivePart();
}

void WorkbenchSourceProvider::PartClosed(const SmartPointer<IWorkbenchPartReference>& )
{
  CheckActivePart();
}

void WorkbenchSourceProvider::PartDeactivated(const SmartPointer<IWorkbenchPartReference>& )
{
  CheckActivePart();
}

void WorkbenchSourceProvider::PartOpened(const SmartPointer<IWorkbenchPartReference>& )
{
  CheckActivePart();
}

void WorkbenchSourceProvider::WindowActivated(const SmartPointer<IWorkbenchWindow>& )
{
  CheckActivePart();
}

void WorkbenchSourceProvider::WindowClosed(const SmartPointer<IWorkbenchWindow>& window)
{
  if (window.IsNotNull())
  {
    window->GetPartService()->RemovePartListener(this);
  }
  CheckActivePart();
}

void WorkbenchSourceProvider::WindowDeactivated(const SmartPointer<IWorkbenchWindow>& )
{
  CheckActivePart();
}

void WorkbenchSourceProvider::WindowOpened(const SmartPointer<IWorkbenchWindow>& window)
{
  if (window.IsNotNull())
  {
    window->GetPartService()->AddPartListener(this);
  }
}

void WorkbenchSourceProvider::HandleCheck(const SmartPointer<const Shell>& s)
{
  if (s != lastActiveShell.Lock())
  {
    lastActiveShell = s;
    CheckActivePart();
    IWorkbenchWindow* window = nullptr;
    if (s.IsNotNull())
    {
      if (s->GetData().Cast<WorkbenchWindow>())
      {
        window = s->GetData().Cast<IWorkbenchWindow>().GetPointer();
      }
      else if (DetachedWindow::Pointer dw = s->GetData().Cast<DetachedWindow>())
      {
        window = dw->GetWorkbenchPage()->GetWorkbenchWindow().GetPointer();
      }
    }
    UpdateWindows(window);
  }
}

void WorkbenchSourceProvider::CheckActivePart()
{
  CheckActivePart(false);
}

void WorkbenchSourceProvider::CheckActivePart(bool updateShowInSelection)
{
  ISourceProvider::StateMapType currentState;
  UpdateActivePart(currentState, updateShowInSelection);

  int sources = 0;

  // Figure out what was changed.
  const Object::ConstPointer newActivePart = currentState.value(ISources::ACTIVE_PART_NAME());
  if (newActivePart != lastActivePart)
  {
    sources |= ISources::ACTIVE_PART();
    if (newActivePart != IEvaluationContext::UNDEFINED_VARIABLE)
    {
      lastActivePart = newActivePart.Cast<const IWorkbenchPart>().GetPointer();
    }
    else
    {
      lastActivePart = nullptr;
    }
  }
  const Object::ConstPointer newActivePartId = currentState.value(ISources::ACTIVE_PART_ID_NAME());
  if (lastActivePartId != newActivePartId)
  {
    sources |= ISources::ACTIVE_PART_ID();
    if (newActivePartId != IEvaluationContext::UNDEFINED_VARIABLE)
    {
      lastActivePartId = newActivePartId.Cast<const ObjectString>();
    }
    else
    {
      lastActivePartId = nullptr;
    }
  }
  const Object::ConstPointer newActivePartSite = currentState.value(ISources::ACTIVE_SITE_NAME());
  if (newActivePartSite != lastActivePartSite)
  {
    sources |= ISources::ACTIVE_SITE();
    if (newActivePartSite != IEvaluationContext::UNDEFINED_VARIABLE)
    {
      lastActivePartSite = newActivePartSite.Cast<const IWorkbenchPartSite>().GetPointer();
    }
    else
    {
      lastActivePartSite = nullptr;
    }
  }
  const Object::ConstPointer newShowInInput = currentState.value(ISources::SHOW_IN_INPUT());
  if (newShowInInput != lastShowInInput)
  {
    sources |= ISources::ACTIVE_SITE();
    lastShowInInput = newShowInInput.GetPointer();
  }
  if (currentState.value(ISources::SHOW_IN_SELECTION()) != IEvaluationContext::UNDEFINED_VARIABLE)
  {
    sources |= ISources::ACTIVE_SITE();
  }
  Object::ConstPointer newActiveEditor = currentState.value(ISources::ACTIVE_EDITOR_NAME());
  if (newActiveEditor != lastActiveEditor)
  {
    sources |= ISources::ACTIVE_EDITOR();
    newActiveEditor = (newActiveEditor == IEvaluationContext::UNDEFINED_VARIABLE ? Object::ConstPointer(nullptr)
                                                                                 : newActiveEditor);
    HookListener(const_cast<IEditorPart*>(lastActiveEditor), const_cast<IEditorPart*>(newActiveEditor.Cast<const IEditorPart>().GetPointer()));
    lastActiveEditor = newActiveEditor.Cast<const IEditorPart>().GetPointer();
  }
  const Object::ConstPointer newEditorInput = currentState.value(ISources::ACTIVE_EDITOR_INPUT_NAME());
  IEditorInput::ConstPointer lastInput;
  if (!lastEditorInput.Expired())
  {
    lastInput = lastEditorInput.Lock();
  }
  if (newEditorInput != lastInput)
  {
    sources |= ISources::ACTIVE_EDITOR();
    if (newEditorInput != IEvaluationContext::UNDEFINED_VARIABLE)
    {
      lastEditorInput = newEditorInput.Cast<const IEditorInput>();
    }
    else
    {
      lastEditorInput.Reset();
    }
  }

  const Object::ConstPointer newActiveEditorId = currentState.value(ISources::ACTIVE_EDITOR_ID_NAME());
  if (newActiveEditorId != lastActiveEditorId)
  {
    sources |= ISources::ACTIVE_EDITOR_ID();
    if (newActiveEditorId != IEvaluationContext::UNDEFINED_VARIABLE)
    {
      lastActiveEditorId = newActiveEditorId.Cast<const ObjectString>();
    }
    else
    {
      lastActiveEditorId = nullptr;
    }
  }

  // Fire the event, if something has changed.
  if (sources != 0)
  {
    if (DEBUG)
    {
      if ((sources & ISources::ACTIVE_PART()) != 0)
      {
        LogDebuggingInfo("Active part changed to " + (lastActivePart ? lastActivePart->ToString() : QString("nullptr")));
      }
      if ((sources & ISources::ACTIVE_PART_ID()) != 0)
      {
        LogDebuggingInfo("Active part id changed to " + (lastActivePartId ? lastActivePartId->ToString() : QString("nullptr")));
      }
      if ((sources & ISources::ACTIVE_SITE()) != 0)
      {
        LogDebuggingInfo("Active site changed to " + (lastActivePartSite ? lastActivePartSite->ToString() : QString("nullptr")));
      }
      if ((sources & ISources::ACTIVE_EDITOR()) != 0)
      {
        LogDebuggingInfo("Active editor changed to " + (lastActiveEditor ? lastActiveEditor->ToString() : QString("nullptr")));
      }
      if ((sources & ISources::ACTIVE_EDITOR_ID()) != 0)
      {
        LogDebuggingInfo("Active editor id changed to " + (lastActiveEditorId ? lastActiveEditorId->ToString() : QString("nullptr")));
      }
    }
    sources |= UpdateSelection(currentState);
    FireSourceChanged(sources, currentState);
  }
}

SmartPointer<IShowInSource> WorkbenchSourceProvider::GetShowInSource(const SmartPointer<IWorkbenchPart>& sourcePart) const
{
  IShowInSource::Pointer result(UiUtil::GetAdapter<IShowInSource>(sourcePart.GetPointer()));
  return result;
}

SmartPointer<ShowInContext> WorkbenchSourceProvider::GetContext(const SmartPointer<IWorkbenchPart>& sourcePart) const
{
  IShowInSource::Pointer source = GetShowInSource(sourcePart);
  if (source.IsNotNull())
  {
    ShowInContext::Pointer context = source->GetShowInContext();
    if (context.IsNotNull())
    {
      return context;
    }
  }
  else if (IEditorPart::Pointer editorPart = sourcePart.Cast<IEditorPart>())
  {
    Object::Pointer input = editorPart->GetEditorInput();
    ISelectionProvider::Pointer sp = sourcePart->GetSite()->GetSelectionProvider();
    ISelection::ConstPointer sel = sp.IsNull() ? ISelection::ConstPointer(nullptr) : sp->GetSelection();
    ShowInContext::Pointer context(new ShowInContext(input, sel));
    return context;
  }
  return ShowInContext::Pointer(nullptr);
}

IWorkbenchWindow* WorkbenchSourceProvider::GetActiveWindow() const
{
//  IContextService* const contextService = workbench->GetService<IContextService>();
//  if (contextService != nullptr)
//  {
//    const int shellType = contextService->GetShellType(qApp->activeWindow());
//    if (shellType != IContextService::TYPE_DIALOG)
//    {
//      return workbench->GetActiveWorkbenchWindow().GetPointer();
//    }
//  }
  if (qApp->activeWindow() != qApp->activeModalWidget())
  {
    return workbench->GetActiveWorkbenchWindow().GetPointer();
  }
  return nullptr;
}

void WorkbenchSourceProvider::UpdateActivePart(ISourceProvider::StateMapType& currentState) const
{
  UpdateActivePart(currentState, false);
}

void WorkbenchSourceProvider::UpdateActivePart(ISourceProvider::StateMapType& currentState, bool updateShowInSelection) const
{
  currentState.insert(ISources::ACTIVE_SITE_NAME(), IEvaluationContext::UNDEFINED_VARIABLE);
  currentState.insert(ISources::ACTIVE_PART_NAME(), IEvaluationContext::UNDEFINED_VARIABLE);
  currentState.insert(ISources::ACTIVE_PART_ID_NAME(), IEvaluationContext::UNDEFINED_VARIABLE);
  currentState.insert(ISources::ACTIVE_EDITOR_NAME(), IEvaluationContext::UNDEFINED_VARIABLE);
  currentState.insert(ISources::ACTIVE_EDITOR_ID_NAME(), IEvaluationContext::UNDEFINED_VARIABLE);
  currentState.insert(ISources::SHOW_IN_INPUT(), IEvaluationContext::UNDEFINED_VARIABLE);
  currentState.insert(ISources::SHOW_IN_SELECTION(), IEvaluationContext::UNDEFINED_VARIABLE);

  IWorkbenchWindow* const activeWorkbenchWindow = GetActiveWindow();
  if (activeWorkbenchWindow != nullptr)
  {
    const IWorkbenchPage::Pointer activeWorkbenchPage = activeWorkbenchWindow->GetActivePage();
    if (activeWorkbenchPage.IsNotNull())
    {
      // Check the active workbench part.
      const IWorkbenchPart::Pointer newActivePart = activeWorkbenchPage->GetActivePart();
      currentState.insert(ISources::ACTIVE_PART_NAME(), newActivePart);
      if (newActivePart.IsNotNull())
      {
        const IWorkbenchPartSite::Pointer activeWorkbenchPartSite = newActivePart->GetSite();
        currentState.insert(ISources::ACTIVE_SITE_NAME(), activeWorkbenchPartSite);
        if (activeWorkbenchPartSite.IsNotNull())
        {
          const Object::Pointer newActivePartId(new ObjectString(activeWorkbenchPartSite->GetId()));
          currentState.insert(ISources::ACTIVE_PART_ID_NAME(), newActivePartId);
        }
        ShowInContext::Pointer context = GetContext(newActivePart);
        if (context.IsNotNull())
        {
          const Object::Pointer input = context->GetInput();
          if (input.IsNotNull())
          {
            currentState.insert(ISources::SHOW_IN_INPUT(), input);
          }
          if (updateShowInSelection)
          {
            const ISelection::ConstPointer selection = context->GetSelection();
            if (selection.IsNotNull())
            {
              currentState.insert(ISources::SHOW_IN_SELECTION(), selection);
            }
          }
        }
      }

      // Check the active editor part.
      const IEditorPart::Pointer newActiveEditor = activeWorkbenchPage->GetActiveEditor();
      currentState.insert(ISources::ACTIVE_EDITOR_NAME(), newActiveEditor);
      if (newActiveEditor.IsNotNull())
      {
        currentState.insert(ISources::ACTIVE_EDITOR_INPUT_NAME(),
                            newActiveEditor->GetEditorInput());
        const IEditorSite::Pointer activeEditorSite = newActiveEditor->GetEditorSite();
        if (activeEditorSite.IsNotNull())
        {
          const Object::Pointer newActiveEditorId(new ObjectString(activeEditorSite->GetId()));
          currentState.insert(ISources::ACTIVE_EDITOR_ID_NAME(), newActiveEditorId);
        }
      }
    }
  }

}

/**
 * The listener to individual window properties.
 */
void WorkbenchSourceProvider::PropertyChange(const SmartPointer<PropertyChangeEvent>& event)
{
    if (WorkbenchWindow::PROP_TOOLBAR_VISIBLE == event->GetProperty())
    {
      const Object::Pointer newValue = event->GetNewValue();
      if (newValue.IsNull() || !(newValue.Cast<ObjectBool>()))
        return;
      if (lastToolbarVisibility != newValue)
      {
        FireSourceChanged(
              ISources::ACTIVE_WORKBENCH_WINDOW_SUBORDINATE(),
              ISources::ACTIVE_WORKBENCH_WINDOW_IS_TOOLBAR_VISIBLE_NAME(),
              newValue);
        lastToolbarVisibility = newValue.Cast<ObjectBool>();
      }
    }
    else if (WorkbenchWindow::PROP_PERSPECTIVEBAR_VISIBLE == event->GetProperty())
    {
      const Object::Pointer newValue = event->GetNewValue();
      if (newValue.IsNull() || !(newValue.Cast<ObjectBool>()))
        return;
      if (lastPerspectiveBarVisibility != newValue)
      {
        FireSourceChanged(
              ISources::ACTIVE_WORKBENCH_WINDOW_SUBORDINATE(),
              ISources::ACTIVE_WORKBENCH_WINDOW_IS_PERSPECTIVEBAR_VISIBLE_NAME(),
              newValue);
        lastPerspectiveBarVisibility = newValue.Cast<ObjectBool>();
      }
    }
    else if (WorkbenchWindow::PROP_STATUS_LINE_VISIBLE == event->GetProperty())
    {
      const Object::Pointer newValue = event->GetNewValue();
      if (newValue.IsNull() || !(newValue.Cast<ObjectBool>()))
        return;
      if (lastStatusLineVisibility != newValue)
      {
        FireSourceChanged(
              ISources::ACTIVE_WORKBENCH_WINDOW_SUBORDINATE(),
              ISources::ACTIVE_WORKBENCH_WINDOW_IS_STATUS_LINE_VISIBLE_NAME(),
              newValue);
        lastStatusLineVisibility = newValue.Cast<ObjectBool>();
      }
    }
    else
    {
      IPropertyChangeListener::PropertyChange(event);
    }
  }

void WorkbenchSourceProvider::PerspectiveActivated(const SmartPointer<IWorkbenchPage>& /*page*/,
                          const SmartPointer<IPerspectiveDescriptor>& perspective)
{
  QString id = perspective.IsNull() ? QString() : perspective->GetId();
  if (lastPerspectiveId.IsNotNull() && *lastPerspectiveId == id)
  {
    return;
  }

  Object::Pointer newValue(new ObjectString(id));
  FireSourceChanged(ISources::ACTIVE_WORKBENCH_WINDOW_SUBORDINATE(),
                    ISources::ACTIVE_WORKBENCH_WINDOW_ACTIVE_PERSPECTIVE_NAME(),
                    newValue);

  lastPerspectiveId = new ObjectString(id);
}

void WorkbenchSourceProvider::PerspectiveChanged(const SmartPointer<IWorkbenchPage>& /*page*/,
                        const SmartPointer<IPerspectiveDescriptor>& /*perspective*/,
                        const QString& /*changeId*/)
{
}

void WorkbenchSourceProvider::PropertyChange(const Object::Pointer& source, int propId)
{
  if (propId == IWorkbenchPartConstants::PROP_INPUT)
  {
    HandleInputChanged(source.Cast<IEditorPart>());
  }
}

bool WorkbenchSourceProvider::eventFilter(QObject* /*obj*/, QEvent *event)
{
  if (event->type() == QEvent::WindowActivate)
    HandleShellEvent();
  return false;
}

void WorkbenchSourceProvider::HandleShellEvent()
{
//  if (!(event.widget instanceof Shell))
//  {
//    if (DEBUG)
//    {
//      logDebuggingInfo("WSP: passOnEvent: " + event.widget); //$NON-NLS-1$
//    }
//    return;
//  }

  LogDebuggingInfo("\tWSP:lastActiveShell: " +
                   (!lastActiveShell.Expired() ? lastActiveShell.Lock()->GetControl()->objectName() : QString("nullptr")));
  LogDebuggingInfo("\tWSP:lastActiveWorkbenchWindowShell: " +
                   (!lastActiveWorkbenchWindowShell.Expired() ? lastActiveWorkbenchWindowShell.Lock()->GetControl()->objectName() : QString("nullptr")));

  const ISourceProvider::StateMapType currentState = GetCurrentState();
  const Shell::ConstPointer newActiveShell = currentState.value(ISources::ACTIVE_SHELL_NAME()).Cast<const Shell>();
  const WorkbenchWindow::ConstPointer newActiveWorkbenchWindow = currentState.value(ISources::ACTIVE_WORKBENCH_WINDOW_NAME()).Cast<const WorkbenchWindow>();
  const Shell::ConstPointer newActiveWorkbenchWindowShell = currentState.value(ISources::ACTIVE_WORKBENCH_WINDOW_SHELL_NAME()).Cast<const Shell>();

  // dont update the coolbar/perspective bar visibility unless we're
  // processing a workbench window change
  const ObjectBool::ConstPointer newToolbarVisibility(
        newActiveWorkbenchWindow.IsNull() ? lastToolbarVisibility.GetPointer()
                                          : (newActiveWorkbenchWindow->GetToolBarVisible() ?
                                               new ObjectBool(true) : new ObjectBool(false)));
  const ObjectBool::ConstPointer newPerspectiveBarVisibility(
        newActiveWorkbenchWindow.IsNull() ? lastPerspectiveBarVisibility.GetPointer()
                                          : (newActiveWorkbenchWindow->GetPerspectiveBarVisible() ?
                                               new ObjectBool(true) : new ObjectBool(false)));
  const ObjectBool::ConstPointer newStatusLineVis(
        newActiveWorkbenchWindow.IsNull() ? lastStatusLineVisibility.GetPointer()
                                          : (newActiveWorkbenchWindow->GetStatusLineVisible() ?
                                               new ObjectBool(true) : new ObjectBool(false)));

  ObjectString::ConstPointer perspectiveId = lastPerspectiveId;
  if (newActiveWorkbenchWindow.IsNotNull())
  {
    IWorkbenchPage::Pointer activePage = newActiveWorkbenchWindow->GetActivePage();
    if (activePage.IsNotNull())
    {
      IPerspectiveDescriptor::Pointer perspective = activePage->GetPerspective();
      if (perspective.IsNotNull())
      {
        perspectiveId = new ObjectString(perspective->GetId());
      }
    }
  }

  // Figure out which variables have changed.
  const bool shellChanged = newActiveShell != lastActiveShell.Lock();
  const bool windowChanged = newActiveWorkbenchWindowShell != lastActiveWorkbenchWindowShell.Lock();
  const bool toolbarChanged = newToolbarVisibility != lastToolbarVisibility;
  const bool statusLineChanged = newStatusLineVis != lastStatusLineVisibility;

  const bool perspectiveBarChanged = newPerspectiveBarVisibility != lastPerspectiveBarVisibility;
  const bool perspectiveIdChanged = lastPerspectiveId != perspectiveId;
  // Fire an event for those sources that have changed.
  if (shellChanged && windowChanged)
  {
    ISourceProvider::StateMapType sourceValuesByName;
    sourceValuesByName.insert(ISources::ACTIVE_SHELL_NAME(), newActiveShell);
    sourceValuesByName.insert(ISources::ACTIVE_WORKBENCH_WINDOW_NAME(), newActiveWorkbenchWindow);
    sourceValuesByName.insert(ISources::ACTIVE_WORKBENCH_WINDOW_SHELL_NAME(), newActiveWorkbenchWindowShell);
    int sourceFlags = ISources::ACTIVE_SHELL() | ISources::ACTIVE_WORKBENCH_WINDOW();

    if (toolbarChanged)
    {
      sourceValuesByName.insert(ISources::ACTIVE_WORKBENCH_WINDOW_IS_TOOLBAR_VISIBLE_NAME(),
                                newToolbarVisibility);
      sourceFlags |= ISources::ACTIVE_WORKBENCH_WINDOW_SUBORDINATE();
    }
    if (statusLineChanged)
    {
      sourceValuesByName.insert(ISources::ACTIVE_WORKBENCH_WINDOW_IS_STATUS_LINE_VISIBLE_NAME(),
                                newStatusLineVis);
      sourceFlags |= ISources::ACTIVE_WORKBENCH_WINDOW_SUBORDINATE();
    }
    if (perspectiveBarChanged)
    {
      sourceValuesByName.insert(ISources::ACTIVE_WORKBENCH_WINDOW_IS_PERSPECTIVEBAR_VISIBLE_NAME(),
                                newPerspectiveBarVisibility);
      sourceFlags |= ISources::ACTIVE_WORKBENCH_WINDOW_SUBORDINATE();
    }
    if (perspectiveIdChanged)
    {
      sourceValuesByName.insert(ISources::ACTIVE_WORKBENCH_WINDOW_ACTIVE_PERSPECTIVE_NAME(),
                                perspectiveId);
      sourceFlags |= ISources::ACTIVE_WORKBENCH_WINDOW_SUBORDINATE();
    }

    if (DEBUG)
    {
      LogDebuggingInfo("Active shell changed to " + (newActiveShell ? newActiveShell->ToString() : QString("nullptr")));
      LogDebuggingInfo("Active workbench window changed to " + (newActiveWorkbenchWindow ?  newActiveWorkbenchWindow->ToString() : QString("nullptr")));
      LogDebuggingInfo("Active workbench window shell changed to " + (newActiveWorkbenchWindowShell ? newActiveWorkbenchWindowShell->ToString() : QString("nullptr")));
      LogDebuggingInfo("Active workbench window coolbar visibility " + (newToolbarVisibility ? newToolbarVisibility->ToString() : QString("nullptr")));
      LogDebuggingInfo("Active workbench window perspective bar visibility " + (newPerspectiveBarVisibility ? newPerspectiveBarVisibility->ToString() : QString("nullptr")));
      LogDebuggingInfo("Active workbench window status line visibility " + (newStatusLineVis ? newStatusLineVis->ToString() : QString("nullptr")));
    }

    FireSourceChanged(sourceFlags, sourceValuesByName);
    HookListener(const_cast<WorkbenchWindow*>(lastActiveWorkbenchWindow.Lock().GetPointer()),
                 const_cast<WorkbenchWindow*>(newActiveWorkbenchWindow.GetPointer()));

  }
  else if (shellChanged)
  {
    LogDebuggingInfo("Active shell changed to " + (newActiveShell ? newActiveShell->ToString() : QString("nullptr")));
    FireSourceChanged(ISources::ACTIVE_SHELL(), ISources::ACTIVE_SHELL_NAME(), newActiveShell);
  }
  else if (windowChanged)
  {
    ISourceProvider::StateMapType sourceValuesByName;
    sourceValuesByName.insert(ISources::ACTIVE_WORKBENCH_WINDOW_NAME(),
                              newActiveWorkbenchWindow);
    sourceValuesByName.insert(ISources::ACTIVE_WORKBENCH_WINDOW_SHELL_NAME(),
                              newActiveWorkbenchWindowShell);

    int sourceFlags = ISources::ACTIVE_SHELL() | ISources::ACTIVE_WORKBENCH_WINDOW();

    if (toolbarChanged)
    {
      sourceValuesByName.insert(ISources::ACTIVE_WORKBENCH_WINDOW_IS_TOOLBAR_VISIBLE_NAME(),
                                newToolbarVisibility);
      sourceFlags |= ISources::ACTIVE_WORKBENCH_WINDOW_SUBORDINATE();
    }
    if (statusLineChanged)
    {
      sourceValuesByName.insert(ISources::ACTIVE_WORKBENCH_WINDOW_IS_STATUS_LINE_VISIBLE_NAME(),
                                newStatusLineVis);
      sourceFlags |= ISources::ACTIVE_WORKBENCH_WINDOW_SUBORDINATE();
    }
    if (perspectiveBarChanged)
    {
      sourceValuesByName.insert(ISources::ACTIVE_WORKBENCH_WINDOW_IS_PERSPECTIVEBAR_VISIBLE_NAME(),
                                newPerspectiveBarVisibility);
      sourceFlags |= ISources::ACTIVE_WORKBENCH_WINDOW_SUBORDINATE();
    }
    if (perspectiveIdChanged)
    {
      sourceValuesByName.insert(ISources::ACTIVE_WORKBENCH_WINDOW_ACTIVE_PERSPECTIVE_NAME(),
                                perspectiveId);
      sourceFlags |= ISources::ACTIVE_WORKBENCH_WINDOW_SUBORDINATE();
    }

    if (DEBUG)
    {
      LogDebuggingInfo("Active workbench window changed to "  + (newActiveWorkbenchWindow ? newActiveWorkbenchWindow->ToString() : QString("nullptr")));
      LogDebuggingInfo("Active workbench window shell changed to " + (newActiveWorkbenchWindowShell ? newActiveWorkbenchWindowShell->ToString() : QString("nullptr")));
      LogDebuggingInfo("Active workbench window coolbar visibility " + (newToolbarVisibility ? newToolbarVisibility->ToString() : QString("nullptr")));
      LogDebuggingInfo("Active workbench window perspective bar visibility " + (newPerspectiveBarVisibility ? newPerspectiveBarVisibility->ToString() : QString("nullptr")));
      LogDebuggingInfo("Active workbench window status line visibility " + (newStatusLineVis ? newStatusLineVis->ToString() : QString("nullptr")));
    }

    FireSourceChanged(sourceFlags, sourceValuesByName);
    HookListener(const_cast<WorkbenchWindow*>(lastActiveWorkbenchWindow.Lock().GetPointer()),
                 const_cast<WorkbenchWindow*>(newActiveWorkbenchWindow.GetPointer()));
  }

  if (shellChanged || windowChanged)
  {
    CheckOtherSources(newActiveShell);
  }

  // Update the member variables.
  lastActiveShell = newActiveShell;
  lastActiveWorkbenchWindowShell = newActiveWorkbenchWindowShell;
  lastActiveWorkbenchWindow = newActiveWorkbenchWindow;
  lastToolbarVisibility = newToolbarVisibility;
  lastStatusLineVisibility = newStatusLineVis;
  lastPerspectiveBarVisibility = newPerspectiveBarVisibility;
  lastPerspectiveId = perspectiveId;
}

void WorkbenchSourceProvider::CheckOtherSources(const SmartPointer<const Shell>& s)
{
  HandleCheck(s);
}

void WorkbenchSourceProvider::HandleInputChanged(const SmartPointer<IEditorPart>& editor)
{
  IEditorInput::Pointer newInput = editor->GetEditorInput();
  IEditorInput::ConstPointer lastInput;
  if (!lastEditorInput.Expired())
  {
    lastInput = lastEditorInput.Lock();
  }
  if (newInput != lastInput)
  {
    FireSourceChanged(ISources::ACTIVE_EDITOR(),
                      ISources::ACTIVE_EDITOR_INPUT_NAME(),
                      newInput.IsNull() ? IEvaluationContext::UNDEFINED_VARIABLE : Object::ConstPointer(newInput));
    lastEditorInput = newInput;
  }
}

void WorkbenchSourceProvider::HookListener(WorkbenchWindow* lastActiveWorkbenchWindow,
                  WorkbenchWindow* newActiveWorkbenchWindow)
{
  if (lastActiveWorkbenchWindow != nullptr)
  {
    lastActiveWorkbenchWindow->RemovePropertyChangeListener(this);
    lastActiveWorkbenchWindow->RemovePerspectiveListener(this);
  }

  if (newActiveWorkbenchWindow != nullptr)
  {
    newActiveWorkbenchWindow->AddPropertyChangeListener(this);
    newActiveWorkbenchWindow->AddPerspectiveListener(this);
  }
}

void WorkbenchSourceProvider::HookListener(IEditorPart* lastActiveEditor,
                                           IEditorPart* newActiveEditor)
{
  if (lastActiveEditor != nullptr)
  {
    lastActiveEditor->RemovePropertyListener(this);
  }
  if (newActiveEditor != nullptr)
  {
    newActiveEditor->AddPropertyListener(this);
  }
}

void WorkbenchSourceProvider::UpdateActiveShell(ISourceProvider::StateMapType& currentState) const
{
  QVariant windowShell;
  QWidget* activeWindow = qApp->activeWindow();
  if (activeWindow)
  {
    windowShell = activeWindow->property("shell");
  }
  Shell::Pointer newActiveShell(windowShell.isValid() ? windowShell.value<Shell*>() : nullptr);
  currentState.insert(ISources::ACTIVE_SHELL_NAME(), newActiveShell);

  /*
   * We will fallback to the workbench window, but only if a dialog is not
   * open.
   */
//  IContextService* const contextService = workbench->GetService<IContextService>();
//  const int shellType = contextService->GetShellType(newActiveShell);
//  if (shellType == IContextService::TYPE_DIALOG)
//    return;
  if (activeWindow == qApp->activeModalWidget()) return;

  const WorkbenchWindow::ConstPointer newActiveWorkbenchWindow =
      workbench->GetActiveWorkbenchWindow().Cast<WorkbenchWindow>();
  Shell::Pointer newActiveWorkbenchWindowShell;
  if (newActiveWorkbenchWindow.IsNotNull())
  {
    newActiveWorkbenchWindowShell = newActiveWorkbenchWindow->GetShell();
  }
  currentState.insert(ISources::ACTIVE_WORKBENCH_WINDOW_NAME(),
                      newActiveWorkbenchWindow);
  currentState.insert(ISources::ACTIVE_WORKBENCH_WINDOW_SHELL_NAME(),
                      newActiveWorkbenchWindowShell);

  const ObjectBool::ConstPointer newToolbarVisibility(
        newActiveWorkbenchWindow.IsNull() ? lastToolbarVisibility.GetPointer()
                                          : (newActiveWorkbenchWindow->GetToolBarVisible() ?
                                               new ObjectBool(true) : new ObjectBool(false)));
  const ObjectBool::ConstPointer newPerspectiveBarVisibility(
        newActiveWorkbenchWindow.IsNull() ? lastPerspectiveBarVisibility.GetPointer()
                                          : (newActiveWorkbenchWindow->GetPerspectiveBarVisible() ?
                                               new ObjectBool(true) : new ObjectBool(false)));
  const ObjectBool::ConstPointer newStatusLineVis(
        newActiveWorkbenchWindow.IsNull() ? lastStatusLineVisibility.GetPointer()
                                          : (newActiveWorkbenchWindow->GetStatusLineVisible() ?
                                               new ObjectBool(true) : new ObjectBool(false)));

  ObjectString::ConstPointer perspectiveId = lastPerspectiveId;
  if (newActiveWorkbenchWindow.IsNotNull())
  {
    const IWorkbenchPage::Pointer activePage = newActiveWorkbenchWindow->GetActivePage();
    if (activePage.IsNotNull())
    {
      const IPerspectiveDescriptor::Pointer perspective = activePage->GetPerspective();
      if (perspective.IsNotNull())
      {
        perspectiveId = new ObjectString(perspective->GetId());
      }
    }
  }

  currentState.insert(ISources::ACTIVE_WORKBENCH_WINDOW_IS_TOOLBAR_VISIBLE_NAME(),
                      newToolbarVisibility);

  currentState.insert(ISources::ACTIVE_WORKBENCH_WINDOW_IS_PERSPECTIVEBAR_VISIBLE_NAME(),
                      newPerspectiveBarVisibility);
  currentState.insert(ISources::ACTIVE_WORKBENCH_WINDOW_IS_STATUS_LINE_VISIBLE_NAME(),
                      newStatusLineVis);

  currentState.insert(ISources::ACTIVE_WORKBENCH_WINDOW_ACTIVE_PERSPECTIVE_NAME(),
                      perspectiveId);

}

IPartListener::Events::Types WorkbenchSourceProvider::GetPartEventTypes() const
{
  return IPartListener::Events::ACTIVATED | IPartListener::Events::BROUGHT_TO_TOP |
      IPartListener::Events::CLOSED | IPartListener::Events::DEACTIVATED | IPartListener::Events::OPENED;
}

IPerspectiveListener::Events::Types WorkbenchSourceProvider::GetPerspectiveEventTypes() const
{
  return IPerspectiveListener::Events::ACTIVATED | IPerspectiveListener::Events::CHANGED;
}

}
