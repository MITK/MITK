/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkSegWithPreviewToolGUIBase.h"

#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qboxlayout.h>
#include <qlabel.h>
#include <QApplication>

bool DefaultEnableConfirmSegBtnFunction(bool enabled)
{
  return enabled;
}

QmitkSegWithPreviewToolGUIBase::QmitkSegWithPreviewToolGUIBase(bool mode2D) : QmitkToolGUI(), m_EnableConfirmSegBtnFnc(DefaultEnableConfirmSegBtnFunction), m_Mode2D(mode2D)
{
  connect(this, SIGNAL(NewToolAssociated(mitk::Tool *)), this, SLOT(OnNewToolAssociated(mitk::Tool *)));
}

QmitkSegWithPreviewToolGUIBase::~QmitkSegWithPreviewToolGUIBase()
{
  if (m_Tool.IsNotNull())
  {
    m_Tool->CurrentlyBusy -= mitk::MessageDelegate1<QmitkSegWithPreviewToolGUIBase, bool>(this, &QmitkSegWithPreviewToolGUIBase::BusyStateChanged);
  }
}

void QmitkSegWithPreviewToolGUIBase::OnNewToolAssociated(mitk::Tool *tool)
{
  if (m_Tool.IsNotNull())
  {
    this->DisconnectOldTool(m_Tool);
  }

  m_Tool = dynamic_cast<mitk::SegWithPreviewTool*>(tool);

  if (nullptr == m_MainLayout)
  {
    // create the visible widgets
    m_MainLayout = new QVBoxLayout(this);
    m_ConfirmSegBtn = new QPushButton("Confirm Segmentation", this);
    connect(m_ConfirmSegBtn, SIGNAL(clicked()), this, SLOT(OnAcceptPreview()));

    m_CheckIgnoreLocks = new QCheckBox("Ignore label locks", this);
    m_CheckIgnoreLocks->setChecked(m_Tool->GetOverwriteStyle() == mitk::MultiLabelSegmentation::OverwriteStyle::IgnoreLocks);
    m_CheckIgnoreLocks->setToolTip("If checked, the lock state of labels will be ignored when the preview segmentation is confermed. Thus also locked label pixels can be changed by the operation.");

    m_CheckMerge = new QCheckBox("Merge with existing content", this);
    m_CheckMerge->setChecked(m_Tool->GetMergeStyle()==mitk::MultiLabelSegmentation::MergeStyle::Merge);
    m_CheckMerge->setToolTip("If checked, the preview segmantation will be merged with the existing segmantation into a union. If unchecked, the preview content will replace the old segmantation");

    m_CheckProcessAll = new QCheckBox("Process all time steps", this);
    m_CheckProcessAll->setChecked(false);
    m_CheckProcessAll->setToolTip("Process all time steps of the dynamic segmentation and not just the currently visible time step.");
    m_CheckProcessAll->setVisible(!m_Mode2D);
    //remark: keept m_CheckProcessAll deactivated in 2D because in this refactoring
    //it should be kept to the status quo and it was not clear how interpolation
    //would behave. As soon as it is sorted out we can remove that "feature switch"
    //or the comment.

    this->InitializeUI(m_MainLayout);

    m_MainLayout->addWidget(m_ConfirmSegBtn);
    m_MainLayout->addWidget(m_CheckIgnoreLocks);
    m_MainLayout->addWidget(m_CheckMerge);
    m_MainLayout->addWidget(m_CheckProcessAll);
  }

  if (m_Tool.IsNotNull())
  {
    this->ConnectNewTool(m_Tool);
  }
}

void QmitkSegWithPreviewToolGUIBase::OnAcceptPreview()
{
  if (m_Tool.IsNotNull())
  {
    if (m_CheckIgnoreLocks->isChecked())
    {
      m_Tool->SetOverwriteStyle(mitk::MultiLabelSegmentation::OverwriteStyle::IgnoreLocks);
    }
    else
    {
      m_Tool->SetOverwriteStyle(mitk::MultiLabelSegmentation::OverwriteStyle::RegardLocks);
    }

    if (m_CheckMerge->isChecked())
    {
      m_Tool->SetMergeStyle(mitk::MultiLabelSegmentation::MergeStyle::Merge);
    }
    else
    {
      m_Tool->SetMergeStyle(mitk::MultiLabelSegmentation::MergeStyle::Replace);
    }

    m_Tool->SetCreateAllTimeSteps(m_CheckProcessAll->isChecked());

    m_ConfirmSegBtn->setEnabled(false);
    m_Tool->ConfirmSegmentation();
  }
}

void QmitkSegWithPreviewToolGUIBase::DisconnectOldTool(mitk::SegWithPreviewTool* oldTool)
{
  oldTool->CurrentlyBusy -= mitk::MessageDelegate1<QmitkSegWithPreviewToolGUIBase, bool>(this, &QmitkSegWithPreviewToolGUIBase::BusyStateChanged);
}

void QmitkSegWithPreviewToolGUIBase::ConnectNewTool(mitk::SegWithPreviewTool* newTool)
{
  newTool->CurrentlyBusy +=
    mitk::MessageDelegate1<QmitkSegWithPreviewToolGUIBase, bool>(this, &QmitkSegWithPreviewToolGUIBase::BusyStateChanged);

  m_CheckProcessAll->setVisible(newTool->GetTargetSegmentationNode()->GetData()->GetTimeSteps() > 1);

  this->EnableWidgets(true);
}

void QmitkSegWithPreviewToolGUIBase::InitializeUI(QBoxLayout* /*mainLayout*/)
{
  //default implementation does nothing
}

void QmitkSegWithPreviewToolGUIBase::BusyStateChanged(bool isBusy)
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

void QmitkSegWithPreviewToolGUIBase::EnableWidgets(bool enabled)
{
  if (nullptr != m_MainLayout)
  {
    if (nullptr != m_ConfirmSegBtn)
    {
      m_ConfirmSegBtn->setEnabled(m_EnableConfirmSegBtnFnc(enabled));
    }
    if (nullptr != m_CheckIgnoreLocks)
    {
      m_CheckIgnoreLocks->setEnabled(enabled);
    }
    if (nullptr != m_CheckMerge)
    {
      m_CheckMerge->setEnabled(enabled);
    }
    if (nullptr != m_CheckProcessAll)
    {
      m_CheckProcessAll->setEnabled(enabled);
    }
  }
}

void QmitkSegWithPreviewToolGUIBase::SetMergeStyle(mitk::MultiLabelSegmentation::MergeStyle mergeStyle)
{
  if (nullptr != m_CheckMerge)
  {
    m_CheckMerge->setChecked(mergeStyle == mitk::MultiLabelSegmentation::MergeStyle::Merge);
  }
};

void QmitkSegWithPreviewToolGUIBase::SetOverwriteStyle(mitk::MultiLabelSegmentation::OverwriteStyle overwriteStyle)
{
  if (nullptr != m_CheckIgnoreLocks)
  {
    m_CheckIgnoreLocks->setChecked(overwriteStyle == mitk::MultiLabelSegmentation::OverwriteStyle::IgnoreLocks);
  }
};

