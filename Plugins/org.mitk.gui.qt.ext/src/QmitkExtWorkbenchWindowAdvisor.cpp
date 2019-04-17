/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

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
#include <berryIActionBarConfigurer.h>
#include <berryIWorkbenchWindow.h>
#include <berryIWorkbenchPage.h>
#include <berryIPreferencesService.h>
#include <berryIPerspectiveRegistry.h>
#include <berryIPerspectiveDescriptor.h>
#include <berryIProduct.h>
#include <berryIWorkbenchPartConstants.h>
#include <berryQtPreferences.h>
#include <berryQtStyleManager.h>
#include <berryWorkbenchPlugin.h>

#include <internal/berryQtShowViewAction.h>
#include <internal/berryQtOpenPerspectiveAction.h>

#include <QmitkFileOpenAction.h>
#include <QmitkFileSaveAction.h>
#include <QmitkExtFileSaveProjectAction.h>
#include <QmitkFileExitAction.h>
#include <QmitkCloseProjectAction.h>
#include <QmitkUndoAction.h>
#include <QmitkRedoAction.h>
#include <QmitkDefaultDropTargetListener.h>
#include <QmitkStatusBar.h>
#include <QmitkProgressBar.h>
#include <QmitkMemoryUsageIndicatorView.h>
#include <QmitkPreferencesDialog.h>
#include <QmitkOpenDicomEditorAction.h>
#include <QmitkOpenMxNMultiWidgetEditorAction.h>
#include <QmitkOpenStdMultiWidgetEditorAction.h>

#include <itkConfigure.h>
#include <vtkConfigure.h>
#include <mitkVersion.h>
#include <mitkIDataStorageService.h>
#include <mitkIDataStorageReference.h>
#include <mitkDataStorageEditorInput.h>
#include <mitkWorkbenchUtil.h>
#include <vtkVersionMacros.h>

// UGLYYY
#include "internal/QmitkExtWorkbenchWindowAdvisorHack.h"
#include "internal/QmitkCommonExtPlugin.h"
#include "mitkUndoController.h"
#include "mitkVerboseLimitedLinearUndo.h"
#include <QToolBar>
#include <QToolButton>
#include <QMessageBox>
#include <QMouseEvent>
#include <QLabel>
#include <QmitkAboutDialog.h>

QmitkExtWorkbenchWindowAdvisorHack* QmitkExtWorkbenchWindowAdvisorHack::undohack =
  new QmitkExtWorkbenchWindowAdvisorHack();

QString QmitkExtWorkbenchWindowAdvisor::QT_SETTINGS_FILENAME = "QtSettings.ini";

static bool USE_EXPERIMENTAL_COMMAND_CONTRIBUTIONS = false;

class PartListenerForTitle: public berry::IPartListener
{
public:

  PartListenerForTitle(QmitkExtWorkbenchWindowAdvisor* wa)
    : windowAdvisor(wa)
  {
  }

  Events::Types GetPartEventTypes() const override
  {
    return Events::ACTIVATED | Events::BROUGHT_TO_TOP | Events::CLOSED
      | Events::HIDDEN | Events::VISIBLE;
  }

  void PartActivated(const berry::IWorkbenchPartReference::Pointer& ref) override
  {
    if (ref.Cast<berry::IEditorReference> ())
    {
      windowAdvisor->UpdateTitle(false);
    }
  }

  void PartBroughtToTop(const berry::IWorkbenchPartReference::Pointer& ref) override
  {
    if (ref.Cast<berry::IEditorReference> ())
    {
      windowAdvisor->UpdateTitle(false);
    }
  }

  void PartClosed(const berry::IWorkbenchPartReference::Pointer& /*ref*/) override
  {
    windowAdvisor->UpdateTitle(false);
  }

  void PartHidden(const berry::IWorkbenchPartReference::Pointer& ref) override
  {
    if (!windowAdvisor->lastActiveEditor.Expired() &&
      ref->GetPart(false) == windowAdvisor->lastActiveEditor.Lock())
    {
      windowAdvisor->UpdateTitle(true);
    }
  }

  void PartVisible(const berry::IWorkbenchPartReference::Pointer& ref) override
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

class PartListenerForViewNavigator: public berry::IPartListener
{
public:

  PartListenerForViewNavigator(QAction* act)
    : viewNavigatorAction(act)
  {
  }

  Events::Types GetPartEventTypes() const override
  {
    return Events::OPENED | Events::CLOSED | Events::HIDDEN |
      Events::VISIBLE;
  }

  void PartOpened(const berry::IWorkbenchPartReference::Pointer& ref) override
  {
    if (ref->GetId()=="org.mitk.views.viewnavigatorview")
    {
      viewNavigatorAction->setChecked(true);
    }
  }

  void PartClosed(const berry::IWorkbenchPartReference::Pointer& ref) override
  {
    if (ref->GetId()=="org.mitk.views.viewnavigatorview")
    {
      viewNavigatorAction->setChecked(false);
    }
  }

  void PartVisible(const berry::IWorkbenchPartReference::Pointer& ref) override
  {
    if (ref->GetId()=="org.mitk.views.viewnavigatorview")
    {
      viewNavigatorAction->setChecked(true);
    }
  }

  void PartHidden(const berry::IWorkbenchPartReference::Pointer& ref) override
  {
    if (ref->GetId()=="org.mitk.views.viewnavigatorview")
    {
      viewNavigatorAction->setChecked(false);
    }
  }

private:
  QAction* viewNavigatorAction;
};

class PartListenerForImageNavigator: public berry::IPartListener
{
public:

