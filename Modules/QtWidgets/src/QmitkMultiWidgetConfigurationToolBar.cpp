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


// mitk qt widgets module
#include "QmitkAbstractMultiWidget.h"
#include "QmitkMultiWidgetLayoutSelectionWidget.h"

// qt
#include <QMenu>
#include <QToolButton>

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
}

void QmitkMultiWidgetConfigurationToolBar::AddButtons()
{
  QAction* setLayoutAction = new QAction(QIcon(":/Qmitk/cmwLayout.png"), tr("Set multi widget layout"), this);
  connect(setLayoutAction, &QAction::triggered, this, &QmitkMultiWidgetConfigurationToolBar::OnSetLayout);

  QToolBar::addAction(setLayoutAction);

  m_SynchronizeAction = new QAction(QIcon(":/Qmitk/cmwSynchronized.png"), tr("Desynchronize render windows"), this);
  m_SynchronizeAction->setCheckable(true);
  m_SynchronizeAction->setChecked(true);
  connect(m_SynchronizeAction, &QAction::triggered, this, &QmitkMultiWidgetConfigurationToolBar::OnSynchronize);

  QToolBar::addAction(m_SynchronizeAction);

  QAction* setAxialViewDirectionAction = new QAction(tr("Set axial view direction"), this);
  setAxialViewDirectionAction->setData(ViewDirection::Axial);
  connect(setAxialViewDirectionAction, &QAction::triggered, this, &QmitkMultiWidgetConfigurationToolBar::OnViewDirectionChanged);

  QAction* setSagittalViewDirectionAction = new QAction(tr("Set sagittal view direction"), this);
  setSagittalViewDirectionAction->setData(ViewDirection::Sagittal);
  connect(setSagittalViewDirectionAction, &QAction::triggered, this, &QmitkMultiWidgetConfigurationToolBar::OnViewDirectionChanged);

  QAction* setCoronalViewDirectionAction = new QAction(tr("Set coronal view direction"), this);
  setCoronalViewDirectionAction->setData(ViewDirection::Frontal);
  connect(setCoronalViewDirectionAction, &QAction::triggered, this, &QmitkMultiWidgetConfigurationToolBar::OnViewDirectionChanged);

  QToolButton* setViewDirectionToolButton = new QToolButton(this);
  setViewDirectionToolButton->setIcon(QIcon(":/Qmitk/cmwViewDirection.png"));
  setViewDirectionToolButton->setText(tr("Set synchronized render window view direction"));

  QMenu* menu = new QMenu();
  menu->addAction(setAxialViewDirectionAction);
  menu->addAction(setSagittalViewDirectionAction);
  menu->addAction(setCoronalViewDirectionAction);
  setViewDirectionToolButton->setMenu(menu);
  setViewDirectionToolButton->setPopupMode(QToolButton::InstantPopup);

  this->addWidget(setViewDirectionToolButton);
}

void QmitkMultiWidgetConfigurationToolBar::OnSetLayout()
{
  if (nullptr != m_MultiWidget)
  {
    m_LayoutSelectionPopup->setWindowFlags(Qt::Popup);
    m_LayoutSelectionPopup->move(this->cursor().pos());
    m_LayoutSelectionPopup->show();
  }
}

void QmitkMultiWidgetConfigurationToolBar::OnSynchronize()
{
  bool synchronized = m_SynchronizeAction->isChecked();
  if (synchronized)
  {
    m_SynchronizeAction->setIcon(QIcon(":/Qmitk/cmwSynchronized.png"));
    m_SynchronizeAction->setText(tr("Desynchronize render windows"));

  }
  else
  {
    m_SynchronizeAction->setIcon(QIcon(":/Qmitk/cmwDesynchronized.png"));
    m_SynchronizeAction->setText(tr("Synchronize render windows"));
  }

  m_SynchronizeAction->setChecked(synchronized);
  emit Synchronized(synchronized);
}

void QmitkMultiWidgetConfigurationToolBar::OnViewDirectionChanged()
{
  QAction* action = dynamic_cast<QAction*>(sender());
  if (action)
  {
    ViewDirection viewDirection = static_cast<ViewDirection>(action->data().toInt());
    emit ViewDirectionChanged(viewDirection);
  }
}
