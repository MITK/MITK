/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkMultiWidgetConfigurationToolBar.h"

// mitk qt widgets module
#include "QmitkAbstractMultiWidget.h"
#include "QmitkMultiWidgetLayoutSelectionWidget.h"

QmitkMultiWidgetConfigurationToolBar::QmitkMultiWidgetConfigurationToolBar(QmitkAbstractMultiWidget* multiWidget)
  : QToolBar(multiWidget)
  , m_MultiWidget(multiWidget)
{
  QToolBar::setOrientation(Qt::Vertical);
  QToolBar::setIconSize(QSize(17, 17));

  InitializeToolBar();
}

QmitkMultiWidgetConfigurationToolBar::~QmitkMultiWidgetConfigurationToolBar()
{
  // nothing here
}

void QmitkMultiWidgetConfigurationToolBar::InitializeToolBar()
{
  // create popup to show a widget to modify the multi widget layout
  m_LayoutSelectionPopup = new QmitkMultiWidgetLayoutSelectionWidget(this);
  m_LayoutSelectionPopup->hide();

  AddButtons();

  connect(m_LayoutSelectionPopup, &QmitkMultiWidgetLayoutSelectionWidget::LayoutSet, this, &QmitkMultiWidgetConfigurationToolBar::LayoutSet);
  connect(m_LayoutSelectionPopup, &QmitkMultiWidgetLayoutSelectionWidget::SaveLayout, this, &QmitkMultiWidgetConfigurationToolBar::SaveLayout);
  connect(m_LayoutSelectionPopup, &QmitkMultiWidgetLayoutSelectionWidget::LoadLayout, this, &QmitkMultiWidgetConfigurationToolBar::LoadLayout);
}

void QmitkMultiWidgetConfigurationToolBar::AddButtons()
{
  QAction* setLayoutAction = new QAction(QIcon(":/Qmitk/mwLayout.png"), tr("Set multi widget layout"), this);
  connect(setLayoutAction, &QAction::triggered, this, &QmitkMultiWidgetConfigurationToolBar::OnSetLayout);
  QToolBar::addAction(setLayoutAction);

  m_SynchronizeAction = new QAction(QIcon(":/Qmitk/mwDesynchronized.png"), tr("Synchronize render windows"), this);
  m_SynchronizeAction->setCheckable(true);
  m_SynchronizeAction->setChecked(false);
  connect(m_SynchronizeAction, &QAction::triggered, this, &QmitkMultiWidgetConfigurationToolBar::OnSynchronize);
  QToolBar::addAction(m_SynchronizeAction);

  m_InteractionSchemeChangeAction = new QAction(QIcon(":/Qmitk/mwMITK.png"), tr("Change to PACS interaction"), this);
  m_InteractionSchemeChangeAction->setCheckable(true);
  m_InteractionSchemeChangeAction->setChecked(false);
  connect(m_InteractionSchemeChangeAction, &QAction::triggered, this, &QmitkMultiWidgetConfigurationToolBar::OnInteractionSchemeChanged);
  QToolBar::addAction(m_InteractionSchemeChangeAction);
}

void QmitkMultiWidgetConfigurationToolBar::OnSetLayout()
{
  if (nullptr != m_MultiWidget)
  {
    m_LayoutSelectionPopup->setWindowFlags(Qt::Popup);
    m_LayoutSelectionPopup->move(this->cursor().pos().x() - m_LayoutSelectionPopup->width(), this->cursor().pos().y());
    m_LayoutSelectionPopup->show();
  }
}

void QmitkMultiWidgetConfigurationToolBar::OnSynchronize()
{
  bool synchronized = m_SynchronizeAction->isChecked();
  if (synchronized)
  {
    m_SynchronizeAction->setIcon(QIcon(":/Qmitk/mwSynchronized.png"));
    m_SynchronizeAction->setText(tr("Desynchronize render windows"));
  }
  else
  {
    m_SynchronizeAction->setIcon(QIcon(":/Qmitk/mwDesynchronized.png"));
    m_SynchronizeAction->setText(tr("Synchronize render windows"));
  }

  m_SynchronizeAction->setChecked(synchronized);
  emit Synchronized(synchronized);
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
