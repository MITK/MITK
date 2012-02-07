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
#include <QString>
#include <QFile>
#include <QRegExp>
#include <QTextStream>
#include <QSettings>

#include <ctkPluginException.h>
#include <service/event/ctkEventAdmin.h>

#include <berryPlatform.h>
#include <berryPlatformUI.h>
#include <berryIWorkbenchWindow.h>
#include <berryIWorkbenchPage.h>
#include <berryIPreferencesService.h>
#include <berryIPerspectiveRegistry.h>
#include <berryIPerspectiveDescriptor.h>
#include <berryIWorkbenchPartConstants.h>

#include <internal/berryQtShowViewAction.h>
#include <internal/berryQtOpenPerspectiveAction.h>

#include <QmitkExtFileOpenAction.h>
#include <QmitkExtFileSaveProjectAction.h>
#include <QmitkFileExitAction.h>
#include <QmitkCloseProjectAction.h>
#include <QmitkStatusBar.h>
#include <QmitkProgressBar.h>
#include <QmitkMemoryUsageIndicatorView.h>
#include <QmitkPreferencesDialog.h>

#include <itkConfigure.h>
#include <vtkConfigure.h>
#include <mitkVersion.h>

// UGLYYY
#include "internal/QmitkExtWorkbenchWindowAdvisorHack.h"
#include "internal/QmitkCommonExtPlugin.h"
#include "mitkUndoController.h"
#include "mitkVerboseLimitedLinearUndo.h"
#include <QToolBar>
#include <QMessageBox>
#include <QLabel>
#include <QmitkAboutDialog/QmitkAboutDialog.h>

QmitkExtWorkbenchWindowAdvisorHack
* QmitkExtWorkbenchWindowAdvisorHack::undohack =
new QmitkExtWorkbenchWindowAdvisorHack();

QString QmitkExtWorkbenchWindowAdvisor::QT_SETTINGS_FILENAME = "QtSettings.ini";

class PartListenerForTitle: public berry::IPartListener
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

   void PartActivated(berry::IWorkbenchPartReference::Pointer ref)
   {
    if (ref.Cast<berry::IEditorReference> ())
    {
     windowAdvisor->UpdateTitle(false);
    }
   }

   void PartBroughtToTop(berry::IWorkbenchPartReference::Pointer ref)
   {
    if (ref.Cast<berry::IEditorReference> ())
    {
     windowAdvisor->UpdateTitle(false);
    }
   }

   void PartClosed(berry::IWorkbenchPartReference::Pointer /*ref*/)
   {
    windowAdvisor->UpdateTitle(false);
   }

   void PartHidden(berry::IWorkbenchPartReference::Pointer ref)
   {
    if (!windowAdvisor->lastActiveEditor.Expired() &&
     ref->GetPart(false) == windowAdvisor->lastActiveEditor.Lock())
    {
     windowAdvisor->UpdateTitle(true);
    }
   }

   void PartVisible(berry::IWorkbenchPartReference::Pointer ref)
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

class PartListenerForImageNavigator: public berry::IPartListener
{
public:

 PartListenerForImageNavigator(QAction* act) :
   imageNavigatorAction(act)
   {
   }

   Events::Types GetPartEventTypes() const
   {
    return Events::OPENED | Events::CLOSED | Events::HIDDEN |
     Events::VISIBLE;
   }

   void PartOpened(berry::IWorkbenchPartReference::Pointer ref)
   {
    if (ref->GetId()=="org.mitk.views.imagenavigator")
    {
     imageNavigatorAction->setChecked(true);
    }
   }

   void PartClosed(berry::IWorkbenchPartReference::Pointer ref)
   {
    if (ref->GetId()=="org.mitk.views.imagenavigator")
    {
     imageNavigatorAction->setChecked(false);
    }
   }

   void PartVisible(berry::IWorkbenchPartReference::Pointer ref)
   {
    if (ref->GetId()=="org.mitk.views.imagenavigator")
    {
     imageNavigatorAction->setChecked(true);
    }
   }

   void PartHidden(berry::IWorkbenchPartReference::Pointer ref)
   {
    if (ref->GetId()=="org.mitk.views.imagenavigator")
    {
     imageNavigatorAction->setChecked(false);
    }
   }

private:
 QAction* imageNavigatorAction;

};

class PerspectiveListenerForTitle: public berry::IPerspectiveListener
{
public:

