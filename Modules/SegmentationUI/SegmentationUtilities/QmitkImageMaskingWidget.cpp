/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkImageMaskingWidget.h"
#include <ui_QmitkImageMaskingWidgetControls.h>

#include <mitkException.h>
#include <mitkExceptionMacro.h>
#include <mitkImage.h>
#include <mitkImageStatisticsHolder.h>
#include <mitkMaskImageFilter.h>
#include <mitkProgressBar.h>
#include <mitkImageAccessByItk.h>
#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateGeometry.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateDataType.h>
#include <mitkNodePredicateProperty.h>
#include <mitkMultiLabelPredicateHelper.h>
#include <mitkLabelSetImageConverter.h>

#include <QMessageBox>

#include <limits>

namespace
{
  mitk::NodePredicateBase::Pointer GetInputPredicate()
  {
    auto isImage = mitk::TNodePredicateDataType<mitk::Image>::New();
    auto isNotSeg = mitk::NodePredicateNot::New(mitk::GetMultiLabelSegmentationPredicate());

    auto isValidInput = mitk::NodePredicateAnd::New(isNotSeg, isImage);
    isValidInput->AddPredicate(mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("helper object")));
    isValidInput->AddPredicate(mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("hidden object")));
    return isValidInput.GetPointer();
  }
}

static const char* const HelpText = "Select an image and a segmentation or surface";

QmitkImageMaskingWidget::QmitkImageMaskingWidget(mitk::DataStorage* dataStorage, QWidget* parent)
  : QWidget(parent), m_DataStorage(dataStorage)
{
  m_Controls = new Ui::QmitkImageMaskingWidgetControls;
  m_Controls->setupUi(this);

  m_Controls->imageNodeSelector->SetDataStorage(dataStorage);
  m_Controls->imageNodeSelector->SetNodePredicate(GetInputPredicate());
  m_Controls->imageNodeSelector->SetSelectionIsOptional(false);
  m_Controls->imageNodeSelector->SetInvalidInfo(QStringLiteral("Please select image for masking"));
  m_Controls->imageNodeSelector->SetPopUpTitel(QStringLiteral("Select image"));
  m_Controls->imageNodeSelector->SetPopUpHint(QStringLiteral("Select an image that you want to mask."));

  m_Controls->segNodeSelector->SetDataStorage(dataStorage);
  m_Controls->segNodeSelector->SetNodePredicate(mitk::GetMultiLabelSegmentationPredicate());
  m_Controls->segNodeSelector->SetSelectionIsOptional(false);
  m_Controls->segNodeSelector->SetInvalidInfo(QStringLiteral("Please select segmentation and its label"));
  m_Controls->segNodeSelector->SetPopUpTitel(QStringLiteral("Select segmentation"));
  m_Controls->segNodeSelector->SetPopUpHint(QStringLiteral("Select the segmentation that should be used for masking.\nSegmentation must have the same geometry like the image that should be masked."));

  this->ConfigureWidgets();

  connect(m_Controls->imageNodeSelector, &QmitkAbstractNodeSelectionWidget::CurrentSelectionChanged,
    this, &QmitkImageMaskingWidget::OnImageSelectionChanged);
  connect(m_Controls->segNodeSelector, &QmitkAbstractNodeSelectionWidget::CurrentSelectionChanged,
    this, &QmitkImageMaskingWidget::OnSegSelectionChanged);

  connect(m_Controls->btnMaskImage, &QPushButton::clicked, this, &QmitkImageMaskingWidget::OnMaskImagePressed);
  connect(m_Controls->rbnCustom, &QRadioButton::toggled, this, &QmitkImageMaskingWidget::OnCustomValueButtonToggled);

  m_Controls->imageNodeSelector->SetAutoSelectNewNodes(true);
  m_Controls->segNodeSelector->SetAutoSelectNewNodes(true);
}

QmitkImageMaskingWidget::~QmitkImageMaskingWidget()
{
  m_Controls->labelInspector->SetMultiLabelNode(nullptr);
}

