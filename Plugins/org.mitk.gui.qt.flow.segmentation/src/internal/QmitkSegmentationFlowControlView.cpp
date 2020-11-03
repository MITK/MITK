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
#include "mitkLabelSetImage.h"
#include "mitkNodePredicateAnd.h"
#include "mitkNodePredicateNot.h"
#include "mitkNodePredicateProperty.h"
#include "mitkNodePredicateDataType.h"
#include "mitkIOUtil.h"

// Qmitk
#include "QmitkSegmentationFlowControlView.h"

// Qt
#include <QMessageBox>
#include <QDir>

const std::string QmitkSegmentationFlowControlView::VIEW_ID = "org.mitk.views.flow.control";

QmitkSegmentationFlowControlView::QmitkSegmentationFlowControlView()
    : m_Parent(nullptr)
{
  auto nodePredicate = mitk::NodePredicateAnd::New();
  nodePredicate->AddPredicate(mitk::TNodePredicateDataType<mitk::LabelSetImage>::New());
  nodePredicate->AddPredicate(mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("helper object")));
  m_SegmentationPredicate = nodePredicate;

  m_OutputDir = QString::fromStdString(itksys::SystemTools::GetCurrentWorkingDirectory());
  m_FileExtension = "nrrd";
}

void QmitkSegmentationFlowControlView::SetFocus()
{
    m_Controls.btnStoreAndAccept->setFocus();
}

void QmitkSegmentationFlowControlView::CreateQtPartControl(QWidget* parent)
{
    // create GUI widgets from the Qt Designer's .ui file
    m_Controls.setupUi(parent);

    m_Parent = parent;

    connect(m_Controls.btnStoreAndAccept, SIGNAL(clicked()), this, SLOT(OnAcceptButtonPushed()));

    m_Controls.labelStored->setVisible(false);
    UpdateControls();

    auto arguments = QCoreApplication::arguments();

    bool isFlagFound = false;
    for (auto arg : arguments)
    {
      if (isFlagFound)
      {
        m_OutputDir = arg;
        break;
      }
      isFlagFound = arg.startsWith("--flow.outputdir");
    }
    isFlagFound = false;
    for (auto arg : arguments)
    {
      if (isFlagFound)
      {
        m_FileExtension = arg;
        break;
      }
      isFlagFound = arg.startsWith("--flow.outputextension");
    }

    m_OutputDir = QDir::fromNativeSeparators(m_OutputDir);
}

void QmitkSegmentationFlowControlView::OnAcceptButtonPushed()
{
  auto nodes = this->GetDataStorage()->GetSubset(m_SegmentationPredicate);

  for (auto node : *nodes)
  {
    QString outputpath = m_OutputDir + "/" + QString::fromStdString(node->GetName()) + "." + m_FileExtension;
    outputpath = QDir::toNativeSeparators(QDir::cleanPath(outputpath));
    mitk::IOUtil::Save(node->GetData(), outputpath.toStdString());
  }

  m_Controls.labelStored->setVisible(true);
}

void QmitkSegmentationFlowControlView::UpdateControls()
{
  auto nodes = this->GetDataStorage()->GetSubset(m_SegmentationPredicate);
  m_Controls.btnStoreAndAccept->setEnabled(!nodes->empty());
};

void QmitkSegmentationFlowControlView::NodeAdded(const mitk::DataNode* /*node*/)
{
  UpdateControls();
};

void QmitkSegmentationFlowControlView::NodeChanged(const mitk::DataNode* /*node*/)
{
  UpdateControls();
};

void QmitkSegmentationFlowControlView::NodeRemoved(const mitk::DataNode* /*node*/)
{
  UpdateControls();
};
