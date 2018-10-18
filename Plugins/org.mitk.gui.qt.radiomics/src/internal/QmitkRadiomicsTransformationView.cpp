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

#include "QmitkRadiomicsTransformationView.h"

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
#include "QmitkGIFConfigurationPanel.h"

QmitkRadiomicsTransformation::QmitkRadiomicsTransformation()
: QmitkAbstractView(),
  m_Controls(nullptr)
{
}

QmitkRadiomicsTransformation::~QmitkRadiomicsTransformation()
{
  //berry::ISelectionService* s = GetSite()->GetWorkbenchWindow()->GetSelectionService();
  //if(s)
  //  s->RemoveSelectionListener(m_SelectionListener);
}

void QmitkRadiomicsTransformation::CreateQtPartControl(QWidget *parent)
{
  if (m_Controls == nullptr)
  {
    m_Controls = new Ui::QmitkRadiomicsTransformationViewControls;
    m_Controls->setupUi(parent);

    QLabel * label1 = new QLabel("Image: ");
    QmitkDataStorageComboBox * cb_inputimage = new QmitkDataStorageComboBox(this->GetDataStorage(), mitk::TNodePredicateDataType<mitk::Image>::New());
    m_Controls->m_InputImageGroup->layout()->addWidget(label1);
    m_Controls->m_InputImageGroup->layout()->addWidget(cb_inputimage);

    this->CreateConnections();
  }
}

void QmitkRadiomicsTransformation::CreateConnections()
{
  if ( m_Controls )
  {
    connect( (QObject*)(m_Controls->buttonExecuteMultiresolution), SIGNAL(clicked() ), this, SLOT(executeButtonMultiResolutionPressed() ) );
    connect((QObject*)(m_Controls->m_WaveletExecuteButton), SIGNAL(clicked()), this, SLOT(executeButtonWaveletPressed()));
    connect((QObject*)(m_Controls->m_ExecuteLOG), SIGNAL(clicked()), this, SLOT(executeButtonLoGPressed()));
    connect((QObject*)(m_Controls->buttonResampleImage), SIGNAL(clicked()), this, SLOT(executeButtonResamplingPressed()));

  }
}

void QmitkRadiomicsTransformation::executeButtonMultiResolutionPressed()
{
  QmitkDataStorageComboBox * cb_image = dynamic_cast<QmitkDataStorageComboBox *>(m_Controls->m_InputImageGroup->layout()->itemAt(1)->widget());
  mitk::BaseData* baseDataRawImage = NULL;

  mitk::Image::Pointer raw_image;
  std::string nodeName;
  if ((cb_image->GetSelectedNode().IsNotNull() ) )
  {
    baseDataRawImage = (cb_image->GetSelectedNode()->GetData());
    nodeName = cb_image->GetSelectedNode()->GetName();
  }
  if ((baseDataRawImage != NULL))
  {
    raw_image = dynamic_cast<mitk::Image *>(baseDataRawImage);
  }
  else {
    QMessageBox msgBox;
    msgBox.setText("Please specify the images that shlould be used.");
    msgBox.exec();
    return;
  }
  if (raw_image.IsNull())
  {
    QMessageBox msgBox;
    msgBox.setText("Error during processing the specified images.");
    msgBox.exec();
    return;
  }

  unsigned int numberOfLevels = m_Controls->m_NumberOfLevels->value();
  bool resultAsDouble = m_Controls->m_resultAsDouble->isChecked();

  auto results = mitk::TransformationOperation::MultiResolution(raw_image, numberOfLevels, resultAsDouble);
  unsigned int level = 1;
  for (auto image : results)
  {
    mitk::DataNode::Pointer result = mitk::DataNode::New();
    result->SetProperty("name", mitk::StringProperty::New(nodeName+"::MultiRes::Level-"+us::Any(numberOfLevels-level).ToString()));
    result->SetData(image);
    GetDataStorage()->Add(result, cb_image->GetSelectedNode());
    ++level;
  }
}

