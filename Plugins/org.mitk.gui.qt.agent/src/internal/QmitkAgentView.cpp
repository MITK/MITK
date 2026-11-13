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

#include <QTimer>
#include <QVBoxLayout>

const std::string QmitkAgentView::VIEW_ID = "org.mitk.views.agent";

namespace
{
  /** Start a plugin now that would otherwise start only when one of its classes is used. */
  void StartPlugin(const QString& symbolicName)
  {
    auto* context = QmitkAgentPluginActivator::GetContext();

    if (context == nullptr)
      return;

    for (const auto& plugin : context->getPlugins())
    {
      if (plugin->getSymbolicName() != symbolicName || plugin->getState() == ctkPlugin::ACTIVE)
        continue;

      try
      {
        plugin->start(ctkPlugin::START_TRANSIENT);
      }
      // A failed activation arrives as a ctkRuntimeException, not as a
      // ctkPluginException.
      catch (const ctkException& e)
      {
        MITK_WARN << "Cannot start " << symbolicName.toStdString() << ": " << e.what();
      }
    }
  }
}

QmitkAgentView::QmitkAgentView()
  : m_Widget(nullptr),
    m_UnlockTimer(nullptr),
    m_HoldsLock(false)
{
}

QmitkAgentView::~QmitkAgentView()
{
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
    connect(activator, &QmitkAgentPluginActivator::ClaudeCodeSignedIn, m_Session.get(), &QmitkAgentSession::CheckClaudeCodeAgain);

  this->UpdateLock();

  // Later, as offering the tools may ask the user, which a view should not do
  // while it is being created, at startup for example.
  QTimer::singleShot(0, this, [this]() {
    if (m_Widget->OfferTools())
    {
      // These plugins add the endpoints of the Workbench, of segmentation, of
      // image statistics and of reports when they start, which they otherwise
      // do only with one of their views or editors. They must not start
      // eagerly with the application: those that require
      // org.mitk.gui.qt.common would start it before the workbench runs,
      // which leaves the views without their lifecycle notifications.
      StartPlugin("org.mitk.gui.qt.restapi");
      StartPlugin("org.mitk.gui.qt.segmentation");
      StartPlugin("org.mitk.gui.qt.measurementtoolbox");
      StartPlugin("org.mitk.gui.qt.reports");
    }

    m_Session->CheckClaudeCode();
  });
}

void QmitkAgentView::SetFocus()
{
  m_Widget->setFocus();
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