void QmitkImageMaskingWidget::OnImageSelectionChanged(QmitkAbstractNodeSelectionWidget::NodeList /*nodes*/)
{
  auto imageNode = m_Controls->imageNodeSelector->GetSelectedNode();
  const mitk::BaseGeometry* refGeometry = nullptr;
  const mitk::BaseData* inputImage = (nullptr != imageNode) ? imageNode->GetData() : nullptr;
  if (nullptr != inputImage)
  {
    refGeometry = inputImage->GetGeometry();
  }

  m_Controls->segNodeSelector->SetNodePredicate(mitk::GetMultiLabelSegmentationPredicate(refGeometry));
  this->ConfigureWidgets();
}

void QmitkImageMaskingWidget::OnSegSelectionChanged(QmitkAbstractNodeSelectionWidget::NodeList /*nodes*/)
{
  auto node = m_Controls->segNodeSelector->GetSelectedNode();
  m_Controls->labelInspector->SetMultiLabelNode(node);
  if (node.IsNotNull())
  {
    auto labelValues = m_Controls->labelInspector->GetMultiLabelSegmentation()->GetAllLabelValues();
    if (!labelValues.empty())
    {
      m_Controls->labelInspector->SetSelectedLabel(labelValues.front());
    }
  }
  this->ConfigureWidgets();
}

void QmitkImageMaskingWidget::ConfigureWidgets()
{
  auto iNode = m_Controls->imageNodeSelector->GetSelectedNode();
  auto sNode = m_Controls->segNodeSelector->GetSelectedNode();

  bool enable = iNode.IsNotNull() && sNode.IsNotNull() && !m_Controls->labelInspector->GetSelectedLabels().empty();

  this->EnableButtons(enable);
}

void QmitkImageMaskingWidget::EnableButtons(bool enable)
{
  m_Controls->grpBackgroundValue->setEnabled(enable);
  m_Controls->btnMaskImage->setEnabled(enable);
}

template<typename TPixel, unsigned int VImageDimension>
void GetRange(const itk::Image<TPixel, VImageDimension>*, double& bottom, double& top)
{
  bottom = std::numeric_limits<TPixel>::lowest();
  top = std::numeric_limits<TPixel>::max();
}

void QmitkImageMaskingWidget::OnCustomValueButtonToggled(bool checked)
{
  m_Controls->txtCustom->setEnabled(checked);
}

void QmitkImageMaskingWidget::OnMaskImagePressed()
{
  //Disable Buttons during calculation and initialize Progressbar
  this->EnableButtons(false);
  mitk::ProgressBar::GetInstance()->AddStepsToDo(4);
  mitk::ProgressBar::GetInstance()->Progress();

  //create result image, get mask node and reference image
  mitk::Image::Pointer resultImage(nullptr);
  mitk::LabelSetImage::Pointer segmentation = m_Controls->labelInspector->GetMultiLabelSegmentation();
  mitk::Image::Pointer referenceImage = static_cast<mitk::Image*>(m_Controls->imageNodeSelector->GetSelectedNode()->GetData());

  mitk::ProgressBar::GetInstance()->Progress();

  auto labelImage = mitk::CreateLabelMask(segmentation, m_Controls->labelInspector->GetSelectedLabels().front(), true);
  resultImage = this->MaskImage(referenceImage, labelImage);

  mitk::ProgressBar::GetInstance()->Progress();

  if( resultImage.IsNull() )
  {
    MITK_ERROR << "Masking failed";
    QMessageBox::information( this, "Image Masking", "Masking failed. For more information please see logging window.", QMessageBox::Ok );
    this->EnableButtons(true);
    mitk::ProgressBar::GetInstance()->Progress(4);
    return;
  }

  //Add result to data storage
  this->AddToDataStorage(m_DataStorage.Lock(),
    resultImage,
    m_Controls->imageNodeSelector->GetSelectedNode()->GetName() + "_" + m_Controls->segNodeSelector->GetSelectedNode()->GetName(),
    m_Controls->imageNodeSelector->GetSelectedNode());

  this->EnableButtons(true);

  mitk::ProgressBar::GetInstance()->Progress();
}

