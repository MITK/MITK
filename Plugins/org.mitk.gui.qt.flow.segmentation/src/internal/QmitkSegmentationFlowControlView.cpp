/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "org_mitk_gui_qt_flow_segmentation_Activator.h"

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
#include <QDir>

const std::string QmitkSegmentationFlowControlView::VIEW_ID = "org.mitk.views.flow.control";

QmitkSegmentationFlowControlView::QmitkSegmentationFlowControlView()
    : m_Controls(new Ui::SegmentationFlowControlView),
      m_Parent(nullptr)
{
  auto notHelperObject = mitk::NodePredicateNot::New(
    mitk::NodePredicateProperty::New("helper object"));

  m_SegmentationPredicate = mitk::NodePredicateAnd::New(
    mitk::TNodePredicateDataType<mitk::LabelSetImage>::New(),
    notHelperObject);
}

QmitkSegmentationFlowControlView::~QmitkSegmentationFlowControlView()
{
}

void QmitkSegmentationFlowControlView::SetFocus()
{
  m_Controls->btnStoreAndAccept->setFocus();
}

void QmitkSegmentationFlowControlView::CreateQtPartControl(QWidget* parent)
{
  m_Controls->setupUi(parent);
  m_Parent = parent;

  using Self = QmitkSegmentationFlowControlView;

  connect(m_Controls->btnStoreAndAccept, &QPushButton::clicked, this, &Self::OnAcceptButtonClicked);

  m_Controls->labelStored->setVisible(false);

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
  auto hasSegmentation = !dataStorage->GetSubset(m_SegmentationPredicate)->empty();

  m_Controls->btnStoreAndAccept->setEnabled(hasSegmentation);
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
