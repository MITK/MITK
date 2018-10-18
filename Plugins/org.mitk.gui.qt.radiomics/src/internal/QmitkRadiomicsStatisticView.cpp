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

#include "QmitkRadiomicsStatisticView.h"

// QT includes (GUI)
#include <qlabel.h>
#include <qspinbox.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qradiobutton.h>
#include <qmessagebox.h>
#include <QApplication>
#include <QClipboard>

// Berry includes (selection service)
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>

// MITK includes (GUI)
#include <QmitkDataStorageComboBox.h>
#include "QmitkDataNodeSelectionProvider.h"
#include "mitkDataNodeObject.h"

// MITK includes (general)
#include "mitkLabelSetImage.h"
#include <mitkAbstractGlobalImageFeature.h>
#include <mitkGIFCooccurenceMatrix.h>
#include <mitkGIFCooccurenceMatrix2.h>
#include <mitkGIFGreyLevelRunLength.h>
#include <mitkGIFFirstOrderStatistics.h>
#include <mitkGIFFirstOrderHistogramStatistics.h>
#include <mitkGIFFirstOrderNumericStatistics.h>
#include <mitkGIFVolumetricStatistics.h>
#include <mitkGIFVolumetricDensityStatistics.h>
#include <mitkGIFGreyLevelSizeZone.h>
#include <mitkGIFGreyLevelDistanceZone.h>
#include <mitkGIFImageDescriptionFeatures.h>
#include <mitkGIFLocalIntensity.h>
#include <mitkGIFCurvatureStatistic.h>
#include <mitkGIFIntensityVolumeHistogramFeatures.h>
#include <mitkGIFNeighbourhoodGreyToneDifferenceFeatures.h>
#include <mitkGIFNeighbouringGreyLevelDependenceFeatures.h>

#include <mitkNodePredicateDataType.h>
#include <mitkNodePredicateDimension.h>
#include <mitkNodePredicateAnd.h>
#include <mitkImageTimeSelector.h>
#include <mitkProperties.h>
#include <mitkLevelWindowProperty.h>
#include <mitkImageStatisticsHolder.h>

// Includes for image casting between ITK and MITK
#include <mitkImageCast.h>
#include <mitkITKImageImport.h>

// Specific GUI Includes
#include "QmitkGIFConfigurationPanel.h"

QmitkRadiomicsStatistic::QmitkRadiomicsStatistic()
: QmitkAbstractView(),
  m_Controls(nullptr)
{
}

QmitkRadiomicsStatistic::~QmitkRadiomicsStatistic()
{
  //berry::ISelectionService* s = GetSite()->GetWorkbenchWindow()->GetSelectionService();
  //if(s)
  //  s->RemoveSelectionListener(m_SelectionListener);
}