mitk::Image::Pointer QmitkImageMaskingWidget::MaskImage(mitk::Image::Pointer referenceImage, mitk::Image::Pointer maskImage )
{
  mitk::ScalarType backgroundValue = 0.0;

  if (m_Controls->rbnMinimum->isChecked())
  {
    backgroundValue = referenceImage->GetStatistics()->GetScalarValueMin();
  }
  else if (m_Controls->rbnCustom->isChecked())
  {
    auto warningTitle = QStringLiteral("Invalid custom pixel value");

    bool ok = false;
    auto originalBackgroundValue = m_Controls->txtCustom->text().toDouble(&ok);

    if (!ok)
    {
      // Input is not even a number
      QMessageBox::warning(nullptr, warningTitle, "Please enter a valid number as custom pixel value.");
      return nullptr;
    }
    else
    {
      // Clamp to the numerical limits of the pixel/component type
      double bottom, top;
      if (referenceImage->GetDimension() == 4)
      {
        AccessFixedDimensionByItk_n(referenceImage, GetRange, 4, (bottom, top));
      }
      else
      {
        AccessByItk_n(referenceImage, GetRange, (bottom, top));
      }
      backgroundValue = std::max(bottom, std::min(originalBackgroundValue, top));

      // Get rid of decimals for integral numbers
      auto type = referenceImage->GetPixelType().GetComponentType();
      if (type != itk::IOComponentEnum::FLOAT && type != itk::IOComponentEnum::DOUBLE)
        backgroundValue = std::round(backgroundValue);
    }

    // Ask the user for permission before correcting their input
    if (std::abs(originalBackgroundValue - backgroundValue) > 1e-4)
    {
      auto warningText = QString(
        "<p>The custom pixel value <b>%1</b> lies not within the range of valid pixel values for the selected image.</p>"
        "<p>Apply the closest valid pixel value <b>%2</b> instead?</p>").arg(originalBackgroundValue).arg(backgroundValue);

      auto ret = QMessageBox::warning(
        nullptr,
        warningTitle,
        warningText,
        QMessageBox::StandardButton::Apply | QMessageBox::StandardButton::Cancel,
        QMessageBox::StandardButton::Apply);

      if (QMessageBox::StandardButton::Apply != ret)
        return nullptr;

      m_Controls->txtCustom->setText(QString("%1").arg(backgroundValue));
    }
  }

  auto maskFilter = mitk::MaskImageFilter::New();
  maskFilter->SetInput(referenceImage);
  maskFilter->SetMask(maskImage);
  maskFilter->OverrideOutsideValueOn();
  maskFilter->SetOutsideValue(backgroundValue);

  try
  {
    maskFilter->Update();
  }
  catch(const itk::ExceptionObject& e)
  {
    MITK_ERROR << e.GetDescription();
    return nullptr;
  }

  return maskFilter->GetOutput();
}

void QmitkImageMaskingWidget::AddToDataStorage(mitk::DataStorage::Pointer dataStorage, mitk::Image::Pointer segmentation, const std::string& name, mitk::DataNode::Pointer parent )
{
  if (dataStorage.IsNull())
  {
    std::string exception = "Cannot add result to the data storage. Data storage invalid.";
    MITK_ERROR << "Masking failed: " << exception;
    QMessageBox::information(nullptr, "Masking failed", QString::fromStdString(exception));
  }

  auto dataNode = mitk::DataNode::New();
  dataNode->SetName(name);
  dataNode->SetData(segmentation);

  if (parent.IsNotNull())
  {
    mitk::LevelWindow levelWindow;
    parent->GetLevelWindow(levelWindow);
    dataNode->SetLevelWindow(levelWindow);
  }

  dataStorage->Add(dataNode, parent);
}
