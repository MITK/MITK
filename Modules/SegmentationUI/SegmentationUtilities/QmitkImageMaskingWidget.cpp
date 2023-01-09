/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkImageMaskingWidget.h"
#include <ui_QmitkImageMaskingWidgetControls.h>

#include <mitkDataStorage.h>
#include <mitkException.h>
#include <mitkExceptionMacro.h>
#include <mitkImage.h>
#include <mitkImageStatisticsHolder.h>
#include <mitkMaskImageFilter.h>
#include <mitkProgressBar.h>
#include <mitkSliceNavigationController.h>
#include <mitkSurfaceToImageFilter.h>
#include <mitkImageAccessByItk.h>
#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateDimension.h>
#include <mitkNodePredicateGeometry.h>
#include <mitkNodePredicateNot.h>

#include <QMessageBox>

#include <limits>

namespace
{
  bool IsSurface(const mitk::DataNode* dataNode)
  {
    if (nullptr != dataNode)
    {
      if (nullptr != dynamic_cast<const mitk::Surface*>(dataNode->GetData()))
        return true;
    }

    return false;
  }
}

static const char* const HelpText = "Select an image and a segmentation or surface";

QmitkImageMaskingWidget::QmitkImageMaskingWidget(mitk::DataStorage* dataStorage,
                                                 mitk::SliceNavigationController* timeNavigationController,
                                                 QWidget* parent)
  : QmitkSegmentationUtilityWidget(timeNavigationController, parent)
{
  m_Controls = new Ui::QmitkImageMaskingWidgetControls;
  m_Controls->setupUi(this);

  m_Controls->dataSelectionWidget->SetDataStorage(dataStorage);
  m_Controls->dataSelectionWidget->AddDataSelection(QmitkDataSelectionWidget::ImagePredicate);
  m_Controls->dataSelectionWidget->AddDataSelection(QmitkDataSelectionWidget::SegmentationOrSurfacePredicate);
  m_Controls->dataSelectionWidget->SetHelpText(HelpText);

  // T28795: Disable 2-d reference images since they do not work yet (segmentations are at least 3-d images with a single slice)
  m_Controls->dataSelectionWidget->SetPredicate(0, mitk::NodePredicateAnd::New(
    mitk::NodePredicateNot::New(mitk::NodePredicateDimension::New(2)),
    m_Controls->dataSelectionWidget->GetPredicate(0)));

  this->EnableButtons(false);

  connect(m_Controls->btnMaskImage, SIGNAL(clicked()), this, SLOT(OnMaskImagePressed()));
  connect(m_Controls->rbnCustom, SIGNAL(toggled(bool)), this, SLOT(OnCustomValueButtonToggled(bool)));
  connect(m_Controls->dataSelectionWidget, SIGNAL(SelectionChanged(unsigned int, const mitk::DataNode*)),
    this, SLOT(OnSelectionChanged(unsigned int, const mitk::DataNode*)));

  if( m_Controls->dataSelectionWidget->GetSelection(0).IsNotNull() &&
    m_Controls->dataSelectionWidget->GetSelection(1).IsNotNull() )
  {
    this->OnSelectionChanged(0, m_Controls->dataSelectionWidget->GetSelection(0));
  }
}

QmitkImageMaskingWidget::~QmitkImageMaskingWidget()
{
}

void QmitkImageMaskingWidget::OnSelectionChanged(unsigned int index, const mitk::DataNode *selection)
{
  auto *dataSelectionWidget = m_Controls->dataSelectionWidget;
  auto node0 = dataSelectionWidget->GetSelection(0);

  if (index == 0)
  {
    dataSelectionWidget->SetPredicate(1, QmitkDataSelectionWidget::SegmentationOrSurfacePredicate);

    if (node0.IsNotNull())
    {
      dataSelectionWidget->SetPredicate(1, mitk::NodePredicateAnd::New(
        mitk::NodePredicateGeometry::New(node0->GetData()->GetGeometry()),
        dataSelectionWidget->GetPredicate(1)));
    }
  }

  auto node1 = dataSelectionWidget->GetSelection(1);

  if (node0.IsNull() || node1.IsNull())
  {
    dataSelectionWidget->SetHelpText(HelpText);
    this->EnableButtons(false);
  }
  else
  {
    this->SelectionControl(index, selection);
  }
}