void QmitkRadiomicsTransformation::executeButtonWaveletPressed()
{
  QmitkDataStorageComboBox * cb_image = dynamic_cast<QmitkDataStorageComboBox *>(m_Controls->m_InputImageGroup->layout()->itemAt(1)->widget());
  mitk::BaseData* baseDataRawImage = NULL;

  mitk::Image::Pointer raw_image;
  std::string nodeName;
  if ((cb_image->GetSelectedNode().IsNotNull()))
  {
    baseDataRawImage = (cb_image->GetSelectedNode()->GetData());
    nodeName = cb_image->GetSelectedNode()->GetName();
  }
  if ((baseDataRawImage != NULL))
  {
    raw_image = dynamic_cast<mitk::Image *>(baseDataRawImage);
  }
  else {
    QMessageBox msgBox;
    msgBox.setText("Please specify the images that shlould be used.");
    msgBox.exec();
    return;
  }
  if (raw_image.IsNull())
  {
    QMessageBox msgBox;
    msgBox.setText("Error during processing the specified images.");
    msgBox.exec();
    return;
  }

  unsigned int numberOfLevels = m_Controls->m_WaveletNumberOfLevels->value();
  unsigned int numberOfBands = m_Controls->m_WaveletNumberOfBands->value();
  mitk::BorderCondition condition = mitk::BorderCondition::Constant;
  mitk::WaveletType waveletType = mitk::WaveletType::Held;

  std::string waveletStr = m_Controls->m_WaveletWavelet->currentText().toStdString();
  if (waveletStr == "Shannon")
  {
    waveletType = mitk::WaveletType::Shannon;
  }
  if (waveletStr == "Simoncelli")
  {
    waveletType = mitk::WaveletType::Simoncelli;
  }
  if (waveletStr == "Vow")
  {
    waveletType = mitk::WaveletType::Vow;
  }
  if (waveletStr == "Held")
  {
    waveletType = mitk::WaveletType::Held;
  }

  std::string conditionStr = m_Controls->m_WaveletBorderCondition->currentText().toStdString();
  if (conditionStr == "Constant")
  {
    condition = mitk::BorderCondition::Constant;
  }
  if (conditionStr == "Periodic")
  {
    condition = mitk::BorderCondition::Periodic;
  }
  if (conditionStr == "Zero Flux Neumann")
  {
    condition = mitk::BorderCondition::ZeroFluxNeumann;
  }

  auto results = mitk::TransformationOperation::WaveletForward(raw_image, numberOfLevels, numberOfBands, condition, waveletType);
  unsigned int level = 0;
  for (auto image : results)
  {
    mitk::DataNode::Pointer result = mitk::DataNode::New();
    result->SetProperty("name", mitk::StringProperty::New(nodeName + "::Wavelet::"+waveletStr+"-"+conditionStr+"::Level-" + us::Any(level).ToString()));
    result->SetData(image);
    GetDataStorage()->Add(result, cb_image->GetSelectedNode());
    ++level;
  }

}


void QmitkRadiomicsTransformation::executeButtonLoGPressed()
{
  QmitkDataStorageComboBox * cb_image = dynamic_cast<QmitkDataStorageComboBox *>(m_Controls->m_InputImageGroup->layout()->itemAt(1)->widget());
  mitk::BaseData* baseDataRawImage = NULL;

  mitk::Image::Pointer raw_image;
  std::string nodeName;
  if ((cb_image->GetSelectedNode().IsNotNull()))
  {
    baseDataRawImage = (cb_image->GetSelectedNode()->GetData());
    nodeName = cb_image->GetSelectedNode()->GetName();
  }
  if ((baseDataRawImage != NULL))
  {
    raw_image = dynamic_cast<mitk::Image *>(baseDataRawImage);
  }
  else {
    QMessageBox msgBox;
    msgBox.setText("Please specify the images that shlould be used.");
    msgBox.exec();
    return;
  }
  if (raw_image.IsNull())
  {
    QMessageBox msgBox;
    msgBox.setText("Error during processing the specified images.");
    msgBox.exec();
    return;
  }

  double sigma = m_Controls->m_LoGSigma->value();
  bool resultAsDouble = m_Controls->m_LogResultAsDouble->isChecked();

  auto results = mitk::TransformationOperation::LaplacianOfGaussian(raw_image, sigma, resultAsDouble);
  mitk::DataNode::Pointer result = mitk::DataNode::New();
  result->SetProperty("name", mitk::StringProperty::New(nodeName + "::LoG::Sigma-" + us::Any(sigma).ToString()));
  result->SetData(results);
  GetDataStorage()->Add(result, cb_image->GetSelectedNode());
}

