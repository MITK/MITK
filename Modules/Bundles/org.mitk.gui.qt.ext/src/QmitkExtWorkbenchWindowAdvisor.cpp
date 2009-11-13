/*=========================================================================

 Program:   Medical Imaging & Interaction Toolkit
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

#include "QmitkExtWorkbenchWindowAdvisor.h"
#include "QmitkExtActionBarAdvisor.h"

#include <QMenu>
#include <QMenuBar>
#include <QMainWindow>
#include <QStatusBar>

#include <cherryPlatform.h>
#include <cherryPlatformUI.h>
#include <cherryIWorkbenchWindow.h>
#include <cherryIWorkbenchPage.h>
#include <cherryIPreferencesService.h>
#include <cherryIPerspectiveRegistry.h>
#include <cherryIPerspectiveDescriptor.h>
#include <cherryIWorkbenchPartConstants.h>

#include <internal/cherryQtShowViewAction.h>
#include <internal/cherryQtOpenPerspectiveAction.h>

#include <QmitkFileOpenAction.h>
#include <QmitkFileSaveProjectAction.h>
#include <QmitkFileExitAction.h>
#include <QmitkStatusBar.h>
#include <QmitkProgressBar.h>
#include <QmitkMemoryUsageIndicatorView.h>
#include <QmitkPreferencesDialog.h>

#include <itkConfigure.h>
#include <vtkConfigure.h>

// UGLYYY
#include "internal/QmitkExtWorkbenchWindowAdvisorHack.h"
#include "mitkUndoController.h"
#include "mitkVerboseLimitedLinearUndo.h"
#include <QToolBar>
#include <QMessageBox>

QmitkExtWorkbenchWindowAdvisorHack
    * QmitkExtWorkbenchWindowAdvisorHack::undohack =
        new QmitkExtWorkbenchWindowAdvisorHack();

class PartListenerForTitle: public cherry::IPartListener
{
public:

  PartListenerForTitle(QmitkExtWorkbenchWindowAdvisor* wa) :
    windowAdvisor(wa)
  {
  }

  Events::Types GetPartEventTypes() const
  {
    return Events::ACTIVATED | Events::BROUGHT_TO_TOP | Events::CLOSED
        | Events::HIDDEN | Events::VISIBLE;
  }

  void PartActivated(cherry::IWorkbenchPartReference::Pointer ref)
  {
    if (ref.Cast<cherry::IEditorReference> ())
    {
      windowAdvisor->UpdateTitle(false);
    }
  }

  void PartBroughtToTop(cherry::IWorkbenchPartReference::Pointer ref)
  {
    if (ref.Cast<cherry::IEditorReference> ())
    {
      windowAdvisor->UpdateTitle(false);
    }
  }

  void PartClosed(cherry::IWorkbenchPartReference::Pointer ref)
  {
    windowAdvisor->UpdateTitle(false);
  }

  void PartHidden(cherry::IWorkbenchPartReference::Pointer ref)
  {
    if (!windowAdvisor->lastActiveEditor.Expired() &&
        ref->GetPart(false) == windowAdvisor->lastActiveEditor.Lock())
    {
      windowAdvisor->UpdateTitle(true);
    }
  }

  void PartVisible(cherry::IWorkbenchPartReference::Pointer ref)
  {
    if (!windowAdvisor->lastActiveEditor.Expired() &&
        ref->GetPart(false) == windowAdvisor->lastActiveEditor.Lock())
    {
      windowAdvisor->UpdateTitle(false);
    }
  }

private:
  QmitkExtWorkbenchWindowAdvisor* windowAdvisor;

};

class PerspectiveListenerForTitle: public cherry::IPerspectiveListener
{
public:

  PerspectiveListenerForTitle(QmitkExtWorkbenchWindowAdvisor* wa) :
    windowAdvisor(wa)
  {
  }

  Events::Types GetPerspectiveEventTypes() const
  {
    return Events::ACTIVATED | Events::SAVED_AS | Events::DEACTIVATED;
  }

  void PerspectiveActivated(cherry::IWorkbenchPage::Pointer page,
      cherry::IPerspectiveDescriptor::Pointer perspective)
  {
    windowAdvisor->UpdateTitle(false);
  }

  void PerspectiveSavedAs(cherry::IWorkbenchPage::Pointer page,
      cherry::IPerspectiveDescriptor::Pointer oldPerspective,
      cherry::IPerspectiveDescriptor::Pointer newPerspective)
  {
    windowAdvisor->UpdateTitle(false);
  }

  void PerspectiveDeactivated(cherry::IWorkbenchPage::Pointer page,
      cherry::IPerspectiveDescriptor::Pointer perspective)
  {
    windowAdvisor->UpdateTitle(false);
  }

private:
  QmitkExtWorkbenchWindowAdvisor* windowAdvisor;
};


QmitkExtWorkbenchWindowAdvisor::QmitkExtWorkbenchWindowAdvisor(
    cherry::WorkbenchAdvisor* wbAdvisor,
    cherry::IWorkbenchWindowConfigurer::Pointer configurer) :
  cherry::WorkbenchWindowAdvisor(configurer),
  lastInput(0),
  wbAdvisor(wbAdvisor),
  showViewToolbar(true)
{

}

cherry::ActionBarAdvisor::Pointer QmitkExtWorkbenchWindowAdvisor::CreateActionBarAdvisor(
    cherry::IActionBarConfigurer::Pointer configurer)
{
  cherry::ActionBarAdvisor::Pointer actionBarAdvisor(
      new QmitkExtActionBarAdvisor(configurer));
  return actionBarAdvisor;
}

void QmitkExtWorkbenchWindowAdvisor::ShowViewToolbar(bool show)
{
  showViewToolbar = show;
}

void QmitkExtWorkbenchWindowAdvisor::PostWindowCreate()
{
  // very bad hack...
  cherry::IWorkbenchWindow::Pointer window =
      this->GetWindowConfigurer()->GetWindow();
  QMainWindow* mainWindow =
      static_cast<QMainWindow*> (window->GetShell()->GetControl());

  // ==== Application menu ============================
  QMenuBar* menuBar = mainWindow->menuBar();

  QMenu* fileMenu = menuBar->addMenu("&File");

  fileMenu->addAction(new QmitkFileOpenAction(window));
  fileMenu->addAction(new QmitkFileSaveProjectAction(window));
  fileMenu->addSeparator();
  fileMenu->addAction(new QmitkFileExitAction(window));

  cherry::IViewRegistry* viewRegistry =
      cherry::PlatformUI::GetWorkbench()->GetViewRegistry();
  const std::vector<cherry::IViewDescriptor::Pointer>& viewDescriptors =
      viewRegistry->GetViews();

  // another bad hack to get an edit/undo menu...
  QMenu* editMenu = menuBar->addMenu("&Edit");
  QAction* undoAction = editMenu->addAction("&Undo",
      QmitkExtWorkbenchWindowAdvisorHack::undohack, SLOT(onUndo()),
      QKeySequence("CTRL+Z"));
  QAction* redoAction = editMenu->addAction("&Redo",
      QmitkExtWorkbenchWindowAdvisorHack::undohack, SLOT(onRedo()),
      QKeySequence("CTRL+Y"));

  // ==== Window Menu ==========================
  QMenu* windowMenu = menuBar->addMenu("Window");
  QAction* newWindowAction = windowMenu->addAction("&New Window",
      QmitkExtWorkbenchWindowAdvisorHack::undohack, SLOT(onNewWindow()));
  windowMenu->addSeparator();
  QMenu* perspMenu = windowMenu->addMenu("&Open Perspective");
  QMenu* viewMenu = windowMenu->addMenu("Show &View");
  windowMenu->addSeparator();
  QAction* resetPerspectiveAction = windowMenu->addAction("&Reset Perspective",
      QmitkExtWorkbenchWindowAdvisorHack::undohack, SLOT(onResetPerspective()));
  QAction* closePerspectiveAction = windowMenu->addAction("&Close Perspective",
      QmitkExtWorkbenchWindowAdvisorHack::undohack, SLOT(onClosePerspective()));
  windowMenu->addSeparator();
  QAction* preferencesAction = windowMenu->addAction("&Preferences...",
      QmitkExtWorkbenchWindowAdvisorHack::undohack, SLOT(onEditPreferences()),
      QKeySequence("CTRL+P"));

  // fill perspective menu
  cherry::IPerspectiveRegistry* perspRegistry =
      window->GetWorkbench()->GetPerspectiveRegistry();
  QActionGroup* perspGroup = new QActionGroup(menuBar);

  std::vector<cherry::IPerspectiveDescriptor::Pointer> perspectives(
      perspRegistry->GetPerspectives());
  for (std::vector<cherry::IPerspectiveDescriptor::Pointer>::iterator perspIt =
      perspectives.begin(); perspIt != perspectives.end(); ++perspIt)
  {
    QAction* perspAction = new cherry::QtOpenPerspectiveAction(window,
        *perspIt, perspGroup);
  }
  perspMenu->addActions(perspGroup->actions());

  // sort elements (converting vector to map...)
  std::vector<cherry::IViewDescriptor::Pointer>::const_iterator iter;
  std::map<std::string, cherry::IViewDescriptor::Pointer> VDMap;

  for (iter = viewDescriptors.begin(); iter != viewDescriptors.end(); ++iter)
  {
    if ((*iter)->GetId() == "org.opencherry.ui.internal.introview")
      continue;
    std::pair<std::string, cherry::IViewDescriptor::Pointer> p(
        (*iter)->GetLabel(), (*iter));
    VDMap.insert(p);
  }
  // ==================================================

  // ==== View Toolbar ==================================
  QToolBar* qToolbar = new QToolBar;

  std::map<std::string, cherry::IViewDescriptor::Pointer>::const_iterator
      MapIter;
  for (MapIter = VDMap.begin(); MapIter != VDMap.end(); ++MapIter)
  {
    cherry::QtShowViewAction* viewAction = new cherry::QtShowViewAction(window,
        (*MapIter).second);
    //m_ViewActions.push_back(viewAction);
    viewMenu->addAction(viewAction);
    if (showViewToolbar)
    {
      qToolbar->addAction(viewAction);
    }
  }

  if (showViewToolbar)
  {
    mainWindow->addToolBar(qToolbar);
  }
  else
    delete qToolbar;
  // ====================================================

  // ===== Help menu ====================================
  QMenu* helpMenu = menuBar->addMenu("Help");
  QAction* welcomeAction = helpMenu->addAction("&Welcome",QmitkExtWorkbenchWindowAdvisorHack::undohack, SLOT(onIntro()));
  QAction* helpAction = helpMenu->addAction("&Active Bundle (F1)",QmitkExtWorkbenchWindowAdvisorHack::undohack, SLOT(onHelp()));
  QAction* aboutAction = helpMenu->addAction("&About",QmitkExtWorkbenchWindowAdvisorHack::undohack, SLOT(onIntro()));
  // =====================================================


  QStatusBar* qStatusBar = new QStatusBar();

  //creating a QmitkStatusBar for Output on the QStatusBar and connecting it with the MainStatusBar
  QmitkStatusBar *statusBar = new QmitkStatusBar(qStatusBar);
  //disabling the SizeGrip in the lower right corner
  statusBar->SetSizeGripEnabled(false);

  QmitkProgressBar *progBar = new QmitkProgressBar();
  qStatusBar->addPermanentWidget(progBar, 0);
  progBar->hide();
  mainWindow->setStatusBar(qStatusBar);

  QmitkMemoryUsageIndicatorView* memoryIndicator =
      new QmitkMemoryUsageIndicatorView();
  qStatusBar->addPermanentWidget(memoryIndicator, 0);
}

void QmitkExtWorkbenchWindowAdvisor::PreWindowOpen()
{
  cherry::IWorkbenchWindowConfigurer::Pointer configurer = GetWindowConfigurer();

  // show the shortcut bar and progress indicator, which are hidden by
  // default
  configurer->SetShowPerspectiveBar(true);
  //configurer->SetShowFastViewBars(true);
  //configurer->SetShowProgressIndicator(true);

//  // add the drag and drop support for the editor area
//  configurer.addEditorAreaTransfer(EditorInputTransfer.getInstance());
//  configurer.addEditorAreaTransfer(ResourceTransfer.getInstance());
//  configurer.addEditorAreaTransfer(FileTransfer.getInstance());
//  configurer.addEditorAreaTransfer(MarkerTransfer.getInstance());
//  configurer.configureEditorAreaDropListener(new EditorAreaDropAdapter(
//      configurer.getWindow()));

  this->HookTitleUpdateListeners(configurer);
}

//--------------------------------------------------------------------------------
// Ugly hack from here on. Feel free to delete when command framework
// and undo buttons are done.
//--------------------------------------------------------------------------------

QmitkExtWorkbenchWindowAdvisorHack::QmitkExtWorkbenchWindowAdvisorHack() :
  QObject()
{
}

void QmitkExtWorkbenchWindowAdvisorHack::onUndo()
{
  mitk::UndoModel* model = mitk::UndoController::GetCurrentUndoModel();
  if (model)
  {
    if (mitk::VerboseLimitedLinearUndo* verboseundo = dynamic_cast<mitk::VerboseLimitedLinearUndo*>( model ))
    {
      mitk::VerboseLimitedLinearUndo::StackDescription descriptions =
          verboseundo->GetUndoDescriptions();
      if (descriptions.size() >= 1)
      {
        LOG_INFO << "Undo " << descriptions.front().second;
      }
    }
    model->Undo();
  }
  else
  {
    LOG_ERROR << "No undo model instantiated";
  }
}

void QmitkExtWorkbenchWindowAdvisorHack::onRedo()
{
  mitk::UndoModel* model = mitk::UndoController::GetCurrentUndoModel();
  if (model)
  {
    if (mitk::VerboseLimitedLinearUndo* verboseundo = dynamic_cast<mitk::VerboseLimitedLinearUndo*>( model ))
    {
      mitk::VerboseLimitedLinearUndo::StackDescription descriptions =
          verboseundo->GetRedoDescriptions();
      if (descriptions.size() >= 1)
      {
        LOG_INFO << "Redo " << descriptions.front().second;
      }
    }
    model->Redo();
  }
  else
  {
    LOG_ERROR << "No undo model instantiated";
  }
}

void QmitkExtWorkbenchWindowAdvisorHack::onEditPreferences()
{
  QmitkPreferencesDialog _PreferencesDialog(QApplication::activeWindow());
  _PreferencesDialog.exec();
}

void QmitkExtWorkbenchWindowAdvisorHack::onQuit()
{
  cherry::PlatformUI::GetWorkbench()->Close();
}

void QmitkExtWorkbenchWindowAdvisorHack::onResetPerspective()
{
  cherry::PlatformUI::GetWorkbench()->GetActiveWorkbenchWindow()->GetActivePage()->ResetPerspective();
}

void QmitkExtWorkbenchWindowAdvisorHack::onClosePerspective()
{
  cherry::IWorkbenchPage::Pointer
      page =
          cherry::PlatformUI::GetWorkbench()->GetActiveWorkbenchWindow()->GetActivePage();
  page->ClosePerspective(page->GetPerspective(), true, true);
}

void QmitkExtWorkbenchWindowAdvisorHack::onNewWindow()
{
  cherry::PlatformUI::GetWorkbench()->OpenWorkbenchWindow(0);
}

void QmitkExtWorkbenchWindowAdvisorHack::onIntro()
{
  bool hasIntro =
      cherry::PlatformUI::GetWorkbench()->GetIntroManager()->HasIntro();
  if (!hasIntro)
  {
    QMessageBox::information(0, "No Welcome Content Found",
        "There is no welcome content suitable for display in this application.");
  }
  else
  {
    cherry::PlatformUI::GetWorkbench()->GetIntroManager()->ShowIntro(
        cherry::PlatformUI::GetWorkbench()->GetActiveWorkbenchWindow(), false);
  }
}

void QmitkExtWorkbenchWindowAdvisorHack::onHelp()
	{
	//call help method here
	}

void QmitkExtWorkbenchWindowAdvisor::HookTitleUpdateListeners(
    cherry::IWorkbenchWindowConfigurer::Pointer configurer)
{
  // hook up the listeners to update the window title
  titlePartListener = new PartListenerForTitle(this);
  titlePerspectiveListener = new PerspectiveListenerForTitle(this);
  editorPropertyListener = new cherry::PropertyChangeIntAdapter<
      QmitkExtWorkbenchWindowAdvisor>(this,
      &QmitkExtWorkbenchWindowAdvisor::PropertyChange);

  //    configurer.getWindow().addPageListener(new IPageListener() {
  //      public void pageActivated(IWorkbenchPage page) {
  //        updateTitle(false);
  //      }
  //
  //      public void pageClosed(IWorkbenchPage page) {
  //        updateTitle(false);
  //      }
  //
  //      public void pageOpened(IWorkbenchPage page) {
  //        // do nothing
  //      }
  //    });

  configurer->GetWindow()->AddPerspectiveListener(titlePerspectiveListener);
  configurer->GetWindow()->GetPartService()->AddPartListener(titlePartListener);
}

std::string QmitkExtWorkbenchWindowAdvisor::ComputeTitle()
{
  cherry::IWorkbenchWindowConfigurer::Pointer configurer =
      GetWindowConfigurer();
  cherry::IWorkbenchPage::Pointer currentPage =
      configurer->GetWindow()->GetActivePage();
  cherry::IEditorPart::Pointer activeEditor;
  if (currentPage)
  {
    activeEditor = lastActiveEditor.Lock();
  }

  std::string title;
  //TODO Product
  //    IProduct product = Platform.getProduct();
  //    if (product != null) {
  //      title = product.getName();
  //    }
  // instead of the product name, we use the executable name for now
  title
      = cherry::Platform::GetConfiguration().getString("application.baseName");

  // add version informatioin
  QString mitkRevision("$Rev$");
  mitkRevision.replace( QRegExp("[^0-9]+(\\d+).*"), "\\1");
  QString versions = QString(" (ITK %1.%2.%3  VTK %4.%5.%6 Qt %7 MITK %8)")
    .arg(ITK_VERSION_MAJOR).arg(ITK_VERSION_MINOR).arg(ITK_VERSION_PATCH)
    .arg(VTK_MAJOR_VERSION).arg(VTK_MINOR_VERSION).arg(VTK_BUILD_VERSION)
    .arg(QT_VERSION_STR)
    .arg(mitkRevision);

  title += versions.toStdString();

  if (currentPage)
  {
    if (activeEditor)
    {
      lastEditorTitle = activeEditor->GetTitleToolTip();
      if (!lastEditorTitle.empty())
        title = lastEditorTitle + " - " + title;
    }
    cherry::IPerspectiveDescriptor::Pointer persp =
        currentPage->GetPerspective();
    std::string label = "";
    if (persp)
    {
      label = persp->GetLabel();
    }
    cherry::IAdaptable* input = currentPage->GetInput();
    if (input && input != wbAdvisor->GetDefaultPageInput())
    {
      label = currentPage->GetLabel();
    }
    if (!label.empty())
    {
      title = label + " - " + title;
    }
  }

  return title;
}

void QmitkExtWorkbenchWindowAdvisor::RecomputeTitle()
{
  cherry::IWorkbenchWindowConfigurer::Pointer configurer =
      GetWindowConfigurer();
  std::string oldTitle = configurer->GetTitle();
  std::string newTitle = ComputeTitle();
  if (newTitle != oldTitle)
  {
    configurer->SetTitle(newTitle);
  }
}

void QmitkExtWorkbenchWindowAdvisor::UpdateTitle(bool editorHidden)
{
  cherry::IWorkbenchWindowConfigurer::Pointer configurer =
      GetWindowConfigurer();
  cherry::IWorkbenchWindow::Pointer window = configurer->GetWindow();
  cherry::IEditorPart::Pointer activeEditor;
  cherry::IWorkbenchPage::Pointer currentPage = window->GetActivePage();
  cherry::IPerspectiveDescriptor::Pointer persp;
  cherry::IAdaptable* input = 0;

  if (currentPage)
  {
    activeEditor = currentPage->GetActiveEditor();
    persp = currentPage->GetPerspective();
    input = currentPage->GetInput();
  }

  if (editorHidden)
  {
    activeEditor = 0;
  }

  // Nothing to do if the editor hasn't changed
  if (activeEditor == lastActiveEditor.Lock() && currentPage == lastActivePage.Lock()
      && persp == lastPerspective.Lock() && input == lastInput)
  {
    return;
  }

  if (!lastActiveEditor.Expired())
  {
    lastActiveEditor.Lock()->RemovePropertyListener(editorPropertyListener);
  }

  lastActiveEditor = activeEditor;
  lastActivePage = currentPage;
  lastPerspective = persp;
  lastInput = input;

  if (activeEditor)
  {
    activeEditor->AddPropertyListener(editorPropertyListener);
  }

  RecomputeTitle();
}

void QmitkExtWorkbenchWindowAdvisor::PropertyChange(cherry::Object::Pointer source, int propId)
  {
    if (propId == cherry::IWorkbenchPartConstants::PROP_TITLE)
    {
      if (!lastActiveEditor.Expired())
      {
        std::string newTitle = lastActiveEditor.Lock()->GetPartName();
        if (lastEditorTitle != newTitle)
        {
          RecomputeTitle();
        }
      }
    }
  }