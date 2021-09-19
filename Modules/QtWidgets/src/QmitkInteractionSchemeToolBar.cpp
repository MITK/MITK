/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkInteractionSchemeToolBar.h"

#include <QActionGroup>

QmitkInteractionSchemeToolBar::QmitkInteractionSchemeToolBar(QWidget* parent/* = nullptr*/)
  : QToolBar(parent)
  , m_ActionGroup(new QActionGroup(this))
  , m_InteractionEventHandler(nullptr)
{
  QToolBar::setOrientation(Qt::Vertical);
  QToolBar::setIconSize(QSize(17, 17));
  QToolBar::setFixedWidth(33);
  m_ActionGroup->setExclusive(false); // allow having no action selected

  AddButton(InteractionScheme::PACSStandard, tr("Pointer"), QIcon(":/Qmitk/mm_pointer.png"), true);
  AddButton(InteractionScheme::PACSLevelWindow, tr("Level/Window"), QIcon(":/Qmitk/mm_contrast.png"));
  AddButton(InteractionScheme::PACSPan, tr("Pan"), QIcon(":/Qmitk/mm_pan.png"));
  AddButton(InteractionScheme::PACSScroll, tr("Scroll"), QIcon(":/Qmitk/mm_scroll.png"));
  AddButton(InteractionScheme::PACSZoom, tr("Zoom"), QIcon(":/Qmitk/mm_zoom.png"));
}

QmitkInteractionSchemeToolBar::~QmitkInteractionSchemeToolBar()
{
  // nothing here
}

void QmitkInteractionSchemeToolBar::SetInteractionEventHandler(mitk::InteractionEventHandler::Pointer interactionEventHandler)
{
  if (interactionEventHandler == m_InteractionEventHandler)
  {
    return;
  }

  m_InteractionEventHandler = interactionEventHandler;
}

void QmitkInteractionSchemeToolBar::AddButton(InteractionScheme interactionScheme, const QString& toolName, const QIcon& icon, bool on)
{
  QAction* action = new QAction(icon, toolName, this);
  action->setCheckable(true);
  action->setActionGroup(m_ActionGroup);
  action->setChecked(on);
  action->setData(interactionScheme);
  connect(action, &QAction::triggered, this, &QmitkInteractionSchemeToolBar::OnInteractionSchemeChanged);
  QToolBar::addAction(action);
}

void QmitkInteractionSchemeToolBar::OnInteractionSchemeChanged()
{
  QAction* action = dynamic_cast<QAction*>(sender());
  if (nullptr != action)
  {
    for (auto actionIter : m_ActionGroup->actions())
    {
      if (actionIter != action)
      {
        actionIter->setChecked(false);
      }
    }

    InteractionScheme interactionScheme = static_cast<InteractionScheme>(action->data().toInt());
    // If the selected option is unchecked, use the base interaction with no primary tool
    if (!action->isChecked())
    {
      interactionScheme = InteractionScheme::PACSBase;
    }

    auto interactionSchemeSwitcher = mitk::InteractionSchemeSwitcher::New();
    try
    {
      interactionSchemeSwitcher->SetInteractionScheme(m_InteractionEventHandler, interactionScheme);
    }
    catch (const mitk::Exception &)
    {
      return;
    }
  }
}