void QmitkRadiomicsStatistic::CreateQtPartControl(QWidget *parent)
{
  if (m_Controls == nullptr)
  {
    mitk::GIFImageDescriptionFeatures::Pointer ipCalculator = mitk::GIFImageDescriptionFeatures::New(); // Commented 2, Tested
    mitk::GIFFirstOrderStatistics::Pointer firstOrderCalculator = mitk::GIFFirstOrderStatistics::New(); //Commented 2
    mitk::GIFFirstOrderHistogramStatistics::Pointer firstOrderHistoCalculator = mitk::GIFFirstOrderHistogramStatistics::New(); // Commented 2, Tested
    mitk::GIFFirstOrderNumericStatistics::Pointer firstOrderNumericCalculator = mitk::GIFFirstOrderNumericStatistics::New(); // Commented 2, Tested
    mitk::GIFVolumetricStatistics::Pointer volCalculator = mitk::GIFVolumetricStatistics::New();   // Commented 2, Tested
    mitk::GIFVolumetricDensityStatistics::Pointer voldenCalculator = mitk::GIFVolumetricDensityStatistics::New(); // Commented 2, Tested
    mitk::GIFCooccurenceMatrix::Pointer coocCalculator = mitk::GIFCooccurenceMatrix::New(); // Commented 2, Will not be tested
    mitk::GIFCooccurenceMatrix2::Pointer cooc2Calculator = mitk::GIFCooccurenceMatrix2::New(); //Commented 2
    mitk::GIFNeighbouringGreyLevelDependenceFeature::Pointer ngldCalculator = mitk::GIFNeighbouringGreyLevelDependenceFeature::New(); //Commented 2, Tested
    mitk::GIFGreyLevelRunLength::Pointer rlCalculator = mitk::GIFGreyLevelRunLength::New(); // Commented 2
    mitk::GIFGreyLevelSizeZone::Pointer glszCalculator = mitk::GIFGreyLevelSizeZone::New(); // Commented 2, Tested
    mitk::GIFGreyLevelDistanceZone::Pointer gldzCalculator = mitk::GIFGreyLevelDistanceZone::New(); //Commented 2, Tested
    mitk::GIFLocalIntensity::Pointer lociCalculator = mitk::GIFLocalIntensity::New(); //Commented 2, Tested
    mitk::GIFIntensityVolumeHistogramFeatures::Pointer ivohCalculator = mitk::GIFIntensityVolumeHistogramFeatures::New(); // Commented 2
    mitk::GIFNeighbourhoodGreyToneDifferenceFeatures::Pointer ngtdCalculator = mitk::GIFNeighbourhoodGreyToneDifferenceFeatures::New(); //Commented 2, Tested
    mitk::GIFCurvatureStatistic::Pointer curvCalculator = mitk::GIFCurvatureStatistic::New(); //Commented 2, Tested

    std::vector<mitk::AbstractGlobalImageFeature::Pointer> features;
    features.push_back(volCalculator.GetPointer());
    features.push_back(voldenCalculator.GetPointer());
    features.push_back(curvCalculator.GetPointer());
    features.push_back(firstOrderCalculator.GetPointer());
    features.push_back(firstOrderNumericCalculator.GetPointer());
    features.push_back(firstOrderHistoCalculator.GetPointer());
    features.push_back(ivohCalculator.GetPointer());
    features.push_back(lociCalculator.GetPointer());
    features.push_back(cooc2Calculator.GetPointer());
    features.push_back(ngldCalculator.GetPointer());
    features.push_back(rlCalculator.GetPointer());
    features.push_back(glszCalculator.GetPointer());
    features.push_back(gldzCalculator.GetPointer());
    features.push_back(ipCalculator.GetPointer());
    features.push_back(ngtdCalculator.GetPointer());

    m_Controls = new Ui::QmitkRadiomicsStatisticViewControls;
    m_Controls->setupUi(parent);

    for (auto cFeature : features)
    {
      mitkUI::GIFConfigurationPanel* gifPanel = new mitkUI::GIFConfigurationPanel(parent, cFeature);
      m_Controls->m_FeaturesGroup->layout()->addWidget(gifPanel);
    }

    QLabel * label1 = new QLabel("Image: ");
    QLabel * label2 = new QLabel("Mask: ");
    QmitkDataStorageComboBox * cb_inputimage = new QmitkDataStorageComboBox(this->GetDataStorage(), mitk::TNodePredicateDataType<mitk::Image>::New());
    QmitkDataStorageComboBox * cb_maskimage = new QmitkDataStorageComboBox(this->GetDataStorage(), mitk::TNodePredicateDataType<mitk::LabelSetImage>::New());
    m_Controls->m_InputImageGroup->layout()->addWidget(label1);
    m_Controls->m_InputImageGroup->layout()->addWidget(cb_inputimage);
    m_Controls->m_InputImageGroup->layout()->addWidget(label2);
    m_Controls->m_InputImageGroup->layout()->addWidget(cb_maskimage);

    this->CreateConnections();

    //setup predictaes for combobox

    mitk::NodePredicateDimension::Pointer dimensionPredicate = mitk::NodePredicateDimension::New(3);
    mitk::NodePredicateDataType::Pointer imagePredicate = mitk::NodePredicateDataType::New("Image");


  }
}

void QmitkRadiomicsStatistic::CreateConnections()
{
  if ( m_Controls )
  {
    connect( (QObject*)(m_Controls->buttonExecute), SIGNAL(clicked() ), this, SLOT(executeButtonPressed() ) );
    connect((QObject*)(m_Controls->buttonExecuteAndAppend), SIGNAL(clicked()), this, SLOT(executeAndAppendButtonPressed()));
    connect((QObject*)(m_Controls->buttonTableToClipboard), SIGNAL(clicked()), this, SLOT(copyToClipboardButtonPressed()));
  }
}

std::map < std::string, us::Any> QmitkRadiomicsStatistic::GenerateParameters()
{
  std::map < std::string, us::Any> parameter;

  if (m_Controls->m_SetMinimumIntensity->isChecked())
  {
    parameter["minimum-intensity"] = us::Any(float(m_Controls->m_ParamMinimumIntensity->value()));
  }
  if (m_Controls->m_SetMaximumIntensity->isChecked())
  {
    parameter["maximum-intensity"] = us::Any(float(m_Controls->m_ParamMaximumIntensity->value()));
  }
  if (m_Controls->m_SetNumberOfBins->isChecked())
  {
    parameter["bins"] = us::Any(m_Controls->m_ParamBins->value());
  }
  if (m_Controls->m_SetBinSize->isChecked())
  {
    parameter["binsize"] = us::Any(float(m_Controls->m_ParamBinSize->value()));
  }
  if (m_Controls->m_SetIgnoreBinSize->isChecked())
  {
    parameter["ignore-mask-for-histogram"] = us::Any(m_Controls->m_ParamIgnoreMask->isChecked());
  }
  if (m_Controls->m_SetEncodeParameterInName->isChecked())
  {
    parameter["encode-parameter-in-name"] = us::Any(m_Controls->m_ParamEncodeName->isChecked());
  }
  if (m_Controls->m_SetDirectionParameter->isChecked())
  {
    parameter["direction"] = us::Any(m_Controls->m_ParamDirection->value());
  }
  if (m_Controls->m_SetSliceWiseParameter->isChecked())
  {
    parameter["slice-wise"] = us::Any(m_Controls->m_ParamSliceWise->value());
  }

  return parameter;
}