 PerspectiveListenerForTitle(QmitkExtWorkbenchWindowAdvisor* wa) :
   windowAdvisor(wa), perspectivesClosed(false)
   {
   }

   Events::Types GetPerspectiveEventTypes() const
   {
    return Events::ACTIVATED | Events::SAVED_AS | Events::DEACTIVATED
     // remove the following line when command framework is finished
     | Events::CLOSED | Events::OPENED;
   }

   void PerspectiveActivated(berry::IWorkbenchPage::Pointer /*page*/,
    berry::IPerspectiveDescriptor::Pointer /*perspective*/)
   {
    windowAdvisor->UpdateTitle(false);
   }

   void PerspectiveSavedAs(berry::IWorkbenchPage::Pointer /*page*/,
    berry::IPerspectiveDescriptor::Pointer /*oldPerspective*/,
    berry::IPerspectiveDescriptor::Pointer /*newPerspective*/)
   {
    windowAdvisor->UpdateTitle(false);
   }

   void PerspectiveDeactivated(berry::IWorkbenchPage::Pointer /*page*/,
    berry::IPerspectiveDescriptor::Pointer /*perspective*/)
   {
    windowAdvisor->UpdateTitle(false);
   }

   void PerspectiveOpened(berry::IWorkbenchPage::Pointer /*page*/,
    berry::IPerspectiveDescriptor::Pointer /*perspective*/)
   {
    if (perspectivesClosed)
    {
     QListIterator<QAction*> i(windowAdvisor->viewActions);
     while (i.hasNext())
     {
      i.next()->setEnabled(true);
     }

     windowAdvisor->fileSaveProjectAction->setEnabled(true);
     windowAdvisor->closeProjectAction->setEnabled(true);
     windowAdvisor->undoAction->setEnabled(true);
     windowAdvisor->redoAction->setEnabled(true);
     windowAdvisor->imageNavigatorAction->setEnabled(true);
     windowAdvisor->resetPerspAction->setEnabled(true);
     if( windowAdvisor->GetShowClosePerspectiveMenuItem() )
     {
       windowAdvisor->closePerspAction->setEnabled(true);
     }   
    }

    perspectivesClosed = false;
   }

   void PerspectiveClosed(berry::IWorkbenchPage::Pointer /*page*/,
    berry::IPerspectiveDescriptor::Pointer /*perspective*/)
   {
    berry::IWorkbenchWindow::Pointer wnd = windowAdvisor->GetWindowConfigurer()->GetWindow();
    bool allClosed = true;
    if (wnd->GetActivePage())
    {
     std::vector<berry::IPerspectiveDescriptor::Pointer> perspectives(wnd->GetActivePage()->GetOpenPerspectives());
     allClosed = perspectives.empty();
    }

    if (allClosed)
    {
     perspectivesClosed = true;

     QListIterator<QAction*> i(windowAdvisor->viewActions);
     while (i.hasNext())
     {
      i.next()->setEnabled(false);
     }

     windowAdvisor->fileSaveProjectAction->setEnabled(false);
     windowAdvisor->closeProjectAction->setEnabled(false);
     windowAdvisor->undoAction->setEnabled(false);
     windowAdvisor->redoAction->setEnabled(false);
     windowAdvisor->imageNavigatorAction->setEnabled(false);
     windowAdvisor->resetPerspAction->setEnabled(false);
     if( windowAdvisor->GetShowClosePerspectiveMenuItem() )
     {
       windowAdvisor->closePerspAction->setEnabled(false);
     }     
    }
   }

private:
 QmitkExtWorkbenchWindowAdvisor* windowAdvisor;
 bool perspectivesClosed;
};

class PerspectiveListenerForMenu: public berry::IPerspectiveListener
{
public:

 PerspectiveListenerForMenu(QmitkExtWorkbenchWindowAdvisor* wa) :
   windowAdvisor(wa)
   {
   }

   Events::Types GetPerspectiveEventTypes() const
   {
    return Events::ACTIVATED | Events::DEACTIVATED;
   }

   void PerspectiveActivated(berry::IWorkbenchPage::Pointer /*page*/,
    berry::IPerspectiveDescriptor::Pointer perspective)
   {
     QAction* action = windowAdvisor->mapPerspIdToAction[perspective->GetId()];
     if (action)
     {
       action->setChecked(true);
     }
   }

