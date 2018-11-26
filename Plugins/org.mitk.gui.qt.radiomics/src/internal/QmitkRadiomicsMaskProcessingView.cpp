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

#include "QmitkRadiomicsMaskProcessingView.h"

// QT includes (GUI)
#include <qlabel.h>
#include <qspinbox.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qradiobutton.h>
#include <qmessagebox.h>

// Berry includes (selection service)
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>

// MITK includes (GUI)
#include <QmitkDataStorageComboBox.h>
#include "QmitkDataNodeSelectionProvider.h"
#include "mitkDataNodeObject.h"

// MITK includes (general
#include <mitkNodePredicateDataType.h>
#include <mitkNodePredicateAnd.h>
#include <mitkProperties.h>
#include <mitkTransformationOperation.h>
#include <mitkLabelSetImage.h>

// Specific GUI Includes
#include <mitkMaskCleaningOperation.h>

QmitkRadiomicsMaskProcessing::QmitkRadiomicsMaskProcessing()
: QmitkAbstractView(),
  m_Controls(nullptr)
{
}

QmitkRadiomicsMaskProcessing::~QmitkRadiomicsMaskProcessing()
{
  //berry::ISelectionService* s = GetSite()->GetWorkbenchWindow()->GetSelectionService();
  //if(s)
  //  s->RemoveSelectionListener(m_SelectionListener);
}

void QmitkRadiomicsMaskProcessing::CreateQtPartControl(QWidget *parent)
{
  if (m_Controls == nullptr)
  {
    m_Controls = new Ui::QmitkRadiomicsMaskProcessingViewControls;
    m_Controls->setupUi(parent);

    QLabel * label1 = new QLabel("Image: ");
    QLabel * label2 = new QLabel("Mask: ");
    QmitkDataStorageComboBox * cb_inputimage = new QmitkDataStorageComboBox(this->GetDataStorage(), mitk::TNodePredicateDataType<mitk::Image>::New());
    QmitkDataStorageComboBox * cb_maskimage = new QmitkDataStorageComboBox(this->GetDataStorage(), mitk::TNodePredicateDataType<mitk::LabelSetImage>::New());
    m_Controls->m_InputImageGroup->layout()->addWidget(label1);
    m_Controls->m_InputImageGroup->layout()->addWidget(cb_inputimage);
    m_Controls->m_InputImageGroup->layout()->addWidget(label2);
    m_Controls->m_InputImageGroup->layout()->addWidget(cb_maskimage);

    this->CreateConnections();
  }
}

void QmitkRadiomicsMaskProcessing::CreateConnections()
{
  if ( m_Controls )
  {
    connect( (QObject*)(m_Controls->maskBasedExecutionButton), SIGNAL(clicked() ), this, SLOT(executeButtonIntervalBasedMaskClearning() ) );
    connect((QObject*)(m_Controls->outlierRemoveButton), SIGNAL(clicked()), this, SLOT(executeButtonMaskOutlierRemoval()));
  }
}

