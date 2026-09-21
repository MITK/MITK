/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkInteractionSchemeToolBar.h>

#include <QmitkIconTheme.h>

#include <QAction>
#include <QActionGroup>

QmitkInteractionSchemeToolBar::QmitkInteractionSchemeToolBar(QWidget* parent/* = nullptr*/)
  : QToolBar(parent)
  , m_ActionGroup(new QActionGroup(this))
{
  QToolBar::setOrientation(Qt::Vertical);
  m_ActionGroup->setExclusive(false); // allow having no action selected

  AddButton(InteractionScheme::PACSStandard, tr("Pointer"), QStringLiteral(":/Qmitk/mm_pointer.svg"));
  AddButton(InteractionScheme::PACSLevelWindow, tr("Level/Window"), QStringLiteral(":/Qmitk/mm_contrast.svg"));
  AddButton(InteractionScheme::PACSPan, tr("Pan"), QStringLiteral(":/Qmitk/mm_pan.svg"));
  AddButton(InteractionScheme::PACSScroll, tr("Scroll"), QStringLiteral(":/Qmitk/mm_scroll.svg"));
  AddButton(InteractionScheme::PACSZoom, tr("Zoom"), QStringLiteral(":/Qmitk/mm_zoom.svg"));
}

QmitkInteractionSchemeToolBar::~QmitkInteractionSchemeToolBar()
{
  // nothing here
}

void QmitkInteractionSchemeToolBar::AddButton(InteractionScheme interactionScheme, const QString& toolName, const QString& iconResource)
{
  auto* action = new QAction(QmitkIconTheme::GetIcon(iconResource), toolName, this);
  action->setCheckable(true);
  action->setActionGroup(m_ActionGroup);
  action->setData(interactionScheme);
  connect(action, &QAction::triggered, this, &QmitkInteractionSchemeToolBar::OnActionTriggered);
  QToolBar::addAction(action);
}

void QmitkInteractionSchemeToolBar::OnActionTriggered()
{
  auto* action = qobject_cast<QAction*>(sender());
  if (nullptr == action)
  {
    return;
  }

  for (auto* otherAction : m_ActionGroup->actions())
  {
    if (otherAction != action)
    {
      otherAction->setChecked(false);
    }
  }

  // Unchecking the active tool falls back to the base scheme, where the left
  // mouse button has no effect at all.
  const auto interactionScheme = action->isChecked()
    ? static_cast<InteractionScheme>(action->data().toInt())
    : InteractionScheme::PACSBase;

  emit InteractionSchemeChanged(interactionScheme);
}

void QmitkInteractionSchemeToolBar::SetInteractionScheme(mitk::InteractionSchemeSwitcher::InteractionScheme interactionScheme)
{
  for (auto* action : m_ActionGroup->actions())
  {
    action->setChecked(static_cast<InteractionScheme>(action->data().toInt()) == interactionScheme);
  }
}