   void PerspectiveDeactivated(berry::IWorkbenchPage::Pointer /*page*/,
    berry::IPerspectiveDescriptor::Pointer perspective)
   {
     QAction* action = windowAdvisor->mapPerspIdToAction[perspective->GetId()];
     if (action)
     {
       action->setChecked(false);
     }
   }

private:
 QmitkExtWorkbenchWindowAdvisor* windowAdvisor;
};



QmitkExtWorkbenchWindowAdvisor::QmitkExtWorkbenchWindowAdvisor(berry::WorkbenchAdvisor* wbAdvisor,
                  berry::IWorkbenchWindowConfigurer::Pointer configurer) :
QmitkCommonWorkbenchWindowAdvisor(configurer),
lastInput(0),
wbAdvisor(wbAdvisor),
showViewToolbar(true),
showPerspectiveToolbar(false),
showVersionInfo(true),
showMitkVersionInfo(true),
showViewMenuItem(true),
showNewWindowMenuItem(true),
showClosePerspectiveMenuItem(true)
{
 productName = berry::Platform::GetConfiguration().getString("application.baseName");
}

berry::ActionBarAdvisor::Pointer QmitkExtWorkbenchWindowAdvisor::CreateActionBarAdvisor(
 berry::IActionBarConfigurer::Pointer configurer)
{
 berry::ActionBarAdvisor::Pointer actionBarAdvisor(
  new QmitkExtActionBarAdvisor(configurer));
 return actionBarAdvisor;
}

void* QmitkExtWorkbenchWindowAdvisor::CreateEmptyWindowContents(void* parent)
{
 QWidget* parentWidget = static_cast<QWidget*>(parent);
 QLabel* label = new QLabel(parentWidget);
 label->setText("<b>No perspectives are open. Open a perspective in the <i>Window->Open Perspective</i> menu.</b>");
 label->setContentsMargins(10,10,10,10);
 label->setAlignment(Qt::AlignTop);
 label->setEnabled(false);
 parentWidget->layout()->addWidget(label);
 return label;
}

void QmitkExtWorkbenchWindowAdvisor::ShowClosePerspectiveMenuItem(bool show)
{
 showClosePerspectiveMenuItem = show;
}

bool QmitkExtWorkbenchWindowAdvisor::GetShowClosePerspectiveMenuItem()
{
  return showClosePerspectiveMenuItem;
}

void QmitkExtWorkbenchWindowAdvisor::ShowNewWindowMenuItem(bool show)
{
 showNewWindowMenuItem = show;
}

void QmitkExtWorkbenchWindowAdvisor::ShowViewToolbar(bool show)
{
 showViewToolbar = show;
}

void QmitkExtWorkbenchWindowAdvisor::ShowViewMenuItem(bool show)
{
 showViewMenuItem = show;
}

void QmitkExtWorkbenchWindowAdvisor::ShowPerspectiveToolbar(bool show)
{
 showPerspectiveToolbar = show;
}

void QmitkExtWorkbenchWindowAdvisor::ShowVersionInfo(bool show)
{
 showVersionInfo = show;
}

void QmitkExtWorkbenchWindowAdvisor::ShowMitkVersionInfo(bool show)
{
 showMitkVersionInfo = show;
}

void QmitkExtWorkbenchWindowAdvisor::SetProductName(const std::string& product)
{
 productName = product;
}

void QmitkExtWorkbenchWindowAdvisor::SetWindowIcon(const std::string& wndIcon)
{
 windowIcon = wndIcon;
}