void QmitkRadiomicsMaskProcessing::executeButtonIntervalBasedMaskClearning()
{
  QmitkDataStorageComboBox * cb_image = dynamic_cast<QmitkDataStorageComboBox *>(m_Controls->m_InputImageGroup->layout()->itemAt(1)->widget());
  QmitkDataStorageComboBox * cb_maskimage = dynamic_cast<QmitkDataStorageComboBox *>(m_Controls->m_InputImageGroup->layout()->itemAt(3)->widget());
  mitk::BaseData* baseDataRawImage = NULL;
  mitk::BaseData* baseDataMaskImage = NULL;

  mitk::Image::Pointer raw_image;
  mitk::Image::Pointer mask_image;
  std::string nodeName;

  if ((cb_image->GetSelectedNode().IsNotNull()) && (cb_maskimage->GetSelectedNode().IsNotNull()))
  {
    baseDataRawImage = (cb_image->GetSelectedNode()->GetData());
    baseDataMaskImage = (cb_maskimage->GetSelectedNode()->GetData());
    nodeName = cb_maskimage->GetSelectedNode()->GetName();
  }
  if ((baseDataRawImage != NULL) && (baseDataMaskImage != NULL))
  {
    raw_image = dynamic_cast<mitk::Image *>(baseDataRawImage);
    mask_image = dynamic_cast<mitk::Image *>(baseDataMaskImage);
  }
  else {
    QMessageBox msgBox;
    msgBox.setText("Please specify the images that shlould be used.");
    msgBox.exec();
    return;
  }
  if (raw_image.IsNull() || mask_image.IsNull())
  {
    QMessageBox msgBox;
    msgBox.setText("Error during processing the specified images.");
    msgBox.exec();
    return;
  }

  bool lowerLimitOn = m_Controls->lowerOn->isChecked();
  bool upperLimitOn = m_Controls->upperOn->isChecked();
  double lowerLimit = m_Controls->lowerLimitSpinbox->value();
  double upperLimit = m_Controls->spinboxUpperValue->value();


  auto image = mitk::MaskCleaningOperation::RangeBasedMasking(raw_image, mask_image, lowerLimitOn, lowerLimit, upperLimitOn, upperLimit);

  mitk::LabelSetImage::Pointer labelResult = mitk::LabelSetImage::New();
  labelResult->InitializeByLabeledImage(image);
  mitk::LabelSetImage::Pointer oldLabelSet = dynamic_cast<mitk::LabelSetImage *>(mask_image.GetPointer());
  labelResult->AddLabelSetToLayer(labelResult->GetActiveLayer(), oldLabelSet->GetLabelSet());

  mitk::DataNode::Pointer result = mitk::DataNode::New();
  result->SetProperty("name", mitk::StringProperty::New(nodeName+"::MaskRange"));
  result->SetData(labelResult);
  GetDataStorage()->Add(result, cb_image->GetSelectedNode());
}

void QmitkRadiomicsMaskProcessing::executeButtonMaskOutlierRemoval()
{
  QmitkDataStorageComboBox * cb_image = dynamic_cast<QmitkDataStorageComboBox *>(m_Controls->m_InputImageGroup->layout()->itemAt(1)->widget());
  QmitkDataStorageComboBox * cb_maskimage = dynamic_cast<QmitkDataStorageComboBox *>(m_Controls->m_InputImageGroup->layout()->itemAt(3)->widget());
  mitk::BaseData* baseDataRawImage = NULL;
  mitk::BaseData* baseDataMaskImage = NULL;

  mitk::Image::Pointer raw_image;
  mitk::Image::Pointer mask_image;
  std::string nodeName;

  if ((cb_image->GetSelectedNode().IsNotNull()) && (cb_maskimage->GetSelectedNode().IsNotNull()))
  {
    baseDataRawImage = (cb_image->GetSelectedNode()->GetData());
    baseDataMaskImage = (cb_maskimage->GetSelectedNode()->GetData());
    nodeName = cb_maskimage->GetSelectedNode()->GetName();
  }
  if ((baseDataRawImage != NULL) && (baseDataMaskImage != NULL))
  {
    raw_image = dynamic_cast<mitk::Image *>(baseDataRawImage);
    mask_image = dynamic_cast<mitk::Image *>(baseDataMaskImage);
  }
  else {
    QMessageBox msgBox;
    msgBox.setText("Please specify the images that shlould be used.");
    msgBox.exec();
    return;
  }
  if (raw_image.IsNull() || mask_image.IsNull())
  {
    QMessageBox msgBox;
    msgBox.setText("Error during processing the specified images.");
    msgBox.exec();
    return;
  }

  auto image = mitk::MaskCleaningOperation::MaskOutlierFiltering(raw_image, mask_image);

  mitk::LabelSetImage::Pointer labelResult = mitk::LabelSetImage::New();
  labelResult->InitializeByLabeledImage(image);
  mitk::LabelSetImage::Pointer oldLabelSet = dynamic_cast<mitk::LabelSetImage *>(mask_image.GetPointer());
  labelResult->AddLabelSetToLayer(labelResult->GetActiveLayer(), oldLabelSet->GetLabelSet());

  mitk::DataNode::Pointer result = mitk::DataNode::New();
  result->SetProperty("name", mitk::StringProperty::New(nodeName + "::MaskOutlier"));
  result->SetData(labelResult);
  GetDataStorage()->Add(result, cb_image->GetSelectedNode());
}

void QmitkRadiomicsMaskProcessing::SetFocus()
{
}

//datamanager selection changed
void QmitkRadiomicsMaskProcessing::OnSelectionChanged(berry::IWorkbenchPart::Pointer, const QList<mitk::DataNode::Pointer>& nodes)
{
  //any nodes there?
  if (!nodes.empty())
  {

  }
}
