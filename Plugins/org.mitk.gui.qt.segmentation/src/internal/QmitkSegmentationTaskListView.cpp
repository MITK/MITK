/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkSegmentationTaskListView.h"
#include <ui_QmitkSegmentationTaskListView.h>

#include <berryPlatformUI.h>

const std::string QmitkSegmentationTaskListView::VIEW_ID = "org.mitk.views.segmentationtasklist";

QmitkSegmentationTaskListView::QmitkSegmentationTaskListView()
  : m_Ui(new Ui::QmitkSegmentationTaskListView)
{
  berry::PlatformUI::GetWorkbench()->AddWorkbenchListener(this);
}

QmitkSegmentationTaskListView::~QmitkSegmentationTaskListView()
{
  berry::PlatformUI::GetWorkbench()->RemoveWorkbenchListener(this);
}

void QmitkSegmentationTaskListView::CreateQtPartControl(QWidget* parent)
{
  m_Ui->setupUi(parent);
  m_Ui->segmentationTaskListWidget->SetDataStorage(this->GetDataStorage());
}

void QmitkSegmentationTaskListView::SetFocus()
{
}

void QmitkSegmentationTaskListView::NodeAdded(const mitk::DataNode*)
{
  m_Ui->segmentationTaskListWidget->CheckDataStorage();
}

void QmitkSegmentationTaskListView::NodeRemoved(const mitk::DataNode* removedNode)
{
  m_Ui->segmentationTaskListWidget->CheckDataStorage(removedNode);
}

bool QmitkSegmentationTaskListView::PreShutdown(berry::IWorkbench*, bool)
{
  return m_Ui->segmentationTaskListWidget->OnPreShutdown();
}