void QmitkExtWorkbenchWindowAdvisor::PostWindowCreate()
{
 QmitkCommonWorkbenchWindowAdvisor::PostWindowCreate();
 // very bad hack...
 berry::IWorkbenchWindow::Pointer window =
  this->GetWindowConfigurer()->GetWindow();
 QMainWindow* mainWindow =
  static_cast<QMainWindow*> (window->GetShell()->GetControl());

 if (!windowIcon.empty())
 {
  mainWindow->setWindowIcon(QIcon(QString::fromStdString(windowIcon)));
 }
 mainWindow->setContextMenuPolicy(Qt::PreventContextMenu);

 /*mainWindow->setStyleSheet("color: white;"
 "background-color: #808080;"
 "selection-color: #659EC7;"
 "selection-background-color: #808080;"
 " QMenuBar {"
 "background-color: #808080; }");*/

 // ==== Application menu ============================
 QMenuBar* menuBar = mainWindow->menuBar();
 menuBar->setContextMenuPolicy(Qt::PreventContextMenu);

 QMenu* fileMenu = menuBar->addMenu("&File");
 fileMenu->setObjectName("FileMenu");

 QAction* fileOpenAction = new QmitkExtFileOpenAction(QIcon(":/org.mitk.gui.qt.ext/Load_48.png"), window);
 fileMenu->addAction(fileOpenAction);
 fileSaveProjectAction = new QmitkExtFileSaveProjectAction(window);
 fileSaveProjectAction->setIcon(QIcon(":/org.mitk.gui.qt.ext/Save_48.png"));
 fileMenu->addAction(fileSaveProjectAction);
 closeProjectAction = new QmitkCloseProjectAction(window);
 closeProjectAction->setIcon(QIcon(":/org.mitk.gui.qt.ext/Remove_48.png"));
 fileMenu->addAction(closeProjectAction);
 fileMenu->addSeparator();
 QAction* fileExitAction = new QmitkFileExitAction(window);
 fileExitAction->setObjectName("QmitkFileExitAction");
 fileMenu->addAction(fileExitAction);

 berry::IViewRegistry* viewRegistry =
  berry::PlatformUI::GetWorkbench()->GetViewRegistry();
 const std::vector<berry::IViewDescriptor::Pointer>& viewDescriptors =
  viewRegistry->GetViews();

 // another bad hack to get an edit/undo menu...
 QMenu* editMenu = menuBar->addMenu("&Edit");
 undoAction = editMenu->addAction(QIcon(":/org.mitk.gui.qt.ext/Undo_48.png"),
  "&Undo",
  QmitkExtWorkbenchWindowAdvisorHack::undohack, SLOT(onUndo()),
  QKeySequence("CTRL+Z"));
 undoAction->setToolTip("Undo the last action (not supported by all modules)");
 redoAction = editMenu->addAction(QIcon(":/org.mitk.gui.qt.ext/Redo_48.png")
  , "&Redo",
  QmitkExtWorkbenchWindowAdvisorHack::undohack, SLOT(onRedo()),
  QKeySequence("CTRL+Y"));
 redoAction->setToolTip("execute the last action that was undone again (not supported by all modules)");

 imageNavigatorAction = new QAction(QIcon(":/org.mitk.gui.qt.ext/Slider.png"), "&Image Navigator", NULL);
 bool imageNavigatorViewFound = window->GetWorkbench()->GetViewRegistry()->Find("org.mitk.views.imagenavigator");
 if (imageNavigatorViewFound)
 {
   QObject::connect(imageNavigatorAction, SIGNAL(triggered(bool)), QmitkExtWorkbenchWindowAdvisorHack::undohack, SLOT(onImageNavigator()));
   imageNavigatorAction->setCheckable(true);

   // add part listener for image navigator
   imageNavigatorPartListener = new PartListenerForImageNavigator(imageNavigatorAction);
   window->GetPartService()->AddPartListener(imageNavigatorPartListener);
   berry::IViewPart::Pointer imageNavigatorView =
       window->GetActivePage()->FindView("org.mitk.views.imagenavigator");
   imageNavigatorAction->setChecked(false);
   if (imageNavigatorView)
   {
     bool isImageNavigatorVisible = window->GetActivePage()->IsPartVisible(imageNavigatorView);
     if (isImageNavigatorVisible)
       imageNavigatorAction->setChecked(true);
   }
   imageNavigatorAction->setToolTip("Open image navigator for navigating through image");
 }

 // toolbar for showing file open, undo, redo and other main actions
 QToolBar* mainActionsToolBar = new QToolBar;
 mainActionsToolBar->setContextMenuPolicy(Qt::PreventContextMenu);
#ifdef __APPLE__
 mainActionsToolBar->setToolButtonStyle ( Qt::ToolButtonTextUnderIcon );
#else
 mainActionsToolBar->setToolButtonStyle ( Qt::ToolButtonTextBesideIcon );
#endif

 mainActionsToolBar->addAction(fileOpenAction);
 mainActionsToolBar->addAction(fileSaveProjectAction);
 mainActionsToolBar->addAction(closeProjectAction);
 mainActionsToolBar->addAction(undoAction);
 mainActionsToolBar->addAction(redoAction);
 if (imageNavigatorViewFound)
 {
   mainActionsToolBar->addAction(imageNavigatorAction);
 }
 mainWindow->addToolBar(mainActionsToolBar);

#ifdef __APPLE__
 mainWindow->setUnifiedTitleAndToolBarOnMac(true);
#endif

 // ==== Window Menu ==========================
 QMenu* windowMenu = menuBar->addMenu("Window");
 if (showNewWindowMenuItem)
 {
   windowMenu->addAction("&New Window", QmitkExtWorkbenchWindowAdvisorHack::undohack, SLOT(onNewWindow()));
   windowMenu->addSeparator();
 }

 QMenu* perspMenu = windowMenu->addMenu("&Open Perspective");

 QMenu* viewMenu;
 if (showViewMenuItem)
 {
   viewMenu = windowMenu->addMenu("Show &View");
   viewMenu->setObjectName("Show View");
 }
 windowMenu->addSeparator();
 resetPerspAction = windowMenu->addAction("&Reset Perspective",
  QmitkExtWorkbenchWindowAdvisorHack::undohack, SLOT(onResetPerspective()));

 if(showClosePerspectiveMenuItem)
  closePerspAction = windowMenu->addAction("&Close Perspective", QmitkExtWorkbenchWindowAdvisorHack::undohack, SLOT(onClosePerspective()));

 windowMenu->addSeparator();
 windowMenu->addAction("&Preferences...",
  QmitkExtWorkbenchWindowAdvisorHack::undohack, SLOT(onEditPreferences()),
  QKeySequence("CTRL+P"));

 // fill perspective menu
 berry::IPerspectiveRegistry* perspRegistry =
  window->GetWorkbench()->GetPerspectiveRegistry();
 QActionGroup* perspGroup = new QActionGroup(menuBar);

 std::vector<berry::IPerspectiveDescriptor::Pointer> perspectives(
  perspRegistry->GetPerspectives());

    bool skip = false;
    for (std::vector<berry::IPerspectiveDescriptor::Pointer>::iterator perspIt =
      perspectives.begin(); perspIt != perspectives.end(); ++perspIt)
    {

      // if perspectiveExcludeList is set, it contains the id-strings of perspectives, which
      // should not appear as an menu-entry in the perspective menu
      if (perspectiveExcludeList.size() > 0)
      {
        for (unsigned int i=0; i<perspectiveExcludeList.size(); i++)
        {
          if (perspectiveExcludeList.at(i) == (*perspIt)->GetId())
          {
            skip = true;
            break;
          }
        }
        if (skip)
        {
          skip = false;
          continue;
        }
      }

      QAction* perspAction = new berry::QtOpenPerspectiveAction(window,
        *perspIt, perspGroup);
      mapPerspIdToAction.insert(std::make_pair((*perspIt)->GetId(), perspAction));
    }
 perspMenu->addActions(perspGroup->actions());

 // sort elements (converting vector to map...)
 std::vector<berry::IViewDescriptor::Pointer>::const_iterator iter;
 std::map<std::string, berry::IViewDescriptor::Pointer> VDMap;

 skip = false;
 for (iter = viewDescriptors.begin(); iter != viewDescriptors.end(); ++iter)
 {

   // if viewExcludeList is set, it contains the id-strings of view, which
   // should not appear as an menu-entry in the menu
   if (viewExcludeList.size() > 0)
   {
     for (unsigned int i=0; i<viewExcludeList.size(); i++)
     {
       if (viewExcludeList.at(i) == (*iter)->GetId())
       {
         skip = true;
         break;
       }
     }
     if (skip)
     {
       skip = false;
       continue;
     }
   }

  if ((*iter)->GetId() == "org.blueberry.ui.internal.introview")
   continue;
  if ((*iter)->GetId() == "org.mitk.views.imagenavigator")
   continue;

  std::pair<std::string, berry::IViewDescriptor::Pointer> p(
   (*iter)->GetLabel(), (*iter));
  VDMap.insert(p);
 }
 // ==================================================

  // ==== Perspective Toolbar ==================================
 QToolBar* qPerspectiveToolbar = new QToolBar;

 if (showPerspectiveToolbar)
 {
  qPerspectiveToolbar->addActions(perspGroup->actions());
  mainWindow->addToolBar(qPerspectiveToolbar);
 }
 else
  delete qPerspectiveToolbar;

 // ==== View Toolbar ==================================
 QToolBar* qToolbar = new QToolBar;

 std::map<std::string, berry::IViewDescriptor::Pointer>::const_iterator
  MapIter;
 for (MapIter = VDMap.begin(); MapIter != VDMap.end(); ++MapIter)
 {
  berry::QtShowViewAction* viewAction = new berry::QtShowViewAction(window,
   (*MapIter).second);
  viewActions.push_back(viewAction);
  if(showViewMenuItem)
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

 QSettings settings(GetQSettingsFile(), QSettings::IniFormat);
 mainWindow->restoreState(settings.value("ToolbarPosition").toByteArray());


 // ====================================================

 // ===== Help menu ====================================
 QMenu* helpMenu = menuBar->addMenu("Help");
 helpMenu->addAction("&Welcome",this, SLOT(onIntro()));
 helpMenu->addAction("&Contents", this, SLOT(onHelpContents()));
  helpMenu->addAction("Context &Help",this, SLOT(onHelp()),  QKeySequence("F1"));
 helpMenu->addAction("&About",this, SLOT(onAbout()));
 // =====================================================


 QStatusBar* qStatusBar = new QStatusBar();

 //creating a QmitkStatusBar for Output on the QStatusBar and connecting it with the MainStatusBar
 QmitkStatusBar *statusBar = new QmitkStatusBar(qStatusBar);
 //disabling the SizeGrip in the lower right corner
 statusBar->SetSizeGripEnabled(false);



 QmitkProgressBar *progBar = new QmitkProgressBar();

 qStatusBar->addPermanentWidget(progBar, 0);
 progBar->hide();
// progBar->AddStepsToDo(2);
// progBar->Progress(1);

 mainWindow->setStatusBar(qStatusBar);

 QmitkMemoryUsageIndicatorView* memoryIndicator =
  new QmitkMemoryUsageIndicatorView();
 qStatusBar->addPermanentWidget(memoryIndicator, 0);
}

void QmitkExtWorkbenchWindowAdvisor::PreWindowOpen()
{
 berry::IWorkbenchWindowConfigurer::Pointer configurer = GetWindowConfigurer();

 // show the shortcut bar and progress indicator, which are hidden by
 // default
 //configurer->SetShowPerspectiveBar(true);
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

 menuPerspectiveListener = new PerspectiveListenerForMenu(this);
 configurer->GetWindow()->AddPerspectiveListener(menuPerspectiveListener);
}

void QmitkExtWorkbenchWindowAdvisor::onIntro()
{
  QmitkExtWorkbenchWindowAdvisorHack::undohack->onIntro();
}

void QmitkExtWorkbenchWindowAdvisor::onHelp()
{
  QmitkExtWorkbenchWindowAdvisorHack::undohack->onHelp();
}

void QmitkExtWorkbenchWindowAdvisor::onHelpContents()
{
  QmitkExtWorkbenchWindowAdvisorHack::undohack->onHelpContents();
}

void QmitkExtWorkbenchWindowAdvisor::onAbout()
{
  QmitkExtWorkbenchWindowAdvisorHack::undohack->onAbout();
}

//--------------------------------------------------------------------------------
// Ugly hack from here on. Feel free to delete when command framework
// and undo buttons are done.
//--------------------------------------------------------------------------------

QmitkExtWorkbenchWindowAdvisorHack::QmitkExtWorkbenchWindowAdvisorHack() : QObject()
{

}

QmitkExtWorkbenchWindowAdvisorHack::~QmitkExtWorkbenchWindowAdvisorHack()
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
    MITK_INFO << "Undo " << descriptions.front().second;
   }
  }
  model->Undo();
 }
 else
 {
  MITK_ERROR << "No undo model instantiated";
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
    MITK_INFO << "Redo " << descriptions.front().second;
   }
  }
  model->Redo();
 }
 else
 {
  MITK_ERROR << "No undo model instantiated";
 }
}

