/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkAutoSegmentationToolGUIBase.h"

#include <qlabel.h>
#include <qpushbutton.h>
#include <qslider.h>
#include <QApplication>

QmitkAutoSegmentationToolGUIBase::QmitkAutoSegmentationToolGUIBase() : QmitkToolGUI()
{
  connect(this, SIGNAL(NewToolAssociated(mitk::Tool *)), this, SLOT(OnNewToolAssociated(mitk::Tool *)));
}

QmitkAutoSegmentationToolGUIBase::~QmitkAutoSegmentationToolGUIBase()
{
  if (m_Tool.IsNotNull())
  {
    m_Tool->CurrentlyBusy -= mitk::MessageDelegate1<QmitkAutoSegmentationToolGUIBase, bool>(this, &QmitkAutoSegmentationToolGUIBase::BusyStateChanged);
  }
}

void QmitkAutoSegmentationToolGUIBase::OnNewToolAssociated(mitk::Tool *tool)
{
  if (m_Tool.IsNotNull())
  {
    this->DisconnectOldTool(m_Tool);
  }

  m_Tool = dynamic_cast<mitk::AutoSegmentationWithPreviewTool*>(tool);

  if (nullptr == m_MainLayout)
  {
    // create the visible widgets
    m_MainLayout = new QVBoxLayout(this);

    this->InitializeUI(m_MainLayout);
  }

  if (m_Tool.IsNotNull())
  {
    this->ConnectNewTool(m_Tool);
  }
}

void QmitkAutoSegmentationToolGUIBase::OnAcceptPreview()
{
  if (m_Tool.IsNotNull())
  {
    if (m_CheckCreateNew->isChecked())
    {
      m_Tool->SetOverwriteExistingSegmentation(false);
    }
    else
    {
      m_Tool->SetOverwriteExistingSegmentation(true);
    }

    m_Tool->SetCreateAllTimeSteps(m_CheckProcessAll->isChecked());

    m_Tool->ConfirmSegmentation();
  }
}

void QmitkAutoSegmentationToolGUIBase::DisconnectOldTool(mitk::AutoSegmentationWithPreviewTool* oldTool)
{
  oldTool->CurrentlyBusy -= mitk::MessageDelegate1<QmitkAutoSegmentationToolGUIBase, bool>(this, &QmitkAutoSegmentationToolGUIBase::BusyStateChanged);
}

void QmitkAutoSegmentationToolGUIBase::ConnectNewTool(mitk::AutoSegmentationWithPreviewTool* newTool)
{
  newTool->CurrentlyBusy +=
    mitk::MessageDelegate1<QmitkAutoSegmentationToolGUIBase, bool>(this, &QmitkAutoSegmentationToolGUIBase::BusyStateChanged);

  newTool->SetOverwriteExistingSegmentation(true);
  m_CheckProcessAll->setVisible(newTool->GetTargetSegmentationNode()->GetData()->GetTimeSteps() > 1);
}

void QmitkAutoSegmentationToolGUIBase::InitializeUI(QBoxLayout* mainLayout)
{
  m_ConfirmSegBtn = new QPushButton("Confirm Segmentation", this);
  connect(m_ConfirmSegBtn, SIGNAL(clicked()), this, SLOT(OnAcceptPreview()));

  mainLayout->addWidget(m_ConfirmSegBtn);

  m_CheckProcessAll = new QCheckBox("Process all time steps", this);
  m_CheckProcessAll->setChecked(false);
  m_CheckProcessAll->setToolTip("Process/overwrite all time steps of the dynamic segmentation and not just the currently visible time step.");
  mainLayout->addWidget(m_CheckProcessAll);

  m_CheckCreateNew = new QCheckBox("Create as new segmentation", this);
  m_CheckCreateNew->setChecked(false);
  m_CheckCreateNew->setToolTip("Add the confirmed segmentation as a new segmentation instead of overwriting the currently selected.");
  mainLayout->addWidget(m_CheckCreateNew);
}

void QmitkAutoSegmentationToolGUIBase::BusyStateChanged(bool isBusy)
{
  if (isBusy)
  {
    QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));
  }
  else
  {
    QApplication::restoreOverrideCursor();
  }

  if (nullptr != m_MainLayout)
  {
    m_ConfirmSegBtn->setEnabled(!isBusy);
    m_CheckProcessAll->setEnabled(!isBusy);
    m_CheckCreateNew->setEnabled(!isBusy);
  }
}