void QmitkRadiomicsTransformation::executeButtonResamplingPressed()
{
  QmitkDataStorageComboBox * cb_image = dynamic_cast<QmitkDataStorageComboBox *>(m_Controls->m_InputImageGroup->layout()->itemAt(1)->widget());
  mitk::BaseData* baseDataRawImage = NULL;

  mitk::Image::Pointer raw_image;
  std::string nodeName;
  if ((cb_image->GetSelectedNode().IsNotNull()))
  {
    baseDataRawImage = (cb_image->GetSelectedNode()->GetData());
    nodeName = cb_image->GetSelectedNode()->GetName();
  }
  if ((baseDataRawImage != NULL))
  {
    raw_image = dynamic_cast<mitk::Image *>(baseDataRawImage);
  }
  else {
    QMessageBox msgBox;
    msgBox.setText("Please specify the images that shlould be used.");
    msgBox.exec();
    return;
  }
  if (raw_image.IsNull())
  {
    QMessageBox msgBox;
    msgBox.setText("Error during processing the specified images.");
    msgBox.exec();
    return;
  }

  mitk::ImageMappingInterpolator::Type interpolatorType;
  switch (m_Controls->comboInterpolationMode->currentIndex())
  {
  case 0:
    interpolatorType = mitk::ImageMappingInterpolator::Linear;
    break;

  case 1:
    interpolatorType = mitk::ImageMappingInterpolator::BSpline_3;
    break;

  case 2:
    interpolatorType = mitk::ImageMappingInterpolator::NearestNeighbor;
    break;

  case 3:
    interpolatorType = mitk::ImageMappingInterpolator::WSinc_Hamming;
    break;

  case 4:
    interpolatorType = mitk::ImageMappingInterpolator::WSinc_Welch;
    break;

  default:
    interpolatorType = mitk::ImageMappingInterpolator::Linear;
  }

  mitk::GridInterpolationPositionType gridPosition;
  switch (m_Controls->comboAxisAlignment->currentIndex())
  {
  case 0:
    gridPosition = mitk::GridInterpolationPositionType::OriginAligned;
    break;

  case 1:
    gridPosition = mitk::GridInterpolationPositionType::CenterAligned;
    break;

  case 2:
    gridPosition = mitk::GridInterpolationPositionType::SameSize;
    break;

  default:
    gridPosition = mitk::GridInterpolationPositionType::OriginAligned;
  }

  bool resultAsDouble = m_Controls->checkResamplingOutputAsDouble->isChecked();
  bool roundResult = m_Controls->checkResamplingRoundOutput->isChecked();

  mitk::Vector3D spacing;
  spacing.Fill(-1);
  if (m_Controls->checkResampleX->isChecked()) {
    spacing[0] = m_Controls->doubleSpinDimensionX->value();
  }
  if (m_Controls->checkResampleY->isChecked()) {
    spacing[1] = m_Controls->doubleSpinDimensionY->value();
  }
  if (m_Controls->checkResampleZ->isChecked()) {
    spacing[2] = m_Controls->doubleSpinDimensionZ->value();
  }


  mitk::Image::Pointer results;
  if (m_Controls->checkResampleAsMask->isChecked()) {
    results = mitk::TransformationOperation::ResampleMask(raw_image, spacing, interpolatorType, gridPosition);
    mitk::LabelSetImage::Pointer oldLabelImage = dynamic_cast<mitk::LabelSetImage *> (raw_image.GetPointer());
    if (oldLabelImage.IsNotNull())
    {
      mitk::LabelSetImage::Pointer labelResult = mitk::LabelSetImage::New();
      labelResult->InitializeByLabeledImage(results);
      labelResult->AddLabelSetToLayer(labelResult->GetActiveLayer(), oldLabelImage->GetLabelSet());
      results = dynamic_cast<mitk::Image*>(labelResult.GetPointer());
    }
  }
  else {
    results = mitk::TransformationOperation::ResampleImage(raw_image, spacing, interpolatorType, gridPosition, resultAsDouble, roundResult);
  }
  mitk::DataNode::Pointer result = mitk::DataNode::New();
  result->SetProperty("name", mitk::StringProperty::New(nodeName + "::Resampled" ));
  result->SetData(results);
  GetDataStorage()->Add(result, cb_image->GetSelectedNode());
}

void QmitkRadiomicsTransformation::SetFocus()
{
}

//datamanager selection changed
void QmitkRadiomicsTransformation::OnSelectionChanged(berry::IWorkbenchPart::Pointer, const QList<mitk::DataNode::Pointer>& nodes)
{
  //any nodes there?
  if (!nodes.empty())
  {

  }
}