void QmitkExtWorkbenchWindowAdvisorHack::onImageNavigator()
{
 // get ImageNavigatorView
 berry::IViewPart::Pointer imageNavigatorView =
  berry::PlatformUI::GetWorkbench()->GetActiveWorkbenchWindow()->GetActivePage()->FindView("org.mitk.views.imagenavigator");
 if (imageNavigatorView)
 {
  bool isImageNavigatorVisible = berry::PlatformUI::GetWorkbench()->GetActiveWorkbenchWindow()->GetActivePage()->IsPartVisible(imageNavigatorView);
  if (isImageNavigatorVisible)
  {
   berry::PlatformUI::GetWorkbench()->GetActiveWorkbenchWindow()->GetActivePage()->HideView(imageNavigatorView);
   return;
  }
 }
 berry::PlatformUI::GetWorkbench()->GetActiveWorkbenchWindow()->GetActivePage()->ShowView("org.mitk.views.imagenavigator");
 //berry::PlatformUI::GetWorkbench()->GetActiveWorkbenchWindow()->GetActivePage()->ResetPerspective();
}

void QmitkExtWorkbenchWindowAdvisorHack::onEditPreferences()
{
 QmitkPreferencesDialog _PreferencesDialog(QApplication::activeWindow());
 _PreferencesDialog.exec();
}

