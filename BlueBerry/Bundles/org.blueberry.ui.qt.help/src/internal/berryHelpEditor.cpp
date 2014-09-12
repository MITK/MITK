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

#include "berryHelpEditor.h"

#include "berryHelpEditorInput.h"
#include "berryHelpPluginActivator.h"
#include "berryHelpPerspective.h"
#include "berryHelpWebView.h"
#include "berryQHelpEngineWrapper.h"
#include "berryHelpEditorFindWidget.h"
#include "berryHelpPluginActivator.h"
#include "berryQHelpEngineWrapper.h"

#include <berryUIException.h>
#include <berryPlatformUI.h>
#include <berryIWorkbenchPage.h>
#include <berryIWorkbenchPartConstants.h>

#include <QToolBar>
#include <QHelpEngine>
#include <QVBoxLayout>

namespace berry {

const std::string HelpEditor::EDITOR_ID = "org.blueberry.editors.help";

HelpEditor::HelpEditor()
  : m_ToolBar(0)
  , m_WebView(0)
{

}

HelpEditor::~HelpEditor()
{
  // we need to wrap the RemovePartListener call inside a
  // register/unregister block to prevent infinite recursion
  // due to the destruction of temporary smartpointer to this
  this->Register();
  this->GetSite()->GetPage()->RemovePartListener(IPartListener::Pointer(this));
  this->GetSite()->GetPage()->GetWorkbenchWindow()->RemovePerspectiveListener(IPerspectiveListener::Pointer(this));
  this->UnRegister(false);
}

void HelpEditor::Init(berry::IEditorSite::Pointer site, berry::IEditorInput::Pointer input)
{
  if (input.Cast<HelpEditorInput>().IsNull())
     throw PartInitException("Invalid Input: Must be berry::HelpEditorInput");

  this->SetSite(site);
  site->GetPage()->AddPartListener(IPartListener::Pointer(this));
  site->GetPage()->GetWorkbenchWindow()->AddPerspectiveListener(IPerspectiveListener::Pointer(this));

  m_WebView = new HelpWebView(site, 0);

  connect(m_WebView, SIGNAL(loadFinished(bool)), this, SLOT(InitializeTitle()));

  this->DoSetInput(input);
}

void HelpEditor::CreateQtPartControl(QWidget* parent)
{
  QVBoxLayout* verticalLayout = new QVBoxLayout(parent);
  verticalLayout->setSpacing(0);
  verticalLayout->setContentsMargins(0, 0, 0, 0);

  m_ToolBar = new QToolBar(parent);
  m_ToolBar->setMaximumHeight(32);
  verticalLayout->addWidget(m_ToolBar);

  m_WebView->setParent(parent);
  m_WebView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  verticalLayout->addWidget(m_WebView);

  m_FindWidget = new HelpEditorFindWidget(parent);
  m_FindWidget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Maximum);
  verticalLayout->addWidget(m_FindWidget);
  m_FindWidget->hide();

  connect(m_FindWidget, SIGNAL(findNext()), this, SLOT(findNext()));
  connect(m_FindWidget, SIGNAL(findPrevious()), this, SLOT(findPrevious()));
  connect(m_FindWidget, SIGNAL(find(QString, bool)), this,
          SLOT(find(QString, bool)));
  connect(m_FindWidget, SIGNAL(escapePressed()), m_WebView, SLOT(setFocus()));

  // Fill the editor toolbar
  m_BackAction = m_ToolBar->addAction(QIcon(":/org.blueberry.ui.qt.help/go-previous.png"), "Go back",
                                             m_WebView, SLOT(backward()));
  m_ForwardAction = m_ToolBar->addAction(QIcon(":/org.blueberry.ui.qt.help/go-next.png"), "Go forward",
                                                m_WebView, SLOT(forward()));
  m_HomeAction = m_ToolBar->addAction(QIcon(":/org.blueberry.ui.qt.help/go-home.png"), "Go home",
                                      m_WebView, SLOT(home()));
  m_PrintAction = m_ToolBar->addAction(QIcon(":/org.blueberry.ui.qt.help/print.png"), "Print",
    m_WebView, SLOT(print()));

