/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical Image Computing.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "QmitkInteractionSchemeToolBar.h"

#include <QActionGroup>

QmitkInteractionSchemeToolBar::QmitkInteractionSchemeToolBar(QWidget* parent/* = nullptr*/)
  : QToolBar(parent)
  , m_ActionGroup(new QActionGroup(this))
  , m_InteractionSchemeSwitcher(nullptr)
  , m_InteractionEventHandler(nullptr)
{
  QToolBar::setOrientation(Qt::Vertical);
  QToolBar::setIconSize(QSize(17, 17));
  m_ActionGroup->setExclusive(true); // only one selectable action

  AddButton(InteractionScheme::PACSStandard, tr("Pointer"), QIcon(":/Qmitk/mm_pointer.png"), true);
  AddButton(InteractionScheme::PACSLevelWindow, tr("Level/Window"), QIcon(":/Qmitk/mm_contrast.png"));
  AddButton(InteractionScheme::PACSPan, tr("Pan"), QIcon(":/Qmitk/mm_pan.png"));
  AddButton(InteractionScheme::PACSScroll, tr("Scroll"), QIcon(":/Qmitk/mm_scroll.png"));
  AddButton(InteractionScheme::PACSZoom, tr("Zoom"), QIcon(":/Qmitk/mm_zoom.png"));

  m_InteractionSchemeSwitcher = mitk::InteractionSchemeSwitcher::New();
}

void QmitkInteractionSchemeToolBar::SetInteractionEventHandler(mitk::InteractionEventHandler::Pointer interactionEventHandler)
{
  if (interactionEventHandler == m_InteractionEventHandler)
  {
    return;
  }

  m_InteractionEventHandler = interactionEventHandler;
  if (nullptr != m_InteractionEventHandler)
  {
    m_InteractionSchemeSwitcher->SetInteractionScheme(m_InteractionEventHandler, InteractionScheme::PACSStandard);
  }
}

void QmitkInteractionSchemeToolBar::AddButton(InteractionScheme interactionScheme, const QString& toolName, const QIcon& icon, bool on)
{
  QAction* action = new QAction(icon, toolName, this);
  action->setCheckable(true);
  action->setActionGroup(m_ActionGroup);
  action->setChecked(on);
  action->setData(interactionScheme);
  connect(action, SIGNAL(triggered()), this, SLOT(OnInteractionSchemeChanged()));
  QToolBar::addAction(action);
}

QmitkInteractionSchemeToolBar::~QmitkInteractionSchemeToolBar()
{
  // nothing here
}

void QmitkInteractionSchemeToolBar::OnInteractionSchemeChanged()
{
  QAction* action = dynamic_cast<QAction*>(sender());
  if (action)
  {
    InteractionScheme interactionScheme = static_cast<InteractionScheme>(action->data().toInt());

    if (m_InteractionEventHandler)
    {
      m_InteractionSchemeSwitcher->SetInteractionScheme(m_InteractionEventHandler, interactionScheme);
    }
  }
}
