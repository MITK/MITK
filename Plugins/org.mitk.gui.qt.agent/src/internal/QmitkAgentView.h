/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkAgentView_h
#define QmitkAgentView_h

#include <QmitkAbstractView.h>

#include <berryISaveablePart.h>
#include <berryIWorkbenchListener.h>

#include <memory>

class QmitkAgentSession;
class QmitkAgentWidget;
class QTimer;

/**
 * \brief Chat with an agent that operates the Workbench.
 *
 * Shows a QmitkAgentWidget for a QmitkAgentSession, which lasts as long as
 * the view. Locks the other views and the editors while the agent works.
 * Closing the view, resetting the perspective and closing the Workbench ask
 * whether to stop the agent while it works. A turn counts as unsaved content
 * for that purpose; there is nothing to save.
 */
class QmitkAgentView : public QmitkAbstractView, public berry::IWorkbenchListener, public berry::ISaveablePart
{
  Q_OBJECT

public:
  static const std::string VIEW_ID;

  QmitkAgentView();
  ~QmitkAgentView() override;

  void CreateQtPartControl(QWidget* parent) override;
  void SetFocus() override;

  /** \brief Asks whether to stop a working agent; a no keeps the Workbench open. */
  bool PreShutdown(berry::IWorkbench* workbench, bool forced) override;

  /** \brief Whether a turn runs and the user has not agreed to close the view. */
  bool IsDirty() const override;

  /** \brief Asks whether to stop a working agent; a yes closes the view, a no keeps it. */
  CloseAnswer PromptToSaveOnClose() override;

protected:
  void OnPreferencesChanged(const mitk::IPreferences* preferences) override;

private:
  /**
   * \brief Ask whether to stop the working agent, unless the user agreed already.
   *
   * The consent holds until the turn ends, so that one close does not ask
   * twice, for example when the Workbench closes and then its views.
   *
   * \return Whether the user agrees, or no turn runs.
   */
  bool ConfirmStopping(const QString& question);

  /**
   * \brief Locks the other views and the editors while the agent works.
   *
   * The lock lasts until the agent's turns have ended and its tool calls no
   * longer run: an interrupted turn ends before the call it interrupted.
   */
  void UpdateLock();

  /**
   * \brief Let the user see the answer when the agent's turns end.
   *
   * Shows the view again if the agent's actions covered it with another
   * view of its tab group, for example by opening the Data Manager.
   */
  void BringToTop();

  std::unique_ptr<QmitkAgentSession> m_Session;
  QmitkAgentWidget* m_Widget;

  /** \brief Looks again whether the views can be unlocked while tool calls run after the turns ended. */
  QTimer* m_UnlockTimer;

  /** \brief Whether this view locked the parts, as only then it may unlock them. */
  bool m_HoldsLock;

  /** \brief Whether the user agreed to stop the working agent for the close that is under way. */
  bool m_StopAgreed;
};

#endif