  m_ToolBar->addSeparator();

  m_FindAction = m_ToolBar->addAction(QIcon(":/org.blueberry.ui.qt.help/find.png"), "Find in text",
                                             this, SLOT(ShowTextSearch()));

  m_ToolBar->addSeparator();

  m_ZoomIn = m_ToolBar->addAction(QIcon(":/org.blueberry.ui.qt.help/zoom-in.png"), "Zoom in", m_WebView, SLOT(scaleUp()));
  m_ZoomOut = m_ToolBar->addAction(QIcon(":/org.blueberry.ui.qt.help/zoom-out.png"), "Zoom out", m_WebView, SLOT(scaleDown()));

  m_ToolBar->addSeparator();

  m_OpenHelpMode = m_ToolBar->addAction("Open Help Perspective", this, SLOT(OpenHelpPerspective()));
  m_CloseHelpMode = m_ToolBar->addAction("Close Help Perspective", this, SLOT(CloseHelpPerspective()));
  IPerspectiveDescriptor::Pointer currPersp = this->GetSite()->GetPage()->GetPerspective();
  m_OpenHelpMode->setVisible(!(currPersp.IsNotNull() && currPersp->GetId() == HelpPerspective::ID));
  m_CloseHelpMode->setVisible((currPersp.IsNotNull() && currPersp->GetId() == HelpPerspective::ID));

  connect(m_WebView, SIGNAL(backwardAvailable(bool)), m_BackAction, SLOT(setEnabled(bool)));
  connect(m_WebView, SIGNAL(forwardAvailable(bool)), m_ForwardAction, SLOT(setEnabled(bool)));

  m_BackAction->setEnabled(false);
  m_ForwardAction->setEnabled(false);
  m_HomeAction->setEnabled(!HelpPluginActivator::getInstance()->getQHelpEngine().homePage().isEmpty());

  connect(&HelpPluginActivator::getInstance()->getQHelpEngine(), SIGNAL(homePageChanged(QString)),
          this, SLOT(HomePageChanged(QString)));
}

void HelpEditor::DoSetInput(IEditorInput::Pointer input)
{
  if (input.IsNull())
  {
    // close editor
    class CloseEditorRunnable : public Poco::Runnable
    {
    private:
      IEditorPart::Pointer editor;

    public:
      CloseEditorRunnable(IEditorPart::Pointer editor)
        : editor(editor)
      {}

      void run()
      {
        editor->GetSite()->GetPage()->CloseEditor(editor, false);
        delete this;
      }
    };

    Display::GetDefault()->AsyncExec(new CloseEditorRunnable(IEditorPart::Pointer(this)));
  }
  else
  {
    // an empty url represents the home page
    HelpEditorInput::Pointer helpInput = input.Cast<HelpEditorInput>();
    QString currHomePage = HelpPluginActivator::getInstance()->getQHelpEngine().homePage();
    if (helpInput->GetUrl().isEmpty() && !currHomePage.isEmpty())
    {
      helpInput = HelpEditorInput::Pointer(new HelpEditorInput(currHomePage));
    }
    QtEditorPart::SetInput(helpInput);
    m_WebView->setSource(helpInput->GetUrl());
  }
}

void HelpEditor::SetInputWithNotify(IEditorInput::Pointer input)
{
  DoSetInput(input);
  FirePropertyChange(IWorkbenchPartConstants::PROP_INPUT);
}

void HelpEditor::SetInput(IEditorInput::Pointer input)
{
  SetInputWithNotify(input);
}

void HelpEditor::HomePageChanged(const QString &page)
{
  if (page.isEmpty())
  {
    m_HomeAction->setEnabled(false);
  }

  m_HomeAction->setEnabled(true);
  if (this->GetEditorInput().Cast<HelpEditorInput>()->GetUrl().isEmpty())
  {
    IEditorInput::Pointer newInput(new HelpEditorInput(page));
    DoSetInput(newInput);
  }
}

void HelpEditor::OpenHelpPerspective()
{
  PlatformUI::GetWorkbench()->ShowPerspective(HelpPerspective::ID, this->GetSite()->GetPage()->GetWorkbenchWindow());
}

void HelpEditor::CloseHelpPerspective()
{
  berry::IWorkbenchPage::Pointer
    page =
    berry::PlatformUI::GetWorkbench()->GetActiveWorkbenchWindow()->GetActivePage();
  page->ClosePerspective(page->GetPerspective(), true, true);
}

void HelpEditor::InitializeTitle()
{
  std::string title = m_WebView->title().toStdString();
  this->SetPartName(title);
}

void HelpEditor::ShowTextSearch()
{
  m_FindWidget->show();
}

void HelpEditor::SetFocus()
{
  m_WebView->setFocus();

  enableShortcuts();
}

QWebPage *HelpEditor::GetQWebPage() const
{
  return m_WebView->page();
}

IPartListener::Events::Types HelpEditor::GetPartEventTypes() const
{
  return IPartListener::Events::DEACTIVATED;
}

void HelpEditor::PartDeactivated(IWorkbenchPartReference::Pointer partRef)
{
  if (partRef == GetSite()->GetPage()->GetReference(IWorkbenchPart::Pointer(this)))
    disableShortcuts();
}

IPerspectiveListener::Events::Types HelpEditor::GetPerspectiveEventTypes() const
{
  return IPerspectiveListener::Events::ACTIVATED | IPerspectiveListener::Events::DEACTIVATED;
}

void HelpEditor::PerspectiveActivated(SmartPointer<IWorkbenchPage> page, IPerspectiveDescriptor::Pointer perspective)
{
  if (perspective->GetId() == HelpPerspective::ID)
  {
    m_OpenHelpMode->setVisible(false);
    m_CloseHelpMode->setVisible(true);
  }
}

void HelpEditor::PerspectiveDeactivated(SmartPointer<IWorkbenchPage> page, IPerspectiveDescriptor::Pointer perspective)
{
  if (perspective->GetId() == HelpPerspective::ID)
  {
    m_OpenHelpMode->setVisible(true);
    m_CloseHelpMode->setVisible(false);
  }
}

void HelpEditor::findNext()
{
  find(m_FindWidget->text(), true);
}

void HelpEditor::findPrevious()
{
  find(m_FindWidget->text(), false);
}

void HelpEditor::find(const QString &ttf, bool forward)
{
  bool found = findInWebPage(ttf, forward);

  if (!found && ttf.isEmpty())
    found = true;   // the line edit is empty, no need to mark it red...

  if (!m_FindWidget->isVisible())
    m_FindWidget->show();
  m_FindWidget->setPalette(found);
}

bool HelpEditor::findInWebPage(const QString &ttf, bool forward)
{
  bool found = false;
  QWebPage::FindFlags options;
  if (!ttf.isEmpty())
  {
    if (!forward)
      options |= QWebPage::FindBackward;

    if (m_FindWidget->caseSensitive())
      options |= QWebPage::FindCaseSensitively;

    found = m_WebView->findText(ttf, options);

    if (!found)
    {
      options |= QWebPage::FindWrapsAroundDocument;
      found = m_WebView->findText(ttf, options);
    }
  }
  // force highlighting of all other matches, also when empty (clear)
  options = QWebPage::HighlightAllOccurrences;
  if (m_FindWidget->caseSensitive())
    options |= QWebPage::FindCaseSensitively;
  m_WebView->findText(QLatin1String(""), options);
  m_WebView->findText(ttf, options);
  return found;
}

void HelpEditor::enableShortcuts()
{
  m_BackAction->setShortcut(QKeySequence::Back);
  m_ForwardAction->setShortcut(QKeySequence::Forward);
  m_FindAction->setShortcut(QKeySequence::Find);
  m_ZoomIn->setShortcut(QKeySequence::ZoomIn);
  m_ZoomOut->setShortcut(QKeySequence::ZoomOut);
}

void HelpEditor::disableShortcuts()
{
  m_BackAction->setShortcut(QKeySequence());
  m_ForwardAction->setShortcut(QKeySequence());
  m_FindAction->setShortcut(QKeySequence());
  m_ZoomIn->setShortcut(QKeySequence());
  m_ZoomOut->setShortcut(QKeySequence());
}

}
