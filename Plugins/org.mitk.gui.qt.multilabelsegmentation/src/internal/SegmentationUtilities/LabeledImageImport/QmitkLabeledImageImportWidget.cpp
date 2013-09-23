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

#include "QmitkLabeledImageImportWidget.h"
#include "../../Common/QmitkDataSelectionWidget.h"

#include <mitkException.h>
#include <mitkExceptionMacro.h>
#include <mitkImageStatisticsHolder.h>
#include <mitkLabelSetImage.h>
#include <mitkProgressBar.h>
#include <mitkSliceNavigationController.h>

#include <qmessagebox.h>

static const char* const HelpText = "Select a labeled image and a segmentation";

QmitkLabeledImageImportWidget::QmitkLabeledImageImportWidget(mitk::SliceNavigationController* timeNavigationController, QWidget* parent)
  : QmitkSegmentationUtilityWidget(timeNavigationController, parent)
{
  m_Controls.setupUi(this);

  m_Controls.m_DataSelectionWidget->AddDataStorageComboBox(QmitkDataSelectionWidget::SegmentationPredicate);
  m_Controls.m_DataSelectionWidget->AddDataStorageComboBox(QmitkDataSelectionWidget::ImagePredicate);
  m_Controls.m_DataSelectionWidget->SetHelpText(HelpText);

  this->EnableButtons(false);

  connect (m_Controls.m_btImport, SIGNAL(pressed()), this, SLOT(OnImportPressed()));

  connect(m_Controls.m_DataSelectionWidget, SIGNAL(SelectionChanged(unsigned int, const mitk::DataNode*)),
    this, SLOT(OnSelectionChanged(unsigned int, const mitk::DataNode*)));

  if( m_Controls.m_DataSelectionWidget->GetSelection(0).IsNotNull() &&
    m_Controls.m_DataSelectionWidget->GetSelection(1).IsNotNull() )
  {
    this->OnSelectionChanged( 0, m_Controls.m_DataSelectionWidget->GetSelection(0));
  }
}

QmitkLabeledImageImportWidget::~QmitkLabeledImageImportWidget()
{
}

void QmitkLabeledImageImportWidget::OnSelectionChanged(unsigned int index, const mitk::DataNode* selection)
{
  //QmitkDataSelectionWidget* dataSelectionWidget = m_Controls.m_DataSelectionWidget;
  mitk::DataNode::Pointer node0 = m_Controls.m_DataSelectionWidget->GetSelection(0);
  mitk::DataNode::Pointer node1 = m_Controls.m_DataSelectionWidget->GetSelection(1);

  if (node0.IsNull() || node1.IsNull() )
  {
    m_Controls.m_DataSelectionWidget->SetHelpText(HelpText);
    this->EnableButtons(false);
  }
  else
  {
    this->SelectionControl(index, selection);
  }
}

void QmitkLabeledImageImportWidget::SelectionControl(unsigned int index, const mitk::DataNode* selection)
{
//  QmitkDataSelectionWidget* dataSelectionWidget = m_Controls.dataSelectionWidget;
  mitk::DataNode::Pointer node = m_Controls.m_DataSelectionWidget->GetSelection(index);

  if( m_Controls.m_DataSelectionWidget->GetSelection(0) == m_Controls.m_DataSelectionWidget->GetSelection(1) )
  {
    m_Controls.m_DataSelectionWidget->SetHelpText("Select two different images above");
    this->EnableButtons(false);
    return;
  }

  else if( node.IsNotNull() && selection )
  {
    mitk::LabelSetImage::Pointer segmentationImage = dynamic_cast<mitk::LabelSetImage*> ( m_Controls.m_DataSelectionWidget->GetSelection(0)->GetData() );
    mitk::Image::Pointer labeledImage = dynamic_cast<mitk::Image*> ( m_Controls.m_DataSelectionWidget->GetSelection(1)->GetData() );
  }

  else
  {
    m_Controls.m_DataSelectionWidget->SetHelpText(HelpText);
    return;
  }

  m_Controls.m_DataSelectionWidget->SetHelpText("");
  this->EnableButtons();
}

void QmitkLabeledImageImportWidget::EnableButtons(bool enable)
{
  m_Controls.m_btImport->setEnabled(enable);
}

void QmitkLabeledImageImportWidget::OnImportPressed()
{
  mitk::LabelSetImage::Pointer segmentationImage = static_cast<mitk::LabelSetImage*>(m_Controls.m_DataSelectionWidget->GetSelection(0)->GetData());
  mitk::Image::Pointer labeledImage = static_cast<mitk::Image*>(m_Controls.m_DataSelectionWidget->GetSelection(1)->GetData());

  if(segmentationImage.IsNull() || labeledImage.IsNull() )
  {
    MITK_ERROR << "Selection does not contain an image";
    QMessageBox::information( this, "Labeled Image Import", "Selection does not contain an image", QMessageBox::Ok );
    return;
  }

  try
  {
    segmentationImage->ImportLabeledImage(labeledImage);
  }
  catch ( mitk::Exception& except)
  {
    MITK_ERROR << "Exception caught: " << except.GetDescription();
    QMessageBox::information( this, "Labeled Image Import", "Could not import the selected labeled image.\n See error log for details.", QMessageBox::Ok );
    this->EnableButtons();
    return;
  }
}
