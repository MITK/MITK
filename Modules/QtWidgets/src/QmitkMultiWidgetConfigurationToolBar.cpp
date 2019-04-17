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

#include "QmitkMultiWidgetConfigurationToolBar.h"

QmitkMultiWidgetConfigurationToolBar::QmitkMultiWidgetConfigurationToolBar()
  : QToolBar()
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
}

void QmitkMultiWidgetConfigurationToolBar::AddButtons()
{
  QAction* setLayoutAction = new QAction(QIcon(":/Qmitk/mwLayout.png"), tr("Set multi widget layout"), this);
  connect(setLayoutAction, &QAction::triggered, this, &QmitkMultiWidgetConfigurationToolBar::OnSetLayout);

  QToolBar::addAction(setLayoutAction);

  m_SynchronizeAction = new QAction(QIcon(":/Qmitk/mwSynchronized.png"), tr("Desynchronize render windows"), this);
  m_SynchronizeAction->setCheckable(true);
  m_SynchronizeAction->setChecked(true);
  connect(m_SynchronizeAction, &QAction::triggered, this, &QmitkMultiWidgetConfigurationToolBar::OnSynchronize);

  QToolBar::addAction(m_SynchronizeAction);
}

void QmitkMultiWidgetConfigurationToolBar::OnSetLayout()
{
  m_LayoutSelectionPopup->setWindowFlags(Qt::Popup);
  m_LayoutSelectionPopup->move(this->cursor().pos());
  m_LayoutSelectionPopup->show();
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
