/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkAutoSegmentationToolGUIBase.h"

#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qboxlayout.h>
#include <qlabel.h>
#include <QApplication>

QmitkAutoSegmentationToolGUIBase::QmitkAutoSegmentationToolGUIBase(bool mode2D) : QmitkToolGUI(), m_Mode2D(mode2D)
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
    m_ConfirmSegBtn = new QPushButton("Confirm Segmentation", this);
    connect(m_ConfirmSegBtn, SIGNAL(clicked()), this, SLOT(OnAcceptPreview()));

    m_CheckProcessAll = new QCheckBox("Process all time steps", this);
    m_CheckProcessAll->setChecked(false);
    m_CheckProcessAll->setToolTip("Process/overwrite all time steps of the dynamic segmentation and not just the currently visible time step.");
    m_CheckProcessAll->setVisible(!m_Mode2D);

    m_CheckCreateNew = new QCheckBox("Create as new segmentation", this);
    m_CheckCreateNew->setChecked(false);
    m_CheckCreateNew->setToolTip("Add the confirmed segmentation as a new segmentation instead of overwriting the currently selected.");
    m_CheckCreateNew->setVisible(!m_Mode2D);

    this->InitializeUI(m_MainLayout);

    m_MainLayout->addWidget(m_ConfirmSegBtn);
    m_MainLayout->addWidget(m_CheckProcessAll);
    m_MainLayout->addWidget(m_CheckCreateNew);
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

    m_ConfirmSegBtn->setEnabled(false);
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
  this->EnableWidgets(!isBusy);
 }

void QmitkAutoSegmentationToolGUIBase::EnableWidgets(bool enabled)
{
  if (nullptr != m_MainLayout)
  {
    if (nullptr != m_ConfirmSegBtn)
    {
      m_ConfirmSegBtn->setEnabled(enabled);
    }
    if (nullptr != m_CheckProcessAll)
    {
      m_CheckProcessAll->setEnabled(enabled);
    }
    if (nullptr != m_CheckCreateNew)
    {
      m_CheckCreateNew->setEnabled(enabled);
    }
  }
}