void QmitkExtWorkbenchWindowAdvisorHack::onQuit()
{
 berry::PlatformUI::GetWorkbench()->Close();
}

void QmitkExtWorkbenchWindowAdvisorHack::onResetPerspective()
{
 berry::PlatformUI::GetWorkbench()->GetActiveWorkbenchWindow()->GetActivePage()->ResetPerspective();
}

void QmitkExtWorkbenchWindowAdvisorHack::onClosePerspective()
{
 berry::IWorkbenchPage::Pointer
  page =
  berry::PlatformUI::GetWorkbench()->GetActiveWorkbenchWindow()->GetActivePage();
 page->ClosePerspective(page->GetPerspective(), true, true);
}

void QmitkExtWorkbenchWindowAdvisorHack::onNewWindow()
{
 berry::PlatformUI::GetWorkbench()->OpenWorkbenchWindow(0);
}

void QmitkExtWorkbenchWindowAdvisorHack::onIntro()
{
 bool hasIntro =
  berry::PlatformUI::GetWorkbench()->GetIntroManager()->HasIntro();
 if (!hasIntro)
 {

  QRegExp reg("(.*)<title>(\\n)*");
  QRegExp reg2("(\\n)*</title>(.*)");
  QFile file(":/org.mitk.gui.qt.ext/index.html");
  file.open(QIODevice::ReadOnly | QIODevice::Text); // Als Text-Datei nur zum Lesen öffnen

  QString text = QString(file.readAll());

  file.close();

  QString title = text;
  title.replace(reg, "");
  title.replace(reg2, "");

  std::cout << title.toStdString() << std::endl;

  QMessageBox::information(NULL, title,
   text, "Close");

 }
 else
 {
  berry::PlatformUI::GetWorkbench()->GetIntroManager()->ShowIntro(
   berry::PlatformUI::GetWorkbench()->GetActiveWorkbenchWindow(), false);
 }
}

