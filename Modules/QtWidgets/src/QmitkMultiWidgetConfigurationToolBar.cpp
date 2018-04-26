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

QmitkMultiWidgetConfigurationToolBar::QmitkMultiWidgetConfigurationToolBar(QmitkCustomMultiWidget* customMultiWidget)
  : QToolBar(customMultiWidget)
  , m_CustomMultiWidget(customMultiWidget)
  , m_ActionGroup(nullptr)
{
  QToolBar::setOrientation(Qt::Vertical);
  QToolBar::setIconSize(QSize(17, 17));

  InitActionGroup();
}

QmitkMultiWidgetConfigurationToolBar::~QmitkMultiWidgetConfigurationToolBar()
{
  // nothing here
}

void QmitkMultiWidgetConfigurationToolBar::InitActionGroup()
{
  m_ActionGroup = new QActionGroup(this);
  m_ActionGroup->setExclusive(true);

  // create popup to show a widget to modify the multi widget layout
  m_LayoutSelectionPopup = new QmitkMultiWidgetLayoutSelectionWidget(this);
  m_LayoutSelectionPopup->hide();

  AddButtons();

  connect(m_LayoutSelectionPopup, SIGNAL(LayoutSet(int, int)), SIGNAL(LayoutSet(int, int)));
}

void QmitkMultiWidgetConfigurationToolBar::AddButtons()
{
  QAction* setLayoutAction = new QAction(QIcon(":/Qmitk/cmwLayout.png"), tr("Set multi widget layout"), this);
  setLayoutAction->setActionGroup(m_ActionGroup);
  connect(setLayoutAction, SIGNAL(triggered()), SLOT(OnSetLayout()));

  QToolBar::addAction(setLayoutAction);


}

void QmitkMultiWidgetConfigurationToolBar::OnSetLayout()
{
  if (nullptr != m_CustomMultiWidget)
  {
    m_LayoutSelectionPopup->setWindowFlags(Qt::Popup);
    m_LayoutSelectionPopup->move(this->cursor().pos());
    m_LayoutSelectionPopup->show();
  }
}

{
  {
  }
}