/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkMultiWidgetConfigurationToolBar.h>

// mitk qt widgets module
#include <QmitkAbstractMultiWidget.h>
#include <QmitkIconTheme.h>

QmitkMultiWidgetConfigurationToolBar::QmitkMultiWidgetConfigurationToolBar(QmitkAbstractMultiWidget* multiWidget)
  : QToolBar(multiWidget)
  , m_MultiWidget(multiWidget)
{
  QToolBar::setOrientation(Qt::Vertical);

  AddButtons();
}

QmitkMultiWidgetConfigurationToolBar::~QmitkMultiWidgetConfigurationToolBar()
{
  // nothing here
}

void QmitkMultiWidgetConfigurationToolBar::AddButtons()
{
  // Opening the layout editor and managing synchronization moved to the per-cell
  // sync barcode and the layout editor view, so those buttons are gone; only the
  // interaction-scheme switch remains here.
  m_InteractionSchemeChangeAction = new QAction(this);
  m_InteractionSchemeChangeAction->setCheckable(true);
  this->UpdateInteractionSchemeAction(false);
  connect(m_InteractionSchemeChangeAction, &QAction::triggered, this, &QmitkMultiWidgetConfigurationToolBar::OnInteractionSchemeChanged);
  QToolBar::addAction(m_InteractionSchemeChangeAction);
}

void QmitkMultiWidgetConfigurationToolBar::UpdateInteractionSchemeAction(bool pacs)
{
  m_InteractionSchemeChangeAction->setChecked(pacs);

  if (pacs)
  {
    m_InteractionSchemeChangeAction->setIcon(QmitkIconTheme::GetIcon(QStringLiteral(":/Qmitk/mwPACS.svg")));
    m_InteractionSchemeChangeAction->setText(tr("Change to MITK interaction"));
  }
  else
  {
    m_InteractionSchemeChangeAction->setIcon(QmitkIconTheme::GetIcon(QStringLiteral(":/Qmitk/mwMITK.svg")));
    m_InteractionSchemeChangeAction->setText(tr("Change to PACS interaction"));
  }
}

void QmitkMultiWidgetConfigurationToolBar::OnInteractionSchemeChanged()
{
  const bool pacs = m_InteractionSchemeChangeAction->isChecked();

  this->UpdateInteractionSchemeAction(pacs);

  emit InteractionSchemeChanged(pacs
    ? mitk::InteractionSchemeSwitcher::PACSStandard
    : mitk::InteractionSchemeSwitcher::MITKStandard);
}

void QmitkMultiWidgetConfigurationToolBar::SetInteractionScheme(mitk::InteractionSchemeSwitcher::InteractionScheme scheme)
{
  this->UpdateInteractionSchemeAction(QmitkAbstractMultiWidget::IsPACSScheme(scheme));
}