void QmitkExtWorkbenchWindowAdvisorHack::onHelp()
{
  ctkPluginContext* context = QmitkCommonExtPlugin::getContext();
  if (context == 0)
  {
    MITK_WARN << "Plugin context not set, unable to open context help";
    return;
  }

  // Check if the org.blueberry.ui.qt.help plug-in is installed and started
  QList<QSharedPointer<ctkPlugin> > plugins = context->getPlugins();
  foreach(QSharedPointer<ctkPlugin> p, plugins)
  {
    if (p->getSymbolicName() == "org.blueberry.ui.qt.help")
    {
      if (p->getState() != ctkPlugin::ACTIVE)
      {
        // try to activate the plug-in explicitly
        try
        {
          p->start(ctkPlugin::START_TRANSIENT);
        }
        catch (const ctkPluginException& pe)
        {
          MITK_ERROR << "Activating org.blueberry.ui.qt.help failed: " << pe.what();
          return;
        }
      }
    }
  }

  ctkServiceReference eventAdminRef = context->getServiceReference<ctkEventAdmin>();
  ctkEventAdmin* eventAdmin = 0;
  if (eventAdminRef)
  {
    eventAdmin = context->getService<ctkEventAdmin>(eventAdminRef);
  }
  if (eventAdmin == 0)
  {
    MITK_WARN << "ctkEventAdmin service not found. Unable to open context help";
  }
  else
  {
    ctkEvent ev("org/blueberry/ui/help/CONTEXTHELP_REQUESTED");
    eventAdmin->postEvent(ev);
  }
}

void QmitkExtWorkbenchWindowAdvisorHack::onHelpContents()
{
  berry::PlatformUI::GetWorkbench()->ShowPerspective("org.blueberry.perspectives.help",
                                                     berry::PlatformUI::GetWorkbench()->GetActiveWorkbenchWindow());
}

void QmitkExtWorkbenchWindowAdvisorHack::onAbout()
{
 QmitkAboutDialog* aboutDialog = new QmitkAboutDialog(QApplication::activeWindow(),NULL);
 aboutDialog->open();
}