void QmitkRadiomicsStatistic::executeButtonPressed()
{
  QmitkDataStorageComboBox * cb_image = dynamic_cast<QmitkDataStorageComboBox *>(m_Controls->m_InputImageGroup->layout()->itemAt(1)->widget());
  QmitkDataStorageComboBox * cb_maskimage = dynamic_cast<QmitkDataStorageComboBox *>(m_Controls->m_InputImageGroup->layout()->itemAt(3)->widget());
  mitk::BaseData* baseDataRawImage = NULL;
  mitk::BaseData* baseDataMaskImage = NULL;

  mitk::Image::Pointer raw_image;
  mitk::Image::Pointer mask_image;
  QString imageName;
  QString maskName;

  if ((cb_image->GetSelectedNode().IsNotNull() ) && (cb_maskimage->GetSelectedNode().IsNotNull()))
  {
    imageName = cb_image->GetSelectedNode()->GetName().c_str();
    maskName = cb_maskimage->GetSelectedNode()->GetName().c_str();
    baseDataRawImage = (cb_image->GetSelectedNode()->GetData());
    baseDataMaskImage = (cb_maskimage->GetSelectedNode()->GetData());
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

  mitk::AbstractGlobalImageFeature::FeatureListType stats;
  for (int i = 0; i < m_Controls->m_FeaturesGroup->layout()->count(); ++i)
  {
    auto parameter = this->GenerateParameters();

    mitkUI::GIFConfigurationPanel* gifPanel = dynamic_cast<mitkUI::GIFConfigurationPanel*>(m_Controls->m_FeaturesGroup->layout()->itemAt(i)->widget());
    if (gifPanel == NULL)
      continue;
    gifPanel->CalculateFeaturesUsingParameters(raw_image, mask_image, parameter, stats);
  }

  m_Controls->m_ResultTable->setRowCount(stats.size());

  for (std::size_t i = 0; i < stats.size(); ++i)
  {
    m_Controls->m_ResultTable->setItem(i, 0, new QTableWidgetItem(imageName));
    m_Controls->m_ResultTable->setItem(i, 1, new QTableWidgetItem(maskName));
    m_Controls->m_ResultTable->setItem(i, 2, new QTableWidgetItem(stats[i].first.c_str()));
    m_Controls->m_ResultTable->setItem(i, 3, new QTableWidgetItem(QString::number(stats[i].second)));
  }
  }

void QmitkRadiomicsStatistic::executeAndAppendButtonPressed()
{
  std::vector<QTableWidgetItem *> elementImage;
  std::vector<QTableWidgetItem *> elementMask;
  std::vector<QTableWidgetItem *> elementText;
  std::vector<QTableWidgetItem *> elementValue;

  for (int i = 0; i < m_Controls->m_ResultTable->rowCount(); ++i)
  {
    auto itemImage = m_Controls->m_ResultTable->item(i, 0)->clone();
    auto itemMask = m_Controls->m_ResultTable->item(i, 1)->clone();
    auto itemText = m_Controls->m_ResultTable->item(i, 2)->clone();
    auto itemValue = m_Controls->m_ResultTable->item(i, 3)->clone();
    elementImage.push_back(itemImage);
    elementMask.push_back(itemMask);
    elementText.push_back(itemText);
    elementValue.push_back(itemValue);
  }

  executeButtonPressed();

  std::size_t oldSize = m_Controls->m_ResultTable->rowCount();
  m_Controls->m_ResultTable->setRowCount(oldSize + elementText.size());

  for (std::size_t i = 0; i < elementText.size(); ++i)
  {
    m_Controls->m_ResultTable->setItem(i + oldSize, 0, elementImage[i]);
    m_Controls->m_ResultTable->setItem(i + oldSize, 1, elementMask[i]);
    m_Controls->m_ResultTable->setItem(i+oldSize, 2, elementText[i]);
    m_Controls->m_ResultTable->setItem(i+oldSize, 3, elementValue[i]);
  }
}

void QmitkRadiomicsStatistic::copyToClipboardButtonPressed()
{
  QString selectedText;
  for (int i = 0; i < m_Controls->m_ResultTable->rowCount(); ++i)
  {
    auto itemImage = m_Controls->m_ResultTable->item(i, 0);
    auto itemMask = m_Controls->m_ResultTable->item(i, 1);
    auto itemText = m_Controls->m_ResultTable->item(i, 2);
    auto itemValue = m_Controls->m_ResultTable->item(i, 3);
    selectedText.append(itemImage->text());
    selectedText.append(";");
    selectedText.append(itemMask->text());
    selectedText.append(";");
    selectedText.append(itemText->text());
    selectedText.append(";");
    selectedText.append(itemValue->text());
    selectedText.append("\n");
  }
  QApplication::clipboard()->setText(selectedText);
}

void QmitkRadiomicsStatistic::SetFocus()
{
}

//datamanager selection changed
void QmitkRadiomicsStatistic::OnSelectionChanged(berry::IWorkbenchPart::Pointer, const QList<mitk::DataNode::Pointer>& nodes)
{
  //any nodes there?
  if (!nodes.empty())
  {

  }
}
