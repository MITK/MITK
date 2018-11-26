/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include <QMenu>

#include "QmitkSimpleBarrierManagerWidget.h"
#include "QmitkSimpleBarrierModel.h"
#include "QmitkSimpleBarrierParametersDelegate.h"
#include "QmitkSimpleBarrierTypeDelegate.h"

QmitkSimpleBarrierManagerWidget::QmitkSimpleBarrierManagerWidget(QWidget*): m_InternalUpdate(false)
{
  this->m_Controls.setupUi(this);

  m_InternalModel = new QmitkSimpleBarrierModel(this);
  m_TypeDelegate = new QmitkSimpleBarrierTypeDelegate(this);
  m_ParametersDelegate = new QmitkSimpleBarrierParametersDelegate(this);

  this->m_Controls.constraintsView->setModel(m_InternalModel);
  this->m_Controls.constraintsView->setItemDelegateForColumn(0, m_ParametersDelegate);
  this->m_Controls.constraintsView->setItemDelegateForColumn(1, m_TypeDelegate);
  this->m_Controls.constraintsView->setContextMenuPolicy(Qt::CustomContextMenu);

  connect(this->m_Controls.btnAdd, SIGNAL(clicked(bool)), this, SLOT(OnAddConstraint(bool)));
  connect(this->m_Controls.btnDel, SIGNAL(clicked(bool)), this, SLOT(OnDelConstraint(bool)));
  connect(m_Controls.constraintsView, SIGNAL(customContextMenuRequested(const QPoint&)), this,
          SLOT(OnShowContextMenuIsoSet(const QPoint&)));

  this->update();
}

QmitkSimpleBarrierManagerWidget::~QmitkSimpleBarrierManagerWidget()
{
  delete m_InternalModel;
  delete m_TypeDelegate;
  delete m_ParametersDelegate;
}

void QmitkSimpleBarrierManagerWidget::setChecker(mitk::SimpleBarrierConstraintChecker* pChecker,
    const mitk::ModelTraitsInterface::ParameterNamesType& names)
{
  this->m_Checker = pChecker;
  this->m_ParameterNames = names;
  this->m_InternalModel->setChecker(m_Checker, names);
  update();
}

void QmitkSimpleBarrierManagerWidget::OnShowContextMenuIsoSet(const QPoint& pos)
{
  QPoint globalPos = m_Controls.constraintsView->viewport()->mapToGlobal(pos);

  QModelIndex selectedIndex = m_Controls.constraintsView->currentIndex();

  QMenu viewMenu;
  QAction* addLevelAct = viewMenu.addAction("Add new constraint");
  QAction* delLevelAct = viewMenu.addAction("Delete selected constraint");
  delLevelAct->setEnabled(selectedIndex.isValid());

  QAction* selectedItem = viewMenu.exec(globalPos);

  if (selectedItem == addLevelAct)
  {
    this->m_InternalModel->addConstraint();
  }
  else if (selectedItem == delLevelAct)
  {
    this->m_InternalModel->deleteConstraint(selectedIndex);
  }
}

void QmitkSimpleBarrierManagerWidget::OnAddConstraint(bool)
{
  this->m_InternalModel->addConstraint();
}

void QmitkSimpleBarrierManagerWidget::OnDelConstraint(bool)
{
  QModelIndex selectedIndex = m_Controls.constraintsView->currentIndex();

  if (!selectedIndex.isValid())
  {
    selectedIndex = m_Controls.constraintsView->indexAt(QPoint(1, 1));
  }

  this->m_InternalModel->deleteConstraint(selectedIndex);
}

void QmitkSimpleBarrierManagerWidget::
update()
{

};
