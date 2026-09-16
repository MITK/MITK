/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryHelpEditor.h"

#include "berryHelpEditorInput.h"
#include "berryHelpPluginActivator.h"
#include "berryHelpWebView.h"
#include "berryQHelpEngineWrapper.h"

#include <berryUIException.h>
#include <berryPlatformUI.h>
#include <berryIWorkbenchPage.h>
#include <berryIWorkbenchPartConstants.h>

#include <QToolBar>
#include <QHelpEngine>
#include <QVBoxLayout>

namespace berry {

const QString HelpEditor::EDITOR_ID = "org.blueberry.editors.help";

HelpEditor::HelpEditor()
  : m_ToolBar(nullptr)
  , m_WebView(nullptr)
{

}

HelpEditor::~HelpEditor()
{
  this->GetSite()->GetPage()->RemovePartListener(this);
}

void HelpEditor::Init(berry::IEditorSite::Pointer site, berry::IEditorInput::Pointer input)
{
  if (input.Cast<HelpEditorInput>().IsNull())
     throw PartInitException("Invalid Input: Must be berry::HelpEditorInput");

  this->SetSite(site);
  site->GetPage()->AddPartListener(this);

  m_WebView = new HelpWebView(site, nullptr);

  connect(m_WebView, SIGNAL(sourceChanged(QUrl)), this, SLOT(InitializeTitle()));
  connect(m_WebView, SIGNAL(sourceChanged(QUrl)),
          &HelpPluginActivator::getInstance()->getQHelpEngine(), SIGNAL(currentPageChanged(QUrl)));

  this->DoSetInput(input);
}

void HelpEditor::CreateQtPartControl(QWidget* parent)
{
  auto   verticalLayout = new QVBoxLayout(parent);
  verticalLayout->setSpacing(0);
  verticalLayout->setContentsMargins(0, 0, 0, 0);

  m_ToolBar = new QToolBar(parent);
  m_ToolBar->setMaximumHeight(32);
  verticalLayout->addWidget(m_ToolBar);

  m_WebView->setParent(parent);
  m_WebView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  verticalLayout->addWidget(m_WebView);

  // Fill the editor toolbar
  m_BackAction = m_ToolBar->addAction(QIcon(":/org.blueberry.ui.qt.help/go-previous.png"), "Go back",
                                             m_WebView, SLOT(backward()));
  m_ForwardAction = m_ToolBar->addAction(QIcon(":/org.blueberry.ui.qt.help/go-next.png"), "Go forward",
                                                m_WebView, SLOT(forward()));

  m_ToolBar->addSeparator();

  m_ZoomIn = m_ToolBar->addAction(QIcon(":/org.blueberry.ui.qt.help/zoom-in.png"), "Zoom in", m_WebView, SLOT(scaleUp()));
  m_ZoomOut = m_ToolBar->addAction(QIcon(":/org.blueberry.ui.qt.help/zoom-out.png"), "Zoom out", m_WebView, SLOT(scaleDown()));

  connect(m_WebView, SIGNAL(backwardAvailable(bool)), m_BackAction, SLOT(setEnabled(bool)));
  connect(m_WebView, SIGNAL(forwardAvailable(bool)), m_ForwardAction, SLOT(setEnabled(bool)));

  m_BackAction->setEnabled(false);
  m_ForwardAction->setEnabled(false);

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

      void run() override
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
  if (this->GetEditorInput().Cast<HelpEditorInput>()->GetUrl().isEmpty())
  {
    IEditorInput::Pointer newInput(new HelpEditorInput(page));
    DoSetInput(newInput);
  }
}

void HelpEditor::InitializeTitle()
{
  QString title = m_WebView->documentTitle();
  this->SetPartName(title);
}

void HelpEditor::SetFocus()
{
  m_WebView->setFocus();

  enableShortcuts();
}

IPartListener::Events::Types HelpEditor::GetPartEventTypes() const
{
  return IPartListener::Events::DEACTIVATED;
}

void HelpEditor::PartDeactivated(const IWorkbenchPartReference::Pointer& partRef)
{
  if (partRef == GetSite()->GetPage()->GetReference(IWorkbenchPart::Pointer(this)))
    disableShortcuts();
}

void HelpEditor::enableShortcuts()
{
  m_BackAction->setShortcut(QKeySequence::Back);
  m_ForwardAction->setShortcut(QKeySequence::Forward);
  m_ZoomIn->setShortcut(QKeySequence::ZoomIn);
  m_ZoomOut->setShortcut(QKeySequence::ZoomOut);
}

void HelpEditor::disableShortcuts()
{
  m_BackAction->setShortcut(QKeySequence());
  m_ForwardAction->setShortcut(QKeySequence());
  m_ZoomIn->setShortcut(QKeySequence());
  m_ZoomOut->setShortcut(QKeySequence());
}

}