  PartListenerForImageNavigator(QAction* act)
    : imageNavigatorAction(act)
  {
  }

  Events::Types GetPartEventTypes() const override
  {
    return Events::OPENED | Events::CLOSED | Events::HIDDEN |
      Events::VISIBLE;
  }

  void PartOpened(const berry::IWorkbenchPartReference::Pointer& ref) override
  {
    if (ref->GetId()=="org.mitk.views.imagenavigator")
    {
      imageNavigatorAction->setChecked(true);
    }
  }

  void PartClosed(const berry::IWorkbenchPartReference::Pointer& ref) override
  {
    if (ref->GetId()=="org.mitk.views.imagenavigator")
    {
      imageNavigatorAction->setChecked(false);
    }
  }

  void PartVisible(const berry::IWorkbenchPartReference::Pointer& ref) override
  {
    if (ref->GetId()=="org.mitk.views.imagenavigator")
    {
      imageNavigatorAction->setChecked(true);
    }
  }

  void PartHidden(const berry::IWorkbenchPartReference::Pointer& ref) override
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

  PerspectiveListenerForTitle(QmitkExtWorkbenchWindowAdvisor* wa)
    : windowAdvisor(wa)
    , perspectivesClosed(false)
  {
  }

  Events::Types GetPerspectiveEventTypes() const override
  {
    if (USE_EXPERIMENTAL_COMMAND_CONTRIBUTIONS)
    {
      return Events::ACTIVATED | Events::SAVED_AS | Events::DEACTIVATED;
    }
    else
    {
      return Events::ACTIVATED | Events::SAVED_AS | Events::DEACTIVATED
        | Events::CLOSED | Events::OPENED;
    }
  }

  void PerspectiveActivated(const berry::IWorkbenchPage::Pointer& /*page*/,
    const berry::IPerspectiveDescriptor::Pointer& /*perspective*/) override
  {
    windowAdvisor->UpdateTitle(false);
  }

  void PerspectiveSavedAs(const berry::IWorkbenchPage::Pointer& /*page*/,
    const berry::IPerspectiveDescriptor::Pointer& /*oldPerspective*/,
    const berry::IPerspectiveDescriptor::Pointer& /*newPerspective*/) override
  {
    windowAdvisor->UpdateTitle(false);
  }

  void PerspectiveDeactivated(const berry::IWorkbenchPage::Pointer& /*page*/,
    const berry::IPerspectiveDescriptor::Pointer& /*perspective*/) override
  {
    windowAdvisor->UpdateTitle(false);
  }

  void PerspectiveOpened(const berry::IWorkbenchPage::Pointer& /*page*/,
    const berry::IPerspectiveDescriptor::Pointer& /*perspective*/) override
  {
    if (perspectivesClosed)
    {
      QListIterator<QAction*> i(windowAdvisor->viewActions);
      while (i.hasNext())
      {
        i.next()->setEnabled(true);
      }

      //GetViewRegistry()->Find("org.mitk.views.imagenavigator");
      if(windowAdvisor->GetWindowConfigurer()->GetWindow()->GetWorkbench()->GetEditorRegistry()->FindEditor("org.mitk.editors.dicomeditor"))
      {
        windowAdvisor->openDicomEditorAction->setEnabled(true);
      }
      if (windowAdvisor->GetWindowConfigurer()->GetWindow()->GetWorkbench()->GetEditorRegistry()->FindEditor("org.mitk.editors.stdmultiwidget"))
      {
        windowAdvisor->openStdMultiWidgetEditorAction->setEnabled(true);
      }
      if (windowAdvisor->GetWindowConfigurer()->GetWindow()->GetWorkbench()->GetEditorRegistry()->FindEditor("org.mitk.editors.mxnmultiwidget"))
      {
        windowAdvisor->openMxNMultiWidgetEditorAction->setEnabled(true);
      }

      windowAdvisor->fileSaveProjectAction->setEnabled(true);
      windowAdvisor->closeProjectAction->setEnabled(true);
      windowAdvisor->undoAction->setEnabled(true);
      windowAdvisor->redoAction->setEnabled(true);
      windowAdvisor->imageNavigatorAction->setEnabled(true);
      windowAdvisor->viewNavigatorAction->setEnabled(true);
      windowAdvisor->resetPerspAction->setEnabled(true);
      if( windowAdvisor->GetShowClosePerspectiveMenuItem() )
      {
        windowAdvisor->closePerspAction->setEnabled(true);
      }
    }

    perspectivesClosed = false;
  }

