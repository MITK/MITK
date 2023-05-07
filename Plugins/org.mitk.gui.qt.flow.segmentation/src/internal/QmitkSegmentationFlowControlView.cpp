/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "org_mitk_gui_qt_flow_segmentation_Activator.h"

#include <itksys/SystemTools.hxx>

#include <berryPlatformUI.h>

//MITK
#include <mitkBaseApplication.h>
#include <mitkLabelSetImage.h>
#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateProperty.h>
#include <mitkNodePredicateDataType.h>
#include <mitkIOUtil.h>

// Qmitk
#include "QmitkSegmentationFlowControlView.h"

#include <ui_QmitkSegmentationFlowControlView.h>

// Qt
#include <QDir>

const std::string QmitkSegmentationFlowControlView::VIEW_ID = "org.mitk.views.flow.control";

QmitkSegmentationFlowControlView::QmitkSegmentationFlowControlView()
    : m_Controls(new Ui::SegmentationFlowControlView)
{
  berry::PlatformUI::GetWorkbench()->AddWorkbenchListener(this);

  auto notHelperObject = mitk::NodePredicateNot::New(
    mitk::NodePredicateProperty::New("helper object"));

  m_SegmentationPredicate = mitk::NodePredicateAnd::New(
    mitk::TNodePredicateDataType<mitk::LabelSetImage>::New(),
    notHelperObject);

  m_SegmentationTaskListPredicate = mitk::NodePredicateAnd::New(
    mitk::TNodePredicateDataType<mitk::SegmentationTaskList>::New(),
    notHelperObject);
}

QmitkSegmentationFlowControlView::~QmitkSegmentationFlowControlView()
{
  berry::PlatformUI::GetWorkbench()->RemoveWorkbenchListener(this);
}

void QmitkSegmentationFlowControlView::SetFocus()
{
  m_Controls->btnStoreAndAccept->setFocus();
}

void QmitkSegmentationFlowControlView::CreateQtPartControl(QWidget* parent)
{
  m_Controls->setupUi(parent);

  m_Controls->segmentationTaskListWidget->SetDataStorage(this->GetDataStorage());
  m_Controls->segmentationTaskListWidget->setVisible(false);

  m_Controls->labelStored->setVisible(false);

  using Self = QmitkSegmentationFlowControlView;

  connect(m_Controls->btnStoreAndAccept, &QPushButton::clicked, this, &Self::OnAcceptButtonClicked);

  this->UpdateControls();

  m_OutputDir = QString::fromStdString(mitk::BaseApplication::instance().config().getString("flow.outputdir", itksys::SystemTools::GetCurrentWorkingDirectory()));
  m_OutputDir = QDir::fromNativeSeparators(m_OutputDir);

  m_FileExtension = QString::fromStdString(mitk::BaseApplication::instance().config().getString("flow.outputextension", "nrrd"));
}

void QmitkSegmentationFlowControlView::OnAcceptButtonClicked()
{
  auto nodes = this->GetDataStorage()->GetSubset(m_SegmentationPredicate);

  for (auto node : *nodes)
  {
    QString outputpath = m_OutputDir + "/" + QString::fromStdString(node->GetName()) + "." + m_FileExtension;
    outputpath = QDir::toNativeSeparators(QDir::cleanPath(outputpath));
    mitk::IOUtil::Save(node->GetData(), outputpath.toStdString());
  }

  m_Controls->labelStored->setVisible(true);
}

void QmitkSegmentationFlowControlView::UpdateControls()
{
  auto dataStorage = this->GetDataStorage();
  auto hasSegmentationTaskList = !dataStorage->GetSubset(m_SegmentationTaskListPredicate)->empty();

  m_Controls->segmentationTaskListWidget->setVisible(hasSegmentationTaskList);

  if (hasSegmentationTaskList) // Give precedence to segmentation task list
  {
    m_Controls->btnStoreAndAccept->setVisible(false);
    m_Controls->labelStored->setVisible(false);
    return;
  }

  auto hasSegmentation = !dataStorage->GetSubset(m_SegmentationPredicate)->empty();

  m_Controls->btnStoreAndAccept->setEnabled(hasSegmentation);
  m_Controls->btnStoreAndAccept->setVisible(true);
}

void QmitkSegmentationFlowControlView::NodeAdded(const mitk::DataNode* node)
{
  if (dynamic_cast<const mitk::LabelSetImage*>(node->GetData()) != nullptr)
    this->UpdateControls();

  if (m_Controls->segmentationTaskListWidget->isVisible())
    m_Controls->segmentationTaskListWidget->CheckDataStorage();
}

void QmitkSegmentationFlowControlView::NodeChanged(const mitk::DataNode* node)
{
  if (dynamic_cast<const mitk::LabelSetImage*>(node->GetData()) != nullptr)
    this->UpdateControls();
}

void QmitkSegmentationFlowControlView::NodeRemoved(const mitk::DataNode* node)
{
  if (dynamic_cast<const mitk::LabelSetImage*>(node->GetData()) != nullptr)
    this->UpdateControls();

  if (m_Controls->segmentationTaskListWidget->isVisible())
    m_Controls->segmentationTaskListWidget->CheckDataStorage(node);
}

bool QmitkSegmentationFlowControlView::PreShutdown(berry::IWorkbench*, bool)
{
  if (m_Controls->segmentationTaskListWidget->isVisible())
    return m_Controls->segmentationTaskListWidget->OnPreShutdown();

  return true; // No veto against shutdown
}
