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

#include <memory>

class QmitkAgentSession;
class QmitkAgentWidget;
class QTimer;

/**
 * \brief Chat with an agent that operates the Workbench.
 *
 * Shows a QmitkAgentWidget for a QmitkAgentSession, which lasts as long as
 * the view. Locks the other views and the editors while the agent works.
 */
class QmitkAgentView : public QmitkAbstractView
{
  Q_OBJECT

public:
  static const std::string VIEW_ID;

  QmitkAgentView();
  ~QmitkAgentView() override;

  void CreateQtPartControl(QWidget* parent) override;
  void SetFocus() override;

protected:
  void OnPreferencesChanged(const mitk::IPreferences* preferences) override;

private:
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
};

#endif