  void PerspectiveClosed(const berry::IWorkbenchPage::Pointer& /*page*/,
    const berry::IPerspectiveDescriptor::Pointer& /*perspective*/) override
  {
    berry::IWorkbenchWindow::Pointer wnd = windowAdvisor->GetWindowConfigurer()->GetWindow();
    bool allClosed = true;
    if (wnd->GetActivePage())
    {
      QList<berry::IPerspectiveDescriptor::Pointer> perspectives(wnd->GetActivePage()->GetOpenPerspectives());
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

      if(windowAdvisor->GetWindowConfigurer()->GetWindow()->GetWorkbench()->GetEditorRegistry()->FindEditor("org.mitk.editors.dicomeditor"))
      {
        windowAdvisor->openDicomEditorAction->setEnabled(false);
      }
      if (windowAdvisor->GetWindowConfigurer()->GetWindow()->GetWorkbench()->GetEditorRegistry()->FindEditor("org.mitk.editors.stdmultiwidget"))
      {
        windowAdvisor->openStdMultiWidgetEditorAction->setEnabled(false);
      }
      if (windowAdvisor->GetWindowConfigurer()->GetWindow()->GetWorkbench()->GetEditorRegistry()->FindEditor("org.mitk.editors.mxnmultiwidget"))
      {
        windowAdvisor->openMxNMultiWidgetEditorAction->setEnabled(false);
      }

      windowAdvisor->fileSaveProjectAction->setEnabled(false);
      windowAdvisor->closeProjectAction->setEnabled(false);
      windowAdvisor->undoAction->setEnabled(false);
      windowAdvisor->redoAction->setEnabled(false);
      windowAdvisor->imageNavigatorAction->setEnabled(false);
      windowAdvisor->viewNavigatorAction->setEnabled(false);
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

  PerspectiveListenerForMenu(QmitkExtWorkbenchWindowAdvisor* wa)
    : windowAdvisor(wa)
  {
  }

  Events::Types GetPerspectiveEventTypes() const override
  {
    return Events::ACTIVATED | Events::DEACTIVATED;
  }

  void PerspectiveActivated(const berry::IWorkbenchPage::Pointer& /*page*/,
    const berry::IPerspectiveDescriptor::Pointer& perspective) override
  {
    QAction* action = windowAdvisor->mapPerspIdToAction[perspective->GetId()];
    if (action)
    {
      action->setChecked(true);
    }
  }

  void PerspectiveDeactivated(const berry::IWorkbenchPage::Pointer& /*page*/,
    const berry::IPerspectiveDescriptor::Pointer& perspective) override
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
                                                               berry::IWorkbenchWindowConfigurer::Pointer configurer)
  : berry::WorkbenchWindowAdvisor(configurer)
  , lastInput(nullptr)
  , wbAdvisor(wbAdvisor)
  , showViewToolbar(true)
  , showPerspectiveToolbar(false)
  , showVersionInfo(true)
  , showMitkVersionInfo(true)
  , showViewMenuItem(true)
  , showNewWindowMenuItem(false)
  , showClosePerspectiveMenuItem(true)
  , viewNavigatorFound(false)
  , showMemoryIndicator(true)
  , dropTargetListener(new QmitkDefaultDropTargetListener)
{
  productName = QCoreApplication::applicationName();
  viewExcludeList.push_back("org.mitk.views.viewnavigatorview");
}

QmitkExtWorkbenchWindowAdvisor::~QmitkExtWorkbenchWindowAdvisor()
{
}

berry::ActionBarAdvisor::Pointer QmitkExtWorkbenchWindowAdvisor::CreateActionBarAdvisor(berry::IActionBarConfigurer::Pointer configurer)
{
  if (USE_EXPERIMENTAL_COMMAND_CONTRIBUTIONS)
  {
    berry::ActionBarAdvisor::Pointer actionBarAdvisor(new QmitkExtActionBarAdvisor(configurer));
    return actionBarAdvisor;
  }
  else
  {
    return berry::WorkbenchWindowAdvisor::CreateActionBarAdvisor(configurer);
  }
}

QWidget* QmitkExtWorkbenchWindowAdvisor::CreateEmptyWindowContents(QWidget* parent)
{
  QWidget* parentWidget = static_cast<QWidget*>(parent);
  auto   label = new QLabel(parentWidget);
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

void QmitkExtWorkbenchWindowAdvisor::ShowMemoryIndicator(bool show)
{
  showMemoryIndicator = show;
}

bool QmitkExtWorkbenchWindowAdvisor::GetShowMemoryIndicator()
{
  return showMemoryIndicator;
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

void QmitkExtWorkbenchWindowAdvisor::SetProductName(const QString& product)
{
  productName = product;
}

void QmitkExtWorkbenchWindowAdvisor::SetWindowIcon(const QString& wndIcon)
{
  windowIcon = wndIcon;
}

void QmitkExtWorkbenchWindowAdvisor::PostWindowCreate()
{
  // very bad hack...
  berry::IWorkbenchWindow::Pointer window = this->GetWindowConfigurer()->GetWindow();
  QMainWindow* mainWindow = qobject_cast<QMainWindow*> (window->GetShell()->GetControl());

  if (!windowIcon.isEmpty())
  {
    mainWindow->setWindowIcon(QIcon(windowIcon));
  }
  mainWindow->setContextMenuPolicy(Qt::PreventContextMenu);

  // Load icon theme
  QIcon::setThemeSearchPaths(QStringList() << QStringLiteral(":/org_mitk_icons/icons/"));
  QIcon::setThemeName(QStringLiteral("awesome"));

  // ==== Application menu ============================

  QMenuBar* menuBar = mainWindow->menuBar();
  menuBar->setContextMenuPolicy(Qt::PreventContextMenu);

#ifdef __APPLE__
  menuBar->setNativeMenuBar(true);
#else
  menuBar->setNativeMenuBar(false);
#endif

  auto basePath = QStringLiteral(":/org_mitk_icons/icons/awesome/scalable/actions/");

  auto fileOpenAction = new QmitkFileOpenAction(berry::QtStyleManager::ThemeIcon(basePath + "document-open.svg"), window);
  fileOpenAction->setShortcut(QKeySequence::Open);
  auto fileSaveAction = new QmitkFileSaveAction(berry::QtStyleManager::ThemeIcon(basePath + "document-save.svg"), window);
  fileSaveAction->setShortcut(QKeySequence::Save);
  fileSaveProjectAction = new QmitkExtFileSaveProjectAction(window);
  fileSaveProjectAction->setIcon(berry::QtStyleManager::ThemeIcon(basePath + "document-save.svg"));
  closeProjectAction = new QmitkCloseProjectAction(window);
  closeProjectAction->setIcon(berry::QtStyleManager::ThemeIcon(basePath + "edit-delete.svg"));

  auto   perspGroup = new QActionGroup(menuBar);
  std::map<QString, berry::IViewDescriptor::Pointer> VDMap;

  // sort elements (converting vector to map...)
  QList<berry::IViewDescriptor::Pointer>::const_iterator iter;

  berry::IViewRegistry* viewRegistry =
    berry::PlatformUI::GetWorkbench()->GetViewRegistry();
  const QList<berry::IViewDescriptor::Pointer> viewDescriptors = viewRegistry->GetViews();

  bool skip = false;
  for (iter = viewDescriptors.begin(); iter != viewDescriptors.end(); ++iter)
  {
    // if viewExcludeList is set, it contains the id-strings of view, which
    // should not appear as an menu-entry in the menu
    if (viewExcludeList.size() > 0)
    {
      for (int i=0; i<viewExcludeList.size(); i++)
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
    if ((*iter)->GetId() == "org.mitk.views.viewnavigatorview")
      continue;

    std::pair<QString, berry::IViewDescriptor::Pointer> p((*iter)->GetLabel(), (*iter));
    VDMap.insert(p);
  }

  std::map<QString, berry::IViewDescriptor::Pointer>::const_iterator MapIter;
  for (MapIter = VDMap.begin(); MapIter != VDMap.end(); ++MapIter)
  {
    berry::QtShowViewAction* viewAction = new berry::QtShowViewAction(window, (*MapIter).second);
    viewActions.push_back(viewAction);
  }

  if (!USE_EXPERIMENTAL_COMMAND_CONTRIBUTIONS)
  {
    QMenu* fileMenu = menuBar->addMenu("&File");
    fileMenu->setObjectName("FileMenu");
    fileMenu->addAction(fileOpenAction);
    fileMenu->addAction(fileSaveAction);
    fileMenu->addAction(fileSaveProjectAction);
    fileMenu->addAction(closeProjectAction);
    fileMenu->addSeparator();

    QAction* fileExitAction = new QmitkFileExitAction(window);
    fileExitAction->setIcon(berry::QtStyleManager::ThemeIcon(basePath + "system-log-out.svg"));
    fileExitAction->setShortcut(QKeySequence::Quit);
    fileExitAction->setObjectName("QmitkFileExitAction");
    fileMenu->addAction(fileExitAction);

    // another bad hack to get an edit/undo menu...
    QMenu* editMenu = menuBar->addMenu("&Edit");
    undoAction = editMenu->addAction(berry::QtStyleManager::ThemeIcon(basePath + "edit-undo.svg"),
      "&Undo",
      QmitkExtWorkbenchWindowAdvisorHack::undohack, SLOT(onUndo()),
      QKeySequence("CTRL+Z"));
    undoAction->setToolTip("Undo the last action (not supported by all modules)");
    redoAction = editMenu->addAction(berry::QtStyleManager::ThemeIcon(basePath + "edit-redo.svg"),
      "&Redo",
      QmitkExtWorkbenchWindowAdvisorHack::undohack, SLOT(onRedo()),
      QKeySequence("CTRL+Y"));
    redoAction->setToolTip("execute the last action that was undone again (not supported by all modules)");

    // ==== Window Menu ==========================
    QMenu* windowMenu = menuBar->addMenu("Window");
    if (showNewWindowMenuItem)
    {
      windowMenu->addAction("&New Window", QmitkExtWorkbenchWindowAdvisorHack::undohack, SLOT(onNewWindow()));
      windowMenu->addSeparator();
    }

    QMenu* perspMenu = windowMenu->addMenu("&Open Perspective");

    QMenu* viewMenu = nullptr;
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

    QList<berry::IPerspectiveDescriptor::Pointer> perspectives(
      perspRegistry->GetPerspectives());

    skip = false;
    for (QList<berry::IPerspectiveDescriptor::Pointer>::iterator perspIt =
      perspectives.begin(); perspIt != perspectives.end(); ++perspIt)
    {
      // if perspectiveExcludeList is set, it contains the id-strings of perspectives, which
      // should not appear as an menu-entry in the perspective menu
      if (perspectiveExcludeList.size() > 0)
      {
        for (int i=0; i<perspectiveExcludeList.size(); i++)
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

      QAction* perspAction = new berry::QtOpenPerspectiveAction(window, *perspIt, perspGroup);
      mapPerspIdToAction.insert((*perspIt)->GetId(), perspAction);
    }
    perspMenu->addActions(perspGroup->actions());

    if (showViewMenuItem)
    {
      for (auto viewAction : viewActions)
      {
        viewMenu->addAction(viewAction);
      }
    }

    // ===== Help menu ====================================
    QMenu* helpMenu = menuBar->addMenu("&Help");
    helpMenu->addAction("&Welcome",this, SLOT(onIntro()));
    helpMenu->addAction("&Open Help Perspective", this, SLOT(onHelpOpenHelpPerspective()));
    helpMenu->addAction("&Context Help",this, SLOT(onHelp()),  QKeySequence("F1"));
    helpMenu->addAction("&About",this, SLOT(onAbout()));
    // =====================================================
  }
  else
  {
    undoAction = new QmitkUndoAction(berry::QtStyleManager::ThemeIcon(basePath + "edit-undo.svg"), nullptr);
    undoAction->setShortcut(QKeySequence::Undo);
    redoAction = new QmitkRedoAction(berry::QtStyleManager::ThemeIcon(basePath + "edit-redo.svg"), nullptr);
    redoAction->setShortcut(QKeySequence::Redo);
  }

  // toolbar for showing file open, undo, redo and other main actions
  auto   mainActionsToolBar = new QToolBar;
  mainActionsToolBar->setObjectName("mainActionsToolBar");
  mainActionsToolBar->setContextMenuPolicy(Qt::PreventContextMenu);
#ifdef __APPLE__
  mainActionsToolBar->setToolButtonStyle ( Qt::ToolButtonTextUnderIcon );
#else
  mainActionsToolBar->setToolButtonStyle ( Qt::ToolButtonTextBesideIcon );
#endif

  basePath = QStringLiteral(":/org.mitk.gui.qt.ext/");
  imageNavigatorAction = new QAction(berry::QtStyleManager::ThemeIcon(basePath + "image_navigator.svg"), "&Image Navigator", nullptr);
  bool imageNavigatorViewFound = window->GetWorkbench()->GetViewRegistry()->Find("org.mitk.views.imagenavigator");

  if (this->GetWindowConfigurer()->GetWindow()->GetWorkbench()->GetEditorRegistry()->FindEditor("org.mitk.editors.dicomeditor"))
  {
    openDicomEditorAction = new QmitkOpenDicomEditorAction(berry::QtStyleManager::ThemeIcon(basePath + "dicom.svg"), window);
  }
  if (this->GetWindowConfigurer()->GetWindow()->GetWorkbench()->GetEditorRegistry()->FindEditor("org.mitk.editors.stdmultiwidget"))
  {
    openStdMultiWidgetEditorAction = new QmitkOpenStdMultiWidgetEditorAction(QIcon(":/org.mitk.gui.qt.ext/Editor.png"), window);
  }
  if (this->GetWindowConfigurer()->GetWindow()->GetWorkbench()->GetEditorRegistry()->FindEditor("org.mitk.editors.mxnmultiwidget"))
  {
    openMxNMultiWidgetEditorAction = new QmitkOpenMxNMultiWidgetEditorAction(QIcon(":/org.mitk.gui.qt.ext/Editor.png"), window);
  }

  if (imageNavigatorViewFound)
  {
    QObject::connect(imageNavigatorAction, SIGNAL(triggered(bool)), QmitkExtWorkbenchWindowAdvisorHack::undohack, SLOT(onImageNavigator()));
    imageNavigatorAction->setCheckable(true);

    // add part listener for image navigator
    imageNavigatorPartListener.reset(new PartListenerForImageNavigator(imageNavigatorAction));
    window->GetPartService()->AddPartListener(imageNavigatorPartListener.data());
    berry::IViewPart::Pointer imageNavigatorView = window->GetActivePage()->FindView("org.mitk.views.imagenavigator");
    imageNavigatorAction->setChecked(false);
    if (imageNavigatorView)
    {
      bool isImageNavigatorVisible = window->GetActivePage()->IsPartVisible(imageNavigatorView);
      if (isImageNavigatorVisible)
        imageNavigatorAction->setChecked(true);
    }
    imageNavigatorAction->setToolTip("Toggle image navigator for navigating through image");
  }

  viewNavigatorAction = new QAction(berry::QtStyleManager::ThemeIcon(QStringLiteral(":/org.mitk.gui.qt.ext/view-manager.svg")),"&View Navigator", nullptr);
  viewNavigatorFound = window->GetWorkbench()->GetViewRegistry()->Find("org.mitk.views.viewnavigatorview");
  if (viewNavigatorFound)
  {
    QObject::connect(viewNavigatorAction, SIGNAL(triggered(bool)), QmitkExtWorkbenchWindowAdvisorHack::undohack, SLOT(onViewNavigator()));
    viewNavigatorAction->setCheckable(true);

    // add part listener for view navigator
    viewNavigatorPartListener.reset(new PartListenerForViewNavigator(viewNavigatorAction));
    window->GetPartService()->AddPartListener(viewNavigatorPartListener.data());
    berry::IViewPart::Pointer viewnavigatorview = window->GetActivePage()->FindView("org.mitk.views.viewnavigatorview");
    viewNavigatorAction->setChecked(false);
    if (viewnavigatorview)
    {
      bool isViewNavigatorVisible = window->GetActivePage()->IsPartVisible(viewnavigatorview);
      if (isViewNavigatorVisible)
        viewNavigatorAction->setChecked(true);
    }
    viewNavigatorAction->setToolTip("Toggle View Navigator");
  }

  mainActionsToolBar->addAction(fileOpenAction);
  mainActionsToolBar->addAction(fileSaveProjectAction);
  mainActionsToolBar->addAction(closeProjectAction);
  mainActionsToolBar->addAction(undoAction);
  mainActionsToolBar->addAction(redoAction);
  if(this->GetWindowConfigurer()->GetWindow()->GetWorkbench()->GetEditorRegistry()->FindEditor("org.mitk.editors.dicomeditor"))
  {
    mainActionsToolBar->addAction(openDicomEditorAction);
  }
  if (this->GetWindowConfigurer()->GetWindow()->GetWorkbench()->GetEditorRegistry()->FindEditor("org.mitk.editors.stdmultiwidget"))
  {
    mainActionsToolBar->addAction(openStdMultiWidgetEditorAction);
  }
  if (this->GetWindowConfigurer()->GetWindow()->GetWorkbench()->GetEditorRegistry()->FindEditor("org.mitk.editors.mxnmultiwidget"))
  {
    mainActionsToolBar->addAction(openMxNMultiWidgetEditorAction);
  }

  if (imageNavigatorViewFound)
  {
    mainActionsToolBar->addAction(imageNavigatorAction);
  }
  if (viewNavigatorFound)
  {
    mainActionsToolBar->addAction(viewNavigatorAction);
  }
  mainWindow->addToolBar(mainActionsToolBar);

  // ==== Perspective Toolbar ==================================
  auto   qPerspectiveToolbar = new QToolBar;
  qPerspectiveToolbar->setObjectName("perspectiveToolBar");

  if (showPerspectiveToolbar)
  {
    qPerspectiveToolbar->addActions(perspGroup->actions());
    mainWindow->addToolBar(qPerspectiveToolbar);
  }
  else
    delete qPerspectiveToolbar;

  if (showViewToolbar)
  {
    auto prefService = berry::WorkbenchPlugin::GetDefault()->GetPreferencesService();
    berry::IPreferences::Pointer stylePrefs = prefService->GetSystemPreferences()->Node(berry::QtPreferences::QT_STYLES_NODE);
    bool showCategoryNames = stylePrefs->GetBool(berry::QtPreferences::QT_SHOW_TOOLBAR_CATEGORY_NAMES, true);

    // Order view descriptors by category

    QMultiMap<QString, berry::IViewDescriptor::Pointer> categoryViewDescriptorMap;

    for (auto labelViewDescriptorPair : VDMap)
    {
      auto viewDescriptor = labelViewDescriptorPair.second;
      auto category = !viewDescriptor->GetCategoryPath().isEmpty()
        ? viewDescriptor->GetCategoryPath().back()
        : QString();

      categoryViewDescriptorMap.insert(category, viewDescriptor);
    }

    // Create a separate toolbar for each category

    for (auto category : categoryViewDescriptorMap.uniqueKeys())
    {
      auto viewDescriptorsInCurrentCategory = categoryViewDescriptorMap.values(category);

      if (!viewDescriptorsInCurrentCategory.isEmpty())
      {
        auto toolbar = new QToolBar;
        toolbar->setObjectName(category + " View Toolbar");
        mainWindow->addToolBar(toolbar);

        if (showCategoryNames && !category.isEmpty())
        {
          auto categoryButton = new QToolButton;
          categoryButton->setToolButtonStyle(Qt::ToolButtonTextOnly);
          categoryButton->setText(category);
          categoryButton->setStyleSheet("background: transparent; margin: 0; padding: 0;");
          toolbar->addWidget(categoryButton);

          connect(categoryButton, &QToolButton::clicked, [toolbar]()
          {
            for (QWidget* widget : toolbar->findChildren<QWidget*>())
            {
              if (QStringLiteral("qt_toolbar_ext_button") == widget->objectName() && widget->isVisible())
              {
                QMouseEvent pressEvent(QEvent::MouseButtonPress, QPointF(0.0f, 0.0f), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
                QMouseEvent releaseEvent(QEvent::MouseButtonRelease, QPointF(0.0f, 0.0f), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
                QApplication::sendEvent(widget, &pressEvent);
                QApplication::sendEvent(widget, &releaseEvent);
              }
            }
          });
        }

        for (auto viewDescriptor : viewDescriptorsInCurrentCategory)
        {
          auto viewAction = new berry::QtShowViewAction(window, viewDescriptor);
          toolbar->addAction(viewAction);
        }
      }
    }
  }

  QSettings settings(GetQSettingsFile(), QSettings::IniFormat);
  mainWindow->restoreState(settings.value("ToolbarPosition").toByteArray());

  auto   qStatusBar = new QStatusBar();

  //creating a QmitkStatusBar for Output on the QStatusBar and connecting it with the MainStatusBar
  auto  statusBar = new QmitkStatusBar(qStatusBar);
  //disabling the SizeGrip in the lower right corner
  statusBar->SetSizeGripEnabled(false);

  auto  progBar = new QmitkProgressBar();

  qStatusBar->addPermanentWidget(progBar, 0);
  progBar->hide();
  // progBar->AddStepsToDo(2);
  // progBar->Progress(1);

  mainWindow->setStatusBar(qStatusBar);

  if (showMemoryIndicator)
  {
    auto   memoryIndicator = new QmitkMemoryUsageIndicatorView();
    qStatusBar->addPermanentWidget(memoryIndicator, 0);
  }
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

  menuPerspectiveListener.reset(new PerspectiveListenerForMenu(this));
  configurer->GetWindow()->AddPerspectiveListener(menuPerspectiveListener.data());

  configurer->AddEditorAreaTransfer(QStringList("text/uri-list"));
  configurer->ConfigureEditorAreaDropListener(dropTargetListener.data());
}

void QmitkExtWorkbenchWindowAdvisor::PostWindowOpen()
{
  berry::WorkbenchWindowAdvisor::PostWindowOpen();
  // Force Rendering Window Creation on startup.
  berry::IWorkbenchWindowConfigurer::Pointer configurer = GetWindowConfigurer();

  ctkPluginContext* context = QmitkCommonExtPlugin::getContext();
  ctkServiceReference serviceRef = context->getServiceReference<mitk::IDataStorageService>();
  if (serviceRef)
  {
    mitk::IDataStorageService *dsService = context->getService<mitk::IDataStorageService>(serviceRef);
    if (dsService)
    {
      mitk::IDataStorageReference::Pointer dsRef = dsService->GetDataStorage();
      mitk::DataStorageEditorInput::Pointer dsInput(new mitk::DataStorageEditorInput(dsRef));
      mitk::WorkbenchUtil::OpenEditor(configurer->GetWindow()->GetActivePage(),dsInput);
    }
  }
}

void QmitkExtWorkbenchWindowAdvisor::onIntro()
{
  QmitkExtWorkbenchWindowAdvisorHack::undohack->onIntro();
}

void QmitkExtWorkbenchWindowAdvisor::onHelp()
{
  QmitkExtWorkbenchWindowAdvisorHack::undohack->onHelp();
}

void QmitkExtWorkbenchWindowAdvisor::onHelpOpenHelpPerspective()
{
  QmitkExtWorkbenchWindowAdvisorHack::undohack->onHelpOpenHelpPerspective();
}

void QmitkExtWorkbenchWindowAdvisor::onAbout()
{
  QmitkExtWorkbenchWindowAdvisorHack::undohack->onAbout();
}

//--------------------------------------------------------------------------------
// Ugly hack from here on. Feel free to delete when command framework
// and undo buttons are done.
//--------------------------------------------------------------------------------

QmitkExtWorkbenchWindowAdvisorHack::QmitkExtWorkbenchWindowAdvisorHack()
  : QObject()
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
      mitk::VerboseLimitedLinearUndo::StackDescription descriptions = verboseundo->GetUndoDescriptions();
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
      mitk::VerboseLimitedLinearUndo::StackDescription descriptions = verboseundo->GetRedoDescriptions();
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

// safe calls to the complete chain
// berry::PlatformUI::GetWorkbench()->GetActiveWorkbenchWindow()->GetActivePage()->FindView("org.mitk.views.imagenavigator");
// to cover for all possible cases of closed pages etc.
static void SafeHandleNavigatorView(QString view_query_name)
{
  berry::IWorkbench* wbench = berry::PlatformUI::GetWorkbench();
  if( wbench == nullptr )
    return;

  berry::IWorkbenchWindow::Pointer wbench_window = wbench->GetActiveWorkbenchWindow();
  if( wbench_window.IsNull() )
    return;

  berry::IWorkbenchPage::Pointer wbench_page = wbench_window->GetActivePage();
  if( wbench_page.IsNull() )
    return;

  auto wbench_view = wbench_page->FindView( view_query_name );

  if( wbench_view.IsNotNull() )
  {
    bool isViewVisible = wbench_page->IsPartVisible( wbench_view );
    if( isViewVisible )
    {
      wbench_page->HideView( wbench_view );
      return;
    }

  }

  wbench_page->ShowView( view_query_name );
}

void QmitkExtWorkbenchWindowAdvisorHack::onImageNavigator()
{
  // show/hide ImageNavigatorView
  SafeHandleNavigatorView("org.mitk.views.imagenavigator");
}

void QmitkExtWorkbenchWindowAdvisorHack::onViewNavigator()
{
  // show/hide viewnavigatorView
  SafeHandleNavigatorView("org.mitk.views.viewnavigatorview");
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
  berry::IWorkbenchPage::Pointer page =
    berry::PlatformUI::GetWorkbench()->GetActiveWorkbenchWindow()->GetActivePage();
  page->ClosePerspective(page->GetPerspective(), true, true);
}

void QmitkExtWorkbenchWindowAdvisorHack::onNewWindow()
{
  berry::PlatformUI::GetWorkbench()->OpenWorkbenchWindow(nullptr);
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
    file.open(QIODevice::ReadOnly | QIODevice::Text); //text file only for reading

    QString text = QString(file.readAll());

    file.close();

    QString title = text;
    title.replace(reg, "");
    title.replace(reg2, "");

    std::cout << title.toStdString() << std::endl;

    QMessageBox::information(nullptr, title,
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
  if (context == nullptr)
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
  ctkEventAdmin* eventAdmin = nullptr;
  if (eventAdminRef)
  {
    eventAdmin = context->getService<ctkEventAdmin>(eventAdminRef);
  }
  if (eventAdmin == nullptr)
  {
    MITK_WARN << "ctkEventAdmin service not found. Unable to open context help";
  }
  else
  {
    ctkEvent ev("org/blueberry/ui/help/CONTEXTHELP_REQUESTED");
    eventAdmin->postEvent(ev);
  }
}

void QmitkExtWorkbenchWindowAdvisorHack::onHelpOpenHelpPerspective()
{
  berry::PlatformUI::GetWorkbench()->ShowPerspective("org.blueberry.perspectives.help",
    berry::PlatformUI::GetWorkbench()->GetActiveWorkbenchWindow());
}

void QmitkExtWorkbenchWindowAdvisorHack::onAbout()
{
  auto aboutDialog = new QmitkAboutDialog(QApplication::activeWindow(),nullptr);
  aboutDialog->open();
}

void QmitkExtWorkbenchWindowAdvisor::HookTitleUpdateListeners(berry::IWorkbenchWindowConfigurer::Pointer configurer)
{
  // hook up the listeners to update the window title
  titlePartListener.reset(new PartListenerForTitle(this));
  titlePerspectiveListener.reset(new PerspectiveListenerForTitle(this));
  editorPropertyListener.reset(new berry::PropertyChangeIntAdapter<
    QmitkExtWorkbenchWindowAdvisor>(this,
    &QmitkExtWorkbenchWindowAdvisor::PropertyChange));

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

  configurer->GetWindow()->AddPerspectiveListener(titlePerspectiveListener.data());
  configurer->GetWindow()->GetPartService()->AddPartListener(titlePartListener.data());
}

QString QmitkExtWorkbenchWindowAdvisor::ComputeTitle()
{
  berry::IWorkbenchWindowConfigurer::Pointer configurer = GetWindowConfigurer();
  berry::IWorkbenchPage::Pointer currentPage = configurer->GetWindow()->GetActivePage();
  berry::IEditorPart::Pointer activeEditor;
  if (currentPage)
  {
    activeEditor = lastActiveEditor.Lock();
  }

  QString title;
  berry::IProduct::Pointer product = berry::Platform::GetProduct();
  if (product.IsNotNull())
  {
    title = product->GetName();
  }
  if (title.isEmpty())
  {
    // instead of the product name, we use a custom variable for now
    title = productName;
  }

  if(showMitkVersionInfo)
  {
    QString mitkVersionInfo = MITK_REVISION_DESC;

    if(mitkVersionInfo.isEmpty())
      mitkVersionInfo = MITK_VERSION_STRING;

    title += " " + mitkVersionInfo;
  }

  if (showVersionInfo)
  {
    // add version informatioin
    QString versions = QString(" (ITK %1.%2.%3 | VTK %4.%5.%6 | Qt %7)")
      .arg(ITK_VERSION_MAJOR).arg(ITK_VERSION_MINOR).arg(ITK_VERSION_PATCH)
      .arg(VTK_MAJOR_VERSION).arg(VTK_MINOR_VERSION).arg(VTK_BUILD_VERSION)
      .arg(QT_VERSION_STR);

    title += versions;
  }

  if (currentPage)
  {
    if (activeEditor)
    {
      lastEditorTitle = activeEditor->GetTitleToolTip();
      if (!lastEditorTitle.isEmpty())
        title = lastEditorTitle + " - " + title;
    }
    berry::IPerspectiveDescriptor::Pointer persp = currentPage->GetPerspective();
    QString label = "";
    if (persp)
    {
      label = persp->GetLabel();
    }
    berry::IAdaptable* input = currentPage->GetInput();
    if (input && input != wbAdvisor->GetDefaultPageInput())
    {
      label = currentPage->GetLabel();
    }
    if (!label.isEmpty())
    {
      title = label + " - " + title;
    }
  }

  title += " (Not for use in diagnosis or treatment of patients)";

  return title;
}

void QmitkExtWorkbenchWindowAdvisor::RecomputeTitle()
{
  berry::IWorkbenchWindowConfigurer::Pointer configurer = GetWindowConfigurer();
  QString oldTitle = configurer->GetTitle();
  QString newTitle = ComputeTitle();
  if (newTitle != oldTitle)
  {
    configurer->SetTitle(newTitle);
  }
}

void QmitkExtWorkbenchWindowAdvisor::UpdateTitle(bool editorHidden)
{
  berry::IWorkbenchWindowConfigurer::Pointer configurer = GetWindowConfigurer();
  berry::IWorkbenchWindow::Pointer window = configurer->GetWindow();
  berry::IEditorPart::Pointer activeEditor;
  berry::IWorkbenchPage::Pointer currentPage = window->GetActivePage();
  berry::IPerspectiveDescriptor::Pointer persp;
  berry::IAdaptable* input = nullptr;

  if (currentPage)
  {
    activeEditor = currentPage->GetActiveEditor();
    persp = currentPage->GetPerspective();
    input = currentPage->GetInput();
  }

  if (editorHidden)
  {
    activeEditor = nullptr;
  }

  // Nothing to do if the editor hasn't changed
  if (activeEditor == lastActiveEditor.Lock() && currentPage == lastActivePage.Lock()
    && persp == lastPerspective.Lock() && input == lastInput)
  {
    return;
  }

  if (!lastActiveEditor.Expired())
  {
    lastActiveEditor.Lock()->RemovePropertyListener(editorPropertyListener.data());
  }

  lastActiveEditor = activeEditor;
  lastActivePage = currentPage;
  lastPerspective = persp;
  lastInput = input;

  if (activeEditor)
  {
    activeEditor->AddPropertyListener(editorPropertyListener.data());
  }

  RecomputeTitle();
}

void QmitkExtWorkbenchWindowAdvisor::PropertyChange(const berry::Object::Pointer& /*source*/, int propId)
{
  if (propId == berry::IWorkbenchPartConstants::PROP_TITLE)
  {
    if (!lastActiveEditor.Expired())
    {
      QString newTitle = lastActiveEditor.Lock()->GetPartName();
      if (lastEditorTitle != newTitle)
      {
        RecomputeTitle();
      }
    }
  }
}

void QmitkExtWorkbenchWindowAdvisor::SetPerspectiveExcludeList(const QList<QString>& v)
{
  this->perspectiveExcludeList = v;
}

QList<QString> QmitkExtWorkbenchWindowAdvisor::GetPerspectiveExcludeList()
{
  return this->perspectiveExcludeList;
}

void QmitkExtWorkbenchWindowAdvisor::SetViewExcludeList(const QList<QString>& v)
{
  this->viewExcludeList = v;
}

QList<QString> QmitkExtWorkbenchWindowAdvisor::GetViewExcludeList()
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