void QmitkImageMaskingWidget::SelectionControl(unsigned int index, const mitk::DataNode* selection)
{
  QmitkDataSelectionWidget* dataSelectionWidget = m_Controls->dataSelectionWidget;
  mitk::DataNode::Pointer node = dataSelectionWidget->GetSelection(index);

  //if Image-Masking is enabled, check if image-dimension of reference and binary image is identical
  if( !IsSurface(dataSelectionWidget->GetSelection(1)) )
  {
    if( dataSelectionWidget->GetSelection(0) == dataSelectionWidget->GetSelection(1) )
    {
      dataSelectionWidget->SetHelpText("Select two different images above");
      this->EnableButtons(false);
      return;
    }

    else if( node.IsNotNull() && selection )
    {
      mitk::Image::Pointer referenceImage = dynamic_cast<mitk::Image*> ( dataSelectionWidget->GetSelection(0)->GetData() );
      mitk::Image::Pointer maskImage = dynamic_cast<mitk::Image*> ( dataSelectionWidget->GetSelection(1)->GetData() );

      if (maskImage.IsNull())
      {
        dataSelectionWidget->SetHelpText("Different image sizes cannot be masked");
        this->EnableButtons(false);
        return;
      }
    }

    else
    {
      dataSelectionWidget->SetHelpText(HelpText);
      return;
    }
  }

  dataSelectionWidget->SetHelpText("");
  this->EnableButtons();
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

  QmitkDataSelectionWidget* dataSelectionWidget = m_Controls->dataSelectionWidget;

  //create result image, get mask node and reference image
  mitk::Image::Pointer resultImage(nullptr);
  mitk::DataNode::Pointer maskingNode = dataSelectionWidget->GetSelection(1);
  mitk::Image::Pointer referenceImage = static_cast<mitk::Image*>(dataSelectionWidget->GetSelection(0)->GetData());

  if(referenceImage.IsNull() || maskingNode.IsNull() )
  {
    MITK_ERROR << "Selection does not contain an image";
    QMessageBox::information( this, "Image and Surface Masking", "Selection does not contain an image", QMessageBox::Ok );
    m_Controls->btnMaskImage->setEnabled(true);
    return;
  }

  //Do Image-Masking
  if (!IsSurface(maskingNode))
  {
    mitk::ProgressBar::GetInstance()->Progress();

    mitk::Image::Pointer maskImage = dynamic_cast<mitk::Image*> ( maskingNode->GetData() );

    if(maskImage.IsNull() )
    {
      MITK_ERROR << "Selection does not contain a segmentation";
      QMessageBox::information( this, "Image and Surface Masking", "Selection does not contain a segmentation", QMessageBox::Ok );
      this->EnableButtons();
      return;
    }

    resultImage = this->MaskImage(referenceImage, maskImage);
  }

  //Do Surface-Masking
  else
  {
    mitk::ProgressBar::GetInstance()->Progress();

    //1. convert surface to image
    mitk::Surface::Pointer surface = dynamic_cast<mitk::Surface*> ( maskingNode->GetData() );

    //TODO Get 3D Surface of current time step

    if(surface.IsNull())
    {
      MITK_ERROR << "Selection does not contain a surface";
      QMessageBox::information( this, "Image and Surface Masking", "Selection does not contain a surface", QMessageBox::Ok );
      this->EnableButtons();
      return;
    }

    mitk::Image::Pointer maskImage = this->ConvertSurfaceToImage( referenceImage, surface );

    //2. mask reference image with mask image
    if(maskImage.IsNotNull() &&
       referenceImage->GetLargestPossibleRegion().GetSize() == maskImage->GetLargestPossibleRegion().GetSize() )
    {
      resultImage = this->MaskImage( referenceImage, maskImage );
    }
  }

  mitk::ProgressBar::GetInstance()->Progress();

  if( resultImage.IsNull() )
  {
    MITK_ERROR << "Masking failed";
    QMessageBox::information( this, "Image and Surface Masking", "Masking failed. For more information please see logging window.", QMessageBox::Ok );
    this->EnableButtons();
    mitk::ProgressBar::GetInstance()->Progress(4);
    return;
  }

  //Add result to data storage
  this->AddToDataStorage(
    dataSelectionWidget->GetDataStorage(),
    resultImage,
    dataSelectionWidget->GetSelection(0)->GetName() + "_" + dataSelectionWidget->GetSelection(1)->GetName(),
    dataSelectionWidget->GetSelection(0));

  this->EnableButtons();

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

mitk::Image::Pointer QmitkImageMaskingWidget::ConvertSurfaceToImage( mitk::Image::Pointer image, mitk::Surface::Pointer surface )
{
  mitk::ProgressBar::GetInstance()->AddStepsToDo(2);
  mitk::ProgressBar::GetInstance()->Progress();

  mitk::SurfaceToImageFilter::Pointer surfaceToImageFilter = mitk::SurfaceToImageFilter::New();
  surfaceToImageFilter->MakeOutputBinaryOn();
  surfaceToImageFilter->SetInput(surface);
  surfaceToImageFilter->SetImage(image);
  try
  {
    surfaceToImageFilter->Update();
  }
  catch(itk::ExceptionObject& excpt)
  {
    MITK_ERROR << excpt.GetDescription();
    return nullptr;
  }

  mitk::ProgressBar::GetInstance()->Progress();
  mitk::Image::Pointer resultImage = mitk::Image::New();
  resultImage = surfaceToImageFilter->GetOutput();

  return resultImage;
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
