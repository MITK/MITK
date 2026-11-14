/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkAgentView.h"
#include "QmitkAgentPluginActivator.h"

#include <QmitkAgentSession.h>
#include <QmitkAgentToolServer.h>
#include <QmitkAgentWidget.h>

#include <ctkException.h>
#include <ctkPlugin.h>
#include <ctkPluginContext.h>

#include <mitkLog.h>

#include <QmitkPartLock.h>

#include <berryIWorkbenchPage.h>
#include <berryPlatformUI.h>

#include <QMessageBox>
#include <QTimer>
#include <QVBoxLayout>

const std::string QmitkAgentView::VIEW_ID = "org.mitk.views.agent";

namespace
{
  /**
   * Start a plugin now that would otherwise start only when one of its classes is used.
   *
   * \return Whether the plugin is part of this build and started.
   */
  bool StartPlugin(const QString& symbolicName)
  {
    auto* context = QmitkAgentPluginActivator::GetContext();

    if (context == nullptr)
      return false;

    for (const auto& plugin : context->getPlugins())
    {
      if (plugin->getSymbolicName() != symbolicName)
        continue;

      if (plugin->getState() == ctkPlugin::ACTIVE)
        return true;

      try
      {
        plugin->start(ctkPlugin::START_TRANSIENT);
        return true;
      }
      // A failed activation arrives as a ctkRuntimeException, not as a
      // ctkPluginException.
      catch (const ctkException& e)
      {
        MITK_WARN << "Cannot start " << symbolicName.toStdString() << ": " << e.what();
        return false;
      }
    }

    MITK_WARN << "Cannot start " << symbolicName.toStdString() << ": it is not part of this build.";
    return false;
  }
}

QmitkAgentView::QmitkAgentView()
  : m_Widget(nullptr),
    m_UnlockTimer(nullptr),
    m_HoldsLock(false),
    m_StopAgreed(false)
{
  berry::PlatformUI::GetWorkbench()->AddWorkbenchListener(this);
}

QmitkAgentView::~QmitkAgentView()
{
  berry::PlatformUI::GetWorkbench()->RemoveWorkbenchListener(this);

  // Ending the session withdraws the tools, which runs the event loop, in
  // which neither the timer nor a signal of the session must reach this view:
  // m_Session is null by then.
  if (m_UnlockTimer != nullptr)
    m_UnlockTimer->stop();

  if (m_Session != nullptr)
    m_Session->disconnect(this);

  m_Session.reset();

  if (m_HoldsLock)
    QmitkPartLock::GetInstance()->Unlock();
}

void QmitkAgentView::CreateQtPartControl(QWidget* parent)
{
  m_Session = std::make_unique<QmitkAgentSession>();
  m_Widget = new QmitkAgentWidget(m_Session.get(), this->GetPreferences(), parent);

  auto* layout = new QVBoxLayout(parent);
  layout->addWidget(m_Widget);

  m_UnlockTimer = new QTimer(this);
  m_UnlockTimer->setInterval(100);

  connect(m_UnlockTimer, &QTimer::timeout, this, &QmitkAgentView::UpdateLock);
  connect(m_Session.get(), &QmitkAgentSession::TurnStateChanged, this, &QmitkAgentView::UpdateLock);
  connect(m_Session.get(), &QmitkAgentSession::TurnsFinished, this, &QmitkAgentView::BringToTop);

  if (auto* activator = QmitkAgentPluginActivator::GetInstance(); activator != nullptr)
    connect(activator, &QmitkAgentPluginActivator::ClaudeCodeAccountChanged, m_Session.get(), &QmitkAgentSession::CheckClaudeCodeAgain);

  this->UpdateLock();

  // These plugins add the endpoints of the Workbench, of segmentation, of
  // image statistics and of reports when they start, which they otherwise do
  // only with one of their views or editors. They must not start eagerly with
  // the application: those that require org.mitk.gui.qt.common would start it
  // before the workbench runs, which leaves the views without their lifecycle
  // notifications. So they start once the tools are offered, which happens
  // right after the view was created at the earliest.
  connect(m_Widget, &QmitkAgentWidget::ToolsOffered, this, [this]() {
    StartPlugin("org.mitk.gui.qt.restapi");
    StartPlugin("org.mitk.gui.qt.reports");

    if (!StartPlugin("org.mitk.gui.qt.segmentation"))
      m_Widget->AddNote("This build of MITK has no segmentation, so the agent cannot segment.");

    if (!StartPlugin("org.mitk.gui.qt.measurementtoolbox"))
      m_Widget->AddNote("This build of MITK has no measurement tools, so the agent cannot measure or draw figures.");
  });

  m_Session->CheckClaudeCode();
}

void QmitkAgentView::SetFocus()
{
  m_Widget->setFocus();
}

bool QmitkAgentView::PreShutdown(berry::IWorkbench*, bool forced)
{
  return forced || this->ConfirmStopping("The agent is working. Stop it and close the Workbench?");
}

bool QmitkAgentView::IsDirty() const
{
  return m_Session != nullptr && m_Session->IsTurnRunning() && !m_StopAgreed;
}

berry::ISaveablePart::CloseAnswer QmitkAgentView::PromptToSaveOnClose()
{
  return this->ConfirmStopping("The agent is working. Stop it and close?") ? CloseAnswer::Discard : CloseAnswer::Cancel;
}

bool QmitkAgentView::ConfirmStopping(const QString& question)
{
  if (m_Session == nullptr || !m_Session->IsTurnRunning() || m_StopAgreed)
    return true;

  const auto answer = QMessageBox::question(m_Widget, "Agent", question,
    QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

  m_StopAgreed = answer == QMessageBox::Yes;
  return m_StopAgreed;
}

void QmitkAgentView::OnPreferencesChanged(const mitk::IPreferences*)
{
  if (m_Widget != nullptr)
    m_Widget->OnPreferencesChanged();
}

void QmitkAgentView::UpdateLock()
{
  auto* lock = QmitkPartLock::GetInstance();
  const bool turnRunning = m_Session->IsTurnRunning();
  const bool callsRunning = m_Session->GetToolServer().GetRunningCallCount() > 0;

  if (!turnRunning)
    m_StopAgreed = false;

  if (!turnRunning && !callsRunning)
  {
    m_UnlockTimer->stop();

    if (m_HoldsLock)
    {
      m_HoldsLock = false;
      lock->Unlock();
    }

    return;
  }

  lock->Lock(QString::fromStdString(VIEW_ID), "Locked while the agent works. You can stop it in the Agent view.");
  m_HoldsLock = true;

  if (!turnRunning)
    m_UnlockTimer->start();
}

void QmitkAgentView::BringToTop()
{
  const auto page = this->GetSite()->GetPage();
  const berry::IWorkbenchPart::Pointer part(this);

  if (page.IsNotNull() && !page->IsPartVisible(part))
    page->BringToTop(part);
}
