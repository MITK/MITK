/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "org_mitk_gui_qt_flow_segmentation_Activator.h"

// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>

#include <itksys/SystemTools.hxx>

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
#include <QMessageBox>
#include <QDir>

#include <filesystem>

const std::string QmitkSegmentationFlowControlView::VIEW_ID = "org.mitk.views.flow.control";

QmitkSegmentationFlowControlView::QmitkSegmentationFlowControlView()
    : m_Parent(nullptr),
      m_Controls(new Ui::SegmentationFlowControlView)
{
  auto notHelperObject = mitk::NodePredicateNot::New(
    mitk::NodePredicateProperty::New("helper object"));

  m_SegmentationPredicate = mitk::NodePredicateAnd::New(
    mitk::TNodePredicateDataType<mitk::LabelSetImage>::New(),
    notHelperObject);

  m_SegmentationTaskListPredicate = mitk::NodePredicateAnd::New(
    mitk::TNodePredicateDataType<mitk::SegmentationTaskList>::New(),
    notHelperObject);
}

void QmitkSegmentationFlowControlView::SetFocus()
{
    m_Controls->btnStoreAndAccept->setFocus();
}

void QmitkSegmentationFlowControlView::CreateQtPartControl(QWidget* parent)
{
  // create GUI widgets from the Qt Designer's .ui file
  m_Controls->setupUi(parent);

  m_Parent = parent;

  using Self = QmitkSegmentationFlowControlView;

  connect(m_Controls->btnStoreAndAccept, &QPushButton::clicked, this, &Self::OnAcceptButtonPushed);
  connect(m_Controls->segmentationTaskListWidget, &QmitkSegmentationTaskListWidget::ActiveTaskChanged, this, &Self::OnActiveTaskChanged);
  connect(m_Controls->segmentationTaskListWidget, &QmitkSegmentationTaskListWidget::CurrentTaskChanged, this, &Self::OnCurrentTaskChanged);

  m_Controls->segmentationTaskListWidget->setVisible(false);
  m_Controls->labelStored->setVisible(false);
  UpdateControls();

  m_OutputDir = QString::fromStdString(mitk::BaseApplication::instance().config().getString("flow.outputdir", itksys::SystemTools::GetCurrentWorkingDirectory()));
  m_OutputDir = QDir::fromNativeSeparators(m_OutputDir);

  m_FileExtension = QString::fromStdString(mitk::BaseApplication::instance().config().getString("flow.outputextension", "nrrd"));
}

void QmitkSegmentationFlowControlView::OnAcceptButtonPushed()
{
  if (m_Controls->segmentationTaskListWidget->isVisible())
  {
    auto* taskList = m_Controls->segmentationTaskListWidget->GetTaskList();

    if (taskList != nullptr)
    {
      auto activeTaskIndex = m_Controls->segmentationTaskListWidget->GetActiveTaskIndex();

      if (activeTaskIndex.has_value())
      {
        auto segmentationNode = m_Controls->segmentationTaskListWidget->GetSegmentationDataNode(activeTaskIndex.value());

        if (segmentationNode != nullptr)
        {
          QApplication::setOverrideCursor(Qt::BusyCursor);

          try
          {
            taskList->SaveTask(activeTaskIndex.value(), segmentationNode->GetData());
            m_Controls->segmentationTaskListWidget->OnUnsavedChangesSaved();
          }
          catch (const mitk::Exception& e)
          {
            MITK_ERROR << e;
          }

          QApplication::restoreOverrideCursor();
        }
      }
    }
  }
  else
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
}

void QmitkSegmentationFlowControlView::OnActiveTaskChanged(const std::optional<size_t>&)
{
  this->UpdateControls();
}

void QmitkSegmentationFlowControlView::OnCurrentTaskChanged(const std::optional<size_t>&)
{
  this->UpdateControls();
}

void QmitkSegmentationFlowControlView::UpdateControls()
{
  auto dataStorage = this->GetDataStorage();

  auto hasTaskList = !dataStorage->GetSubset(m_SegmentationTaskListPredicate)->empty();
  m_Controls->segmentationTaskListWidget->setVisible(hasTaskList);

  if (hasTaskList)
  {
    auto activeTaskIsShown = m_Controls->segmentationTaskListWidget->ActiveTaskIsShown();
    m_Controls->btnStoreAndAccept->setEnabled(activeTaskIsShown);
  }
  else
  {
    auto hasSegmentation = !dataStorage->GetSubset(m_SegmentationPredicate)->empty();
    m_Controls->btnStoreAndAccept->setEnabled(hasSegmentation);
  }
}

void QmitkSegmentationFlowControlView::NodeAdded(const mitk::DataNode* node)
{
  if (dynamic_cast<const mitk::LabelSetImage*>(node->GetData()) != nullptr)
    this->UpdateControls();
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
}
