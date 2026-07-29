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

QmitkMultiWidgetConfigurationToolBar::QmitkMultiWidgetConfigurationToolBar(QmitkAbstractMultiWidget* multiWidget)
  : QToolBar(multiWidget)
  , m_MultiWidget(multiWidget)
{
  QToolBar::setOrientation(Qt::Vertical);
  QToolBar::setIconSize(QSize(17, 17));

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
  m_InteractionSchemeChangeAction = new QAction(QIcon(":/Qmitk/mwMITK.png"), tr("Change to PACS interaction"), this);
  m_InteractionSchemeChangeAction->setCheckable(true);
  m_InteractionSchemeChangeAction->setChecked(false);
  connect(m_InteractionSchemeChangeAction, &QAction::triggered, this, &QmitkMultiWidgetConfigurationToolBar::OnInteractionSchemeChanged);
  QToolBar::addAction(m_InteractionSchemeChangeAction);
}

void QmitkMultiWidgetConfigurationToolBar::OnInteractionSchemeChanged()
{
  bool PACSInteractionScheme = m_InteractionSchemeChangeAction->isChecked();
  if (PACSInteractionScheme)
  {
    m_InteractionSchemeChangeAction->setIcon(QIcon(":/Qmitk/mwPACS.png"));
    m_InteractionSchemeChangeAction->setText(tr("Change to MITK interaction"));
    emit InteractionSchemeChanged(mitk::InteractionSchemeSwitcher::PACSStandard);
  }
  else
  {
    m_InteractionSchemeChangeAction->setIcon(QIcon(":/Qmitk/mwMITK.png"));
    m_InteractionSchemeChangeAction->setText(tr("Change to PACS interaction"));
    emit InteractionSchemeChanged(mitk::InteractionSchemeSwitcher::MITKStandard);
  }

  m_InteractionSchemeChangeAction->setChecked(PACSInteractionScheme);
}