void QmitkExtWorkbenchWindowAdvisor::HookTitleUpdateListeners(
 berry::IWorkbenchWindowConfigurer::Pointer configurer)
{
 // hook up the listeners to update the window title
 titlePartListener = new PartListenerForTitle(this);
 titlePerspectiveListener = new PerspectiveListenerForTitle(this);
 editorPropertyListener = new berry::PropertyChangeIntAdapter<
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
 berry::IWorkbenchWindowConfigurer::Pointer configurer =
  GetWindowConfigurer();
 berry::IWorkbenchPage::Pointer currentPage =
  configurer->GetWindow()->GetActivePage();
 berry::IEditorPart::Pointer activeEditor;
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
 // instead of the product name, we use a custom variable for now
 title = productName;

 if(showMitkVersionInfo)
 {
   title += std::string(" ") + MITK_VERSION_STRING;
 }

 if (showVersionInfo)
 {
  // add version informatioin
  QString mitkRevision(MITK_REVISION);
  mitkRevision = mitkRevision.left(6) + " (" MITK_REVISION_NAME ")";
  QString versions = QString(" (ITK %1.%2.%3  VTK %4.%5.%6 Qt %7 MITK %8)")
   .arg(ITK_VERSION_MAJOR).arg(ITK_VERSION_MINOR).arg(ITK_VERSION_PATCH)
   .arg(VTK_MAJOR_VERSION).arg(VTK_MINOR_VERSION).arg(VTK_BUILD_VERSION)
   .arg(QT_VERSION_STR)
   .arg(mitkRevision);

  title += versions.toStdString();
 }

 if (currentPage)
 {
  if (activeEditor)
  {
   lastEditorTitle = activeEditor->GetTitleToolTip();
   if (!lastEditorTitle.empty())
    title = lastEditorTitle + " - " + title;
  }
  berry::IPerspectiveDescriptor::Pointer persp =
   currentPage->GetPerspective();
  std::string label = "";
  if (persp)
  {
   label = persp->GetLabel();
  }
  berry::IAdaptable* input = currentPage->GetInput();
  if (input && input != wbAdvisor->GetDefaultPageInput())
  {
   label = currentPage->GetLabel();
  }
  if (!label.empty())
  {
   title = label + " - " + title;
  }
 }

 title += " (Not for use in diagnosis or treatment of patients)";

 return title;
}

void QmitkExtWorkbenchWindowAdvisor::RecomputeTitle()
{
 berry::IWorkbenchWindowConfigurer::Pointer configurer =
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
 berry::IWorkbenchWindowConfigurer::Pointer configurer =
  GetWindowConfigurer();
 berry::IWorkbenchWindow::Pointer window = configurer->GetWindow();
 berry::IEditorPart::Pointer activeEditor;
 berry::IWorkbenchPage::Pointer currentPage = window->GetActivePage();
 berry::IPerspectiveDescriptor::Pointer persp;
 berry::IAdaptable* input = 0;

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

void QmitkExtWorkbenchWindowAdvisor::PropertyChange(berry::Object::Pointer /*source*/, int propId)
{
 if (propId == berry::IWorkbenchPartConstants::PROP_TITLE)
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


void QmitkExtWorkbenchWindowAdvisor::SetPerspectiveExcludeList(std::vector<std::string> v)
{
  this->perspectiveExcludeList = v;
}

std::vector<std::string> QmitkExtWorkbenchWindowAdvisor::GetPerspectiveExcludeList()
{
  return this->perspectiveExcludeList;
}

void QmitkExtWorkbenchWindowAdvisor::SetViewExcludeList(std::vector<std::string> v)
{
  this->viewExcludeList = v;
}

std::vector<std::string> QmitkExtWorkbenchWindowAdvisor::GetViewExcludeList()
{
  return this->viewExcludeList;
}

void QmitkExtWorkbenchWindowAdvisor::PostWindowClose()
{
  berry::IWorkbenchWindow::Pointer window = this->GetWindowConfigurer()->GetWindow();
  QMainWindow* mainWindow = static_cast<QMainWindow*> (window->GetShell()->GetControl());

  QSettings settings(GetQSettingsFile(), QSettings::IniFormat);
  settings.setValue("ToolbarPosition", mainWindow->saveState());
}

QString QmitkExtWorkbenchWindowAdvisor::GetQSettingsFile() const
{
  QFileInfo settingsInfo = QmitkCommonExtPlugin::getContext()->getDataFile(QT_SETTINGS_FILENAME);
  return settingsInfo.canonicalFilePath();
}
