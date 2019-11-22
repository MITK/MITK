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

#include "org_mitk_gui_qt_flowbench_segmentation_Activator.h"

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

const std::string QmitkSegmentationFlowControlView::VIEW_ID = "org.mitk.views.flowbench.control";

QmitkSegmentationFlowControlView::QmitkSegmentationFlowControlView()
    : m_Parent(nullptr)
{
  auto nodePredicate = mitk::NodePredicateAnd::New();
  nodePredicate->AddPredicate(mitk::TNodePredicateDataType<mitk::LabelSetImage>::New());
  nodePredicate->AddPredicate(mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("helper object")));
  m_SegmentationPredicate = nodePredicate;

  m_OutputDir = itksys::SystemTools::GetCurrentWorkingDirectory();
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
        m_OutputDir = arg.toLatin1();
        break;
      }
      isFlagFound = arg.startsWith("--flow.outputdir");
    }
    isFlagFound = false;
    for (auto arg : arguments)
    {
      if (isFlagFound)
      {
        m_FileExtension = arg.toLatin1();
        break;
      }
      isFlagFound = arg.startsWith("--flow.outputformat");
    }
}

void QmitkSegmentationFlowControlView::OnAcceptButtonPushed()
{
  auto nodes = this->GetDataStorage()->GetSubset(m_SegmentationPredicate);

  for (auto node : *nodes)
  {
    std::string outputpath = m_OutputDir + "\\" + node->GetName() + "." + m_FileExtension;
    mitk::IOUtil::Save(node->GetData(), outputpath);
  }

  m_Controls.labelStored->setVisible(true);
}

void QmitkSegmentationFlowControlView::UpdateControls()
{
  auto nodes = this->GetDataStorage()->GetSubset(m_SegmentationPredicate);
  m_Controls.btnStoreAndAccept->setEnabled(!nodes->empty());
};

void QmitkSegmentationFlowControlView::NodeAdded(const mitk::DataNode* node)
{
  UpdateControls();
};

void QmitkSegmentationFlowControlView::NodeChanged(const mitk::DataNode* node)
{
  UpdateControls();
};

void QmitkSegmentationFlowControlView::NodeRemoved(const mitk::DataNode* node)
{
  UpdateControls();
};
