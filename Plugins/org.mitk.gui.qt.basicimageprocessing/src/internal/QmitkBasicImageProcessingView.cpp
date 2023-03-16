/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkBasicImageProcessingView.h"

// QT includes (GUI)
#include <qlabel.h>
#include <qspinbox.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qradiobutton.h>
#include <qmessagebox.h>

// MITK includes (general)
#include <mitkNodePredicateDataType.h>
#include <mitkNodePredicateDimension.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateOr.h>
#include <mitkNodePredicateProperty.h>
#include <mitkImageTimeSelector.h>
#include <mitkVectorImageMapper2D.h>
#include <mitkProperties.h>
#include <mitkLevelWindowProperty.h>
#include <mitkImageStatisticsHolder.h>

// Includes for image casting between ITK and MITK
#include <mitkImageCast.h>
#include <mitkITKImageImport.h>

// ITK includes (general)
#include <itkVectorImage.h>
#include <itkImageFileWriter.h>

// Morphological Operations
#include <itkBinaryBallStructuringElement.h>
#include <itkGrayscaleDilateImageFilter.h>
#include <itkGrayscaleErodeImageFilter.h>
#include <itkGrayscaleMorphologicalOpeningImageFilter.h>
#include <itkGrayscaleMorphologicalClosingImageFilter.h>

// Smoothing
#include <itkMedianImageFilter.h>
#include <itkDiscreteGaussianImageFilter.h>
#include <itkTotalVariationDenoisingImageFilter.h>

// Threshold
#include <itkBinaryThresholdImageFilter.h>

// Inversion
#include <itkInvertIntensityImageFilter.h>

// Derivatives
#include <itkGradientMagnitudeRecursiveGaussianImageFilter.h>
#include <itkLaplacianImageFilter.h>
#include <itkSobelEdgeDetectionImageFilter.h>

// Resampling
#include <itkResampleImageFilter.h>
#include <itkNearestNeighborInterpolateImageFunction.h>
#include <itkBSplineInterpolateImageFunction.h>
#include <itkCastImageFilter.h>
#include <itkLinearInterpolateImageFunction.h>

// Image Arithmetics
#include <itkAddImageFilter.h>
#include <itkSubtractImageFilter.h>
#include <itkMultiplyImageFilter.h>
#include <itkDivideImageFilter.h>

// Boolean operations
#include <itkOrImageFilter.h>
#include <itkAndImageFilter.h>
#include <itkXorImageFilter.h>

// Flip Image
#include <itkFlipImageFilter.h>

#include <itkRescaleIntensityImageFilter.h>
#include <itkShiftScaleImageFilter.h>

// Convenient Definitions
typedef itk::Image<short, 3>                                                            ImageType;
typedef itk::Image<unsigned char, 3>                                                    SegmentationImageType;
typedef itk::Image<double, 3>                                                           DoubleImageType;
typedef itk::Image<itk::Vector<float,3>, 3>                                             VectorImageType;

typedef itk::BinaryBallStructuringElement<ImageType::PixelType, 3>                      BallType;
typedef itk::GrayscaleDilateImageFilter<ImageType, ImageType, BallType>                 DilationFilterType;
typedef itk::GrayscaleErodeImageFilter<ImageType, ImageType, BallType>                  ErosionFilterType;
typedef itk::GrayscaleMorphologicalOpeningImageFilter<ImageType, ImageType, BallType>   OpeningFilterType;
typedef itk::GrayscaleMorphologicalClosingImageFilter<ImageType, ImageType, BallType>   ClosingFilterType;

typedef itk::MedianImageFilter< ImageType, ImageType >                                  MedianFilterType;
typedef itk::DiscreteGaussianImageFilter< ImageType, ImageType>                         GaussianFilterType;
typedef itk::TotalVariationDenoisingImageFilter<DoubleImageType, DoubleImageType>       TotalVariationFilterType;
typedef itk::TotalVariationDenoisingImageFilter<VectorImageType, VectorImageType>       VectorTotalVariationFilterType;

typedef itk::BinaryThresholdImageFilter< ImageType, ImageType >                         ThresholdFilterType;
typedef itk::InvertIntensityImageFilter< ImageType, ImageType >                         InversionFilterType;

typedef itk::GradientMagnitudeRecursiveGaussianImageFilter< ImageType, ImageType >      GradientFilterType;
typedef itk::LaplacianImageFilter< DoubleImageType, DoubleImageType >                   LaplacianFilterType;
typedef itk::SobelEdgeDetectionImageFilter< DoubleImageType, DoubleImageType >          SobelFilterType;

typedef itk::ResampleImageFilter< ImageType, ImageType >                                ResampleImageFilterType;
typedef itk::ResampleImageFilter< ImageType, ImageType >                                ResampleImageFilterType2;
typedef itk::CastImageFilter< ImageType, DoubleImageType >                              ImagePTypeToFloatPTypeCasterType;

typedef itk::AddImageFilter< ImageType, ImageType, ImageType >                          AddFilterType;
typedef itk::SubtractImageFilter< ImageType, ImageType, ImageType >                     SubtractFilterType;
typedef itk::MultiplyImageFilter< ImageType, ImageType, ImageType >                     MultiplyFilterType;
typedef itk::DivideImageFilter< ImageType, ImageType, DoubleImageType >                 DivideFilterType;

typedef itk::OrImageFilter< ImageType, ImageType >                                      OrImageFilterType;
typedef itk::AndImageFilter< ImageType, ImageType >                                     AndImageFilterType;
typedef itk::XorImageFilter< ImageType, ImageType >                                     XorImageFilterType;

typedef itk::FlipImageFilter< ImageType >                                               FlipImageFilterType;

typedef itk::LinearInterpolateImageFunction< ImageType, double >                        LinearInterpolatorType;
typedef itk::NearestNeighborInterpolateImageFunction< ImageType, double >               NearestInterpolatorType;

const std::string QmitkBasicImageProcessing::VIEW_ID = "org.mitk.views.basicimageprocessing";

QmitkBasicImageProcessing::QmitkBasicImageProcessing()
  : QmitkAbstractView()
  , m_Controls(new Ui::QmitkBasicImageProcessingViewControls)
  , m_TimeStepperAdapter(nullptr)
{
  auto isImage = mitk::TNodePredicateDataType<mitk::Image>::New();
  auto isNotHelperObject = mitk::NodePredicateNot::New(
    mitk::NodePredicateProperty::New("helper object", mitk::BoolProperty::New(true)));
  auto dimensionPredicate = mitk::NodePredicateOr::New(
    mitk::NodePredicateDimension::New(3), mitk::NodePredicateDimension::New(4));

  m_IsImagePredicate = mitk::NodePredicateAnd::New(
    isImage, isNotHelperObject, dimensionPredicate);
}

QmitkBasicImageProcessing::~QmitkBasicImageProcessing()
{

}

void QmitkBasicImageProcessing::CreateQtPartControl(QWidget *parent)
{
  m_Controls->setupUi(parent);

  m_Controls->selectedImageWidget->SetDataStorage(this->GetDataStorage());
  m_Controls->selectedImageWidget->SetNodePredicate(m_IsImagePredicate);
  m_Controls->selectedImageWidget->SetSelectionIsOptional(true);
  m_Controls->selectedImageWidget->SetAutoSelectNewNodes(true);
  m_Controls->selectedImageWidget->SetEmptyInfo(QString("Please select a 3D / 4D image"));
  m_Controls->selectedImageWidget->SetPopUpTitel(QString("Select an image"));

  m_Controls->selectedImageWidget_2->SetDataStorage(this->GetDataStorage());
  m_Controls->selectedImageWidget_2->SetNodePredicate(m_IsImagePredicate);
  m_Controls->selectedImageWidget_2->SetSelectionIsOptional(true);
  m_Controls->selectedImageWidget_2->SetAutoSelectNewNodes(true);
  m_Controls->selectedImageWidget_2->SetEmptyInfo(QString("Please select a 3D / 4D image"));
  m_Controls->selectedImageWidget_2->SetPopUpTitel(QString("Select an image"));

  m_Controls->gbTwoImageOps->hide();

  m_Controls->cbWhat1->clear();
  m_Controls->cbWhat1->insertItem(NOACTIONSELECTED, "Please select an operation");
  m_Controls->cbWhat1->insertItem(CATEGORY_DENOISING, "--- Denoising ---");
  m_Controls->cbWhat1->insertItem(GAUSSIAN, "Gaussian");
  m_Controls->cbWhat1->insertItem(MEDIAN, "Median");
  m_Controls->cbWhat1->insertItem(TOTALVARIATION, "Total Variation");
  m_Controls->cbWhat1->insertItem(CATEGORY_MORPHOLOGICAL, "--- Morphological ---");
  m_Controls->cbWhat1->insertItem(DILATION, "Dilation");
  m_Controls->cbWhat1->insertItem(EROSION, "Erosion");
  m_Controls->cbWhat1->insertItem(OPENING, "Opening");
  m_Controls->cbWhat1->insertItem(CLOSING, "Closing");
  m_Controls->cbWhat1->insertItem(CATEGORY_EDGE_DETECTION, "--- Edge Detection ---");
  m_Controls->cbWhat1->insertItem(GRADIENT, "Gradient");
  m_Controls->cbWhat1->insertItem(LAPLACIAN, "Laplacian (2nd Derivative)");
  m_Controls->cbWhat1->insertItem(SOBEL, "Sobel Operator");
  m_Controls->cbWhat1->insertItem(CATEGORY_MISC, "--- Misc ---");
  m_Controls->cbWhat1->insertItem(THRESHOLD, "Threshold");
  m_Controls->cbWhat1->insertItem(INVERSION, "Image Inversion");
  m_Controls->cbWhat1->insertItem(DOWNSAMPLING, "Downsampling");
  m_Controls->cbWhat1->insertItem(FLIPPING, "Flipping");
  m_Controls->cbWhat1->insertItem(RESAMPLING, "Resample to");
  m_Controls->cbWhat1->insertItem(RESCALE, "Rescale values to interval");
  m_Controls->cbWhat1->insertItem(RESCALE2, "Rescale values by scalar");

  m_Controls->cbWhat2->clear();
  m_Controls->cbWhat2->insertItem(TWOIMAGESNOACTIONSELECTED, "Please select an operation");
  m_Controls->cbWhat2->insertItem(CATEGORY_ARITHMETIC, "--- Arithmetric operations ---");
  m_Controls->cbWhat2->insertItem(ADD, "Add to Image 1:");
  m_Controls->cbWhat2->insertItem(SUBTRACT, "Subtract from Image 1:");
  m_Controls->cbWhat2->insertItem(MULTIPLY, "Multiply with Image 1:");
  m_Controls->cbWhat2->insertItem(RESAMPLE_TO, "Resample Image 1 to fit geometry:");
  m_Controls->cbWhat2->insertItem(DIVIDE, "Divide Image 1 by:");
  m_Controls->cbWhat2->insertItem(CATEGORY_BOOLEAN, "--- Boolean operations ---");
  m_Controls->cbWhat2->insertItem(AND, "AND");
  m_Controls->cbWhat2->insertItem(OR, "OR");
  m_Controls->cbWhat2->insertItem(XOR, "XOR");

  m_Controls->cbParam4->clear();
  m_Controls->cbParam4->insertItem(LINEAR, "Linear");
  m_Controls->cbParam4->insertItem(NEAREST, "Nearest neighbor");

  m_Controls->dsbParam1->hide();
  m_Controls->dsbParam2->hide();
  m_Controls->dsbParam3->hide();
  m_Controls->tlParam3->hide();
  m_Controls->tlParam4->hide();
  m_Controls->cbParam4->hide();

  this->CreateConnections();
}

void QmitkBasicImageProcessing::CreateConnections()
{
  connect(m_Controls->cbWhat1, QOverload<int>::of(&QComboBox::activated), this, &QmitkBasicImageProcessing::SelectAction);
  connect(m_Controls->btnDoIt, &QPushButton::clicked, this, &QmitkBasicImageProcessing::StartButtonClicked);
  connect(m_Controls->cbWhat2, QOverload<int>::of(&QComboBox::activated), this, &QmitkBasicImageProcessing::SelectAction2);
  connect(m_Controls->btnDoIt2, &QPushButton::clicked, this, &QmitkBasicImageProcessing::StartButton2Clicked);
  connect(m_Controls->rBOneImOp, &QRadioButton::clicked, this, &QmitkBasicImageProcessing::ChangeGUI);
  connect(m_Controls->rBTwoImOp, &QRadioButton::clicked, this, &QmitkBasicImageProcessing::ChangeGUI);
  connect(m_Controls->cbParam4, QOverload<int>::of(&QComboBox::activated), this, &QmitkBasicImageProcessing::SelectInterpolator);
  connect(m_Controls->selectedImageWidget, &QmitkAbstractNodeSelectionWidget::CurrentSelectionChanged,
    this, &QmitkBasicImageProcessing::OnCurrentSelectionChanged);
  connect(m_Controls->selectedImageWidget_2, &QmitkAbstractNodeSelectionWidget::CurrentSelectionChanged,
    this, &QmitkBasicImageProcessing::OnCurrentSelectionChanged);
}

void QmitkBasicImageProcessing::InternalGetTimeNavigationController()
{
  auto renwin_part = GetRenderWindowPart();
  if( renwin_part != nullptr )
  {
    auto tnc = renwin_part->GetTimeNavigationController();
    if( tnc != nullptr )
    {
      m_TimeStepperAdapter = new QmitkStepperAdapter(m_Controls->timeSliceNavigationWidget, tnc->GetTime());
    }
  }
}

void QmitkBasicImageProcessing::SetFocus()
{
  m_Controls->rBOneImOp->setFocus();
}

void QmitkBasicImageProcessing::OnCurrentSelectionChanged(const QList<mitk::DataNode::Pointer>& nodes)
{
  if (nodes.empty() || nodes.front().IsNull())
  {
    m_Controls->timeSliceNavigationWidget->setEnabled(false);
    m_Controls->tlTime->setEnabled(false);
    m_Controls->tlWhat1->setEnabled(false);
    m_Controls->cbWhat1->setEnabled(false);
    m_Controls->tlWhat2->setEnabled(false);
    m_Controls->cbWhat2->setEnabled(false);
    return;
  }

  auto selectedImage = dynamic_cast<mitk::Image*>(nodes.front()->GetData());
  if (nullptr == selectedImage)
  {
    return;
  }

  if (selectedImage->GetDimension() > 3)
  {
    // try to retrieve the TNC (for 4-D Processing )
    this->InternalGetTimeNavigationController();

    m_Controls->timeSliceNavigationWidget->setEnabled(true);
    m_Controls->tlTime->setEnabled(true);
  }

  m_Controls->tlWhat1->setEnabled(true);
  m_Controls->cbWhat1->setEnabled(true);
  m_Controls->tlWhat2->setEnabled(true);
  m_Controls->cbWhat2->setEnabled(true);
}

void QmitkBasicImageProcessing::ChangeGUI()
{
  if(m_Controls->rBOneImOp->isChecked())
  {
    m_Controls->gbTwoImageOps->hide();
    m_Controls->gbOneImageOps->show();
  }
  else if(m_Controls->rBTwoImOp->isChecked())
  {
    m_Controls->gbOneImageOps->hide();
    m_Controls->gbTwoImageOps->show();
  }
}

void QmitkBasicImageProcessing::ResetParameterPanel()
{
  m_Controls->tlParam->setEnabled(false);
  m_Controls->tlParam1->setEnabled(false);
  m_Controls->tlParam2->setEnabled(false);
  m_Controls->tlParam3->setEnabled(false);
  m_Controls->tlParam4->setEnabled(false);

  m_Controls->sbParam1->setEnabled(false);
  m_Controls->sbParam2->setEnabled(false);
  m_Controls->dsbParam1->setEnabled(false);
  m_Controls->dsbParam2->setEnabled(false);
  m_Controls->dsbParam3->setEnabled(false);
  m_Controls->cbParam4->setEnabled(false);
  m_Controls->sbParam1->setValue(0);
  m_Controls->sbParam2->setValue(0);
  m_Controls->dsbParam1->setValue(0);
  m_Controls->dsbParam2->setValue(0);
  m_Controls->dsbParam3->setValue(0);

  m_Controls->sbParam1->show();
  m_Controls->sbParam2->show();
  m_Controls->dsbParam1->hide();
  m_Controls->dsbParam2->hide();
  m_Controls->dsbParam3->hide();
  m_Controls->cbParam4->hide();
  m_Controls->tlParam3->hide();
  m_Controls->tlParam4->hide();
}

void QmitkBasicImageProcessing::SelectAction(int action)
{
  auto selectedImage = m_Controls->selectedImageWidget->GetSelectedNode();
  if (selectedImage.IsNull())
  {
    return;
  }

  // Prepare GUI
  this->ResetParameterPanel();
  m_Controls->btnDoIt->setEnabled(false);
  m_Controls->cbHideOrig->setEnabled(false);

  QString text1 = tr("No Parameters");
  QString text2 = text1;
  QString text3 = text1;
  QString text4 = text1;

  if (action != 19)
  {
    m_Controls->dsbParam1->hide();
    m_Controls->dsbParam2->hide();
    m_Controls->dsbParam3->hide();
    m_Controls->tlParam1->show();
    m_Controls->tlParam2->show();
    m_Controls->tlParam3->hide();
    m_Controls->tlParam4->hide();
    m_Controls->sbParam1->show();
    m_Controls->sbParam2->show();
    m_Controls->cbParam4->hide();
  }

  switch (action)
  {
  case 2:
    {
      m_SelectedAction = GAUSSIAN;
      m_Controls->tlParam1->setEnabled(true);
      m_Controls->sbParam1->hide();
      m_Controls->dsbParam1->show();
      m_Controls->dsbParam1->setEnabled(true);
      text1 = tr("&Variance:");
      m_Controls->tlParam2->hide();
      m_Controls->sbParam2->hide();

      m_Controls->dsbParam1->setMinimum( 0 );
      m_Controls->dsbParam1->setMaximum( 200 );
      m_Controls->dsbParam1->setValue( 2 );
      break;
    }
  case 3:
    {
      m_SelectedAction = MEDIAN;
      m_Controls->tlParam1->setEnabled(true);
      m_Controls->sbParam1->setEnabled(true);
      text1 = tr("&Radius:");
      m_Controls->sbParam1->setMinimum( 0 );
      m_Controls->sbParam1->setMaximum( 200 );
      m_Controls->sbParam1->setValue( 3 );
      break;
    }
  case 4:
    {
      m_SelectedAction = TOTALVARIATION;
      m_Controls->tlParam1->setEnabled(true);
      m_Controls->sbParam1->setEnabled(true);
      m_Controls->tlParam2->setEnabled(true);
      m_Controls->sbParam2->setEnabled(true);
      text1 = tr("Number Iterations:");
      text2 = tr("Regularization\n(Lambda/1000):");
      m_Controls->sbParam1->setMinimum( 1 );
      m_Controls->sbParam1->setMaximum( 1000 );
      m_Controls->sbParam1->setValue( 40 );
      m_Controls->sbParam2->setMinimum( 0 );
      m_Controls->sbParam2->setMaximum( 100000 );
      m_Controls->sbParam2->setValue( 1 );
      break;
    }
  case 6:
    {
      m_SelectedAction = DILATION;
      m_Controls->tlParam1->setEnabled(true);
      m_Controls->sbParam1->setEnabled(true);
      text1 = tr("&Radius:");
      m_Controls->sbParam1->setMinimum( 0 );
      m_Controls->sbParam1->setMaximum( 200 );
      m_Controls->sbParam1->setValue( 3 );
      break;
    }
  case 7:
    {
      m_SelectedAction = EROSION;
      m_Controls->tlParam1->setEnabled(true);
      m_Controls->sbParam1->setEnabled(true);
      text1 = tr("&Radius:");
      m_Controls->sbParam1->setMinimum( 0 );
      m_Controls->sbParam1->setMaximum( 200 );
      m_Controls->sbParam1->setValue( 3 );
      break;
    }
  case 8:
    {
      m_SelectedAction = OPENING;
      m_Controls->tlParam1->setEnabled(true);
      m_Controls->sbParam1->setEnabled(true);
      text1 = tr("&Radius:");
      m_Controls->sbParam1->setMinimum( 0 );
      m_Controls->sbParam1->setMaximum( 200 );
      m_Controls->sbParam1->setValue( 3 );
      break;
    }
  case 9:
    {
      m_SelectedAction = CLOSING;
      m_Controls->tlParam1->setEnabled(true);
      m_Controls->sbParam1->setEnabled(true);
      text1 = tr("&Radius:");
      m_Controls->sbParam1->setMinimum( 0 );
      m_Controls->sbParam1->setMaximum( 200 );
      m_Controls->sbParam1->setValue( 3 );
      break;
    }
  case 11:
    {
      m_SelectedAction = GRADIENT;
      m_Controls->tlParam1->setEnabled(true);
      m_Controls->sbParam1->hide();
      m_Controls->dsbParam1->show();
      m_Controls->dsbParam1->setEnabled(true);
      text1 = tr("Sigma of Gaussian Kernel:\n(in Image Spacing Units)");
      m_Controls->tlParam2->hide();
      m_Controls->sbParam2->hide();

      m_Controls->dsbParam1->setMinimum( 0 );
      m_Controls->dsbParam1->setMaximum( 200 );
      m_Controls->dsbParam1->setValue( 2 );
      break;
    }
  case 12:
    {
      m_SelectedAction = LAPLACIAN;
      break;
    }
  case 13:
    {
      m_SelectedAction = SOBEL;
      break;
    }
  case 15:
    {
      m_SelectedAction = THRESHOLD;
      m_Controls->tlParam1->setEnabled(true);
      m_Controls->sbParam1->setEnabled(true);
      m_Controls->tlParam2->setEnabled(true);
      m_Controls->sbParam2->setEnabled(true);
      text1 = tr("Lower threshold:");
      text2 = tr("Upper threshold:");
      m_Controls->sbParam1->setMinimum( -100000 );
      m_Controls->sbParam1->setMaximum( 100000 );
      m_Controls->sbParam1->setValue( 0 );
      m_Controls->sbParam2->setMinimum( -100000 );
      m_Controls->sbParam2->setMaximum( 100000 );
      m_Controls->sbParam2->setValue( 300 );
      break;
    }
  case 16:
    {
      m_SelectedAction = INVERSION;
      break;
    }
  case 17:
    {
      m_SelectedAction = DOWNSAMPLING;
      m_Controls->tlParam1->setEnabled(true);
      m_Controls->sbParam1->setEnabled(true);
      text1 = tr("Downsampling by Factor:");
      m_Controls->sbParam1->setMinimum( 1 );
      m_Controls->sbParam1->setMaximum( 100 );
      m_Controls->sbParam1->setValue( 2 );
      break;
    }
  case 18:
    {
      m_SelectedAction = FLIPPING;
      m_Controls->tlParam1->setEnabled(true);
      m_Controls->sbParam1->setEnabled(true);
      text1 = tr("Flip across axis:");
      m_Controls->sbParam1->setMinimum( 0 );
      m_Controls->sbParam1->setMaximum( 2 );
      m_Controls->sbParam1->setValue( 1 );
      break;
    }
  case 19:
    {
      m_SelectedAction = RESAMPLING;
      m_Controls->tlParam1->setEnabled(true);
      m_Controls->sbParam1->setEnabled(false);
      m_Controls->sbParam1->hide();
      m_Controls->dsbParam1->show();
      m_Controls->dsbParam1->setEnabled(true);
      m_Controls->tlParam2->setEnabled(true);
      m_Controls->sbParam2->setEnabled(false);
      m_Controls->sbParam2->hide();
      m_Controls->dsbParam2->show();
      m_Controls->dsbParam2->setEnabled(true);
      m_Controls->tlParam3->show();
      m_Controls->tlParam3->setEnabled(true);
      m_Controls->dsbParam3->show();
      m_Controls->dsbParam3->setEnabled(true);
      m_Controls->tlParam4->show();
      m_Controls->tlParam4->setEnabled(true);
      m_Controls->cbParam4->show();
      m_Controls->cbParam4->setEnabled(true);

      m_Controls->dsbParam1->setMinimum(0.01);
      m_Controls->dsbParam1->setMaximum(10.0);
      m_Controls->dsbParam1->setSingleStep(0.1);
      m_Controls->dsbParam1->setValue(0.3);
      m_Controls->dsbParam2->setMinimum(0.01);
      m_Controls->dsbParam2->setMaximum(10.0);
      m_Controls->dsbParam2->setSingleStep(0.1);
      m_Controls->dsbParam2->setValue(0.3);
      m_Controls->dsbParam3->setMinimum(0.01);
      m_Controls->dsbParam3->setMaximum(10.0);
      m_Controls->dsbParam3->setSingleStep(0.1);
      m_Controls->dsbParam3->setValue(1.5);

      text1 = tr("x-spacing:");
      text2 = tr("y-spacing:");
      text3 = tr("z-spacing:");
      text4 = tr("Interplation:");
      break;
    }
  case 20:
    {
      m_SelectedAction = RESCALE;
      m_Controls->dsbParam1->show();
      m_Controls->tlParam1->show();
      m_Controls->dsbParam1->setEnabled(true);
      m_Controls->tlParam1->setEnabled(true);
      m_Controls->dsbParam2->show();
      m_Controls->tlParam2->show();
      m_Controls->dsbParam2->setEnabled(true);
      m_Controls->tlParam2->setEnabled(true);
      text1 = tr("Output minimum:");
      text2 = tr("Output maximum:");
      break;
    }
  case 21:
  {
      m_SelectedAction = RESCALE2;
      m_Controls->dsbParam1->show();
      m_Controls->tlParam1->show();
      m_Controls->dsbParam1->setEnabled(true);
      m_Controls->tlParam1->setEnabled(true);
      text1 = tr("Scaling value:");
      break;
  }
  default:
    return;
  }

  m_Controls->tlParam->setEnabled(true);
  m_Controls->tlParam1->setText(text1);
  m_Controls->tlParam2->setText(text2);
  m_Controls->tlParam3->setText(text3);
  m_Controls->tlParam4->setText(text4);

  m_Controls->btnDoIt->setEnabled(true);
  m_Controls->cbHideOrig->setEnabled(true);
}

void QmitkBasicImageProcessing::StartButtonClicked()
{
  auto selectedNode = m_Controls->selectedImageWidget->GetSelectedNode();
  if (selectedNode.IsNull())
  {
    return;
  }

  this->BusyCursorOn();

  mitk::Image::Pointer newImage;
  try
  {
    newImage = dynamic_cast<mitk::Image*>(selectedNode->GetData());
  }
  catch ( std::exception &e )
  {
  QString exceptionString = tr("An error occured during image loading:\n");
  exceptionString.append( e.what() );
    QMessageBox::warning( nullptr, "Basic Image Processing", exceptionString , QMessageBox::Ok, QMessageBox::NoButton );
    this->BusyCursorOff();
    return;
  }

  // check if input image is valid, casting does not throw exception when casting from 'nullptr-Object'
  if ( (! newImage) || (newImage->IsInitialized() == false) )
  {
    this->BusyCursorOff();

    QMessageBox::warning( nullptr, "Basic Image Processing", tr("Input image is broken or not initialized. Returning."), QMessageBox::Ok, QMessageBox::NoButton );
    return;
  }

  // check if operation is done on 4D a image time step
  if(newImage->GetDimension() > 3)
  {
    auto timeSelector = mitk::ImageTimeSelector::New();
    timeSelector->SetInput(newImage);
    timeSelector->SetTimeNr(m_Controls->timeSliceNavigationWidget->GetPos() );
    timeSelector->Update();
    newImage = timeSelector->GetOutput();
  }

  // check if image or vector image
  auto itkImage = ImageType::New();
  auto itkVecImage = VectorImageType::New();

  int isVectorImage = newImage->GetPixelType().GetNumberOfComponents();

  if(isVectorImage > 1)
  {
    CastToItkImage( newImage, itkVecImage );
  }
  else
  {
    CastToItkImage( newImage, itkImage );
  }

  std::stringstream nameAddition("");

  int param1 = m_Controls->sbParam1->value();
  int param2 = m_Controls->sbParam2->value();
  double dparam1 = m_Controls->dsbParam1->value();
  double dparam2 = m_Controls->dsbParam2->value();
  double dparam3 = m_Controls->dsbParam3->value();

  try
  {
  switch (m_SelectedAction)
  {
  case GAUSSIAN:
    {
      GaussianFilterType::Pointer gaussianFilter = GaussianFilterType::New();
      gaussianFilter->SetInput( itkImage );
      gaussianFilter->SetVariance( dparam1 );
      gaussianFilter->UpdateLargestPossibleRegion();
      newImage = mitk::ImportItkImage(gaussianFilter->GetOutput())->Clone();
      nameAddition << "_Gaussian_var_" << dparam1;
      std::cout << "Gaussian filtering successful." << std::endl;
      break;
    }
  case MEDIAN:
    {
      MedianFilterType::Pointer medianFilter = MedianFilterType::New();
      MedianFilterType::InputSizeType size;
      size.Fill(param1);
      medianFilter->SetRadius( size );
      medianFilter->SetInput(itkImage);
      medianFilter->UpdateLargestPossibleRegion();
      newImage = mitk::ImportItkImage(medianFilter->GetOutput())->Clone();
      nameAddition << "_Median_radius_" << param1;
      std::cout << "Median Filtering successful." << std::endl;
      break;
    }
  case TOTALVARIATION:
    {
      if(isVectorImage > 1)
      {
        VectorTotalVariationFilterType::Pointer TVFilter
          = VectorTotalVariationFilterType::New();
        TVFilter->SetInput( itkVecImage.GetPointer() );
        TVFilter->SetNumberIterations(param1);
        TVFilter->SetLambda(double(param2)/1000.);
        TVFilter->UpdateLargestPossibleRegion();

        newImage = mitk::ImportItkImage(TVFilter->GetOutput())->Clone();
      }
      else
      {
        ImagePTypeToFloatPTypeCasterType::Pointer floatCaster = ImagePTypeToFloatPTypeCasterType::New();
        floatCaster->SetInput( itkImage );
        floatCaster->Update();
        DoubleImageType::Pointer fImage = floatCaster->GetOutput();

        TotalVariationFilterType::Pointer TVFilter
          = TotalVariationFilterType::New();
        TVFilter->SetInput( fImage.GetPointer() );
        TVFilter->SetNumberIterations(param1);
        TVFilter->SetLambda(double(param2)/1000.);
        TVFilter->UpdateLargestPossibleRegion();

        newImage = mitk::ImportItkImage(TVFilter->GetOutput())->Clone();
      }

      nameAddition << "_TV_Iter_" << param1 << "_L_" << param2;
      std::cout << "Total Variation Filtering successful." << std::endl;
      break;
    }
  case DILATION:
    {
      BallType binaryBall;
      binaryBall.SetRadius( param1 );
      binaryBall.CreateStructuringElement();

      DilationFilterType::Pointer dilationFilter = DilationFilterType::New();
      dilationFilter->SetInput( itkImage );
      dilationFilter->SetKernel( binaryBall );
      dilationFilter->UpdateLargestPossibleRegion();
      newImage = mitk::ImportItkImage(dilationFilter->GetOutput())->Clone();
      nameAddition << "_Dilated_by_" << param1;
      std::cout << "Dilation successful." << std::endl;
      break;
    }
  case EROSION:
    {
      BallType binaryBall;
      binaryBall.SetRadius( param1 );
      binaryBall.CreateStructuringElement();

      ErosionFilterType::Pointer erosionFilter = ErosionFilterType::New();
      erosionFilter->SetInput( itkImage );
      erosionFilter->SetKernel( binaryBall );
      erosionFilter->UpdateLargestPossibleRegion();
      newImage = mitk::ImportItkImage(erosionFilter->GetOutput())->Clone();
      nameAddition << "_Eroded_by_" << param1;
      std::cout << "Erosion successful." << std::endl;
      break;
    }
  case OPENING:
    {
      BallType binaryBall;
      binaryBall.SetRadius( param1 );
      binaryBall.CreateStructuringElement();

      OpeningFilterType::Pointer openFilter = OpeningFilterType::New();
      openFilter->SetInput( itkImage );
      openFilter->SetKernel( binaryBall );
      openFilter->UpdateLargestPossibleRegion();
      newImage = mitk::ImportItkImage(openFilter->GetOutput())->Clone();
      nameAddition << "_Opened_by_" << param1;
      std::cout << "Opening successful." << std::endl;
      break;
    }
  case CLOSING:
    {
      BallType binaryBall;
      binaryBall.SetRadius( param1 );
      binaryBall.CreateStructuringElement();

      ClosingFilterType::Pointer closeFilter = ClosingFilterType::New();
      closeFilter->SetInput( itkImage );
      closeFilter->SetKernel( binaryBall );
      closeFilter->UpdateLargestPossibleRegion();
      newImage = mitk::ImportItkImage(closeFilter->GetOutput())->Clone();
      nameAddition << "_Closed_by_" << param1;
      std::cout << "Closing successful." << std::endl;
      break;
    }
  case GRADIENT:
    {
      GradientFilterType::Pointer gradientFilter = GradientFilterType::New();
      gradientFilter->SetInput( itkImage );
      gradientFilter->SetSigma( dparam1 );
      gradientFilter->UpdateLargestPossibleRegion();
      newImage = mitk::ImportItkImage(gradientFilter->GetOutput())->Clone();
      nameAddition << "_Gradient_sigma_" << dparam1;
      std::cout << "Gradient calculation successful." << std::endl;
      break;
    }
  case LAPLACIAN:
    {
      // the laplace filter requires a float type image as input, we need to cast the itkImage
      // to correct type
      ImagePTypeToFloatPTypeCasterType::Pointer caster = ImagePTypeToFloatPTypeCasterType::New();
      caster->SetInput( itkImage );
      caster->Update();
      DoubleImageType::Pointer fImage = caster->GetOutput();

      LaplacianFilterType::Pointer laplacianFilter = LaplacianFilterType::New();
      laplacianFilter->SetInput( fImage );
      laplacianFilter->UpdateLargestPossibleRegion();
      newImage = mitk::ImportItkImage(laplacianFilter->GetOutput())->Clone();
      nameAddition << "_Second_Derivative";
      std::cout << "Laplacian filtering successful." << std::endl;
      break;
    }
  case SOBEL:
    {
      // the sobel filter requires a float type image as input, we need to cast the itkImage
      // to correct type
      ImagePTypeToFloatPTypeCasterType::Pointer caster = ImagePTypeToFloatPTypeCasterType::New();
      caster->SetInput( itkImage );
      caster->Update();
      DoubleImageType::Pointer fImage = caster->GetOutput();

      SobelFilterType::Pointer sobelFilter = SobelFilterType::New();
      sobelFilter->SetInput( fImage );
      sobelFilter->UpdateLargestPossibleRegion();
      newImage = mitk::ImportItkImage(sobelFilter->GetOutput())->Clone();
      nameAddition << "_Sobel";
      std::cout << "Edge Detection successful." << std::endl;
      break;
    }
  case THRESHOLD:
    {
      ThresholdFilterType::Pointer thFilter = ThresholdFilterType::New();
      thFilter->SetLowerThreshold(param1 < param2 ? param1 : param2);
      thFilter->SetUpperThreshold(param2 > param1 ? param2 : param1);
      thFilter->SetInsideValue(1);
      thFilter->SetOutsideValue(0);
      thFilter->SetInput(itkImage);
      thFilter->UpdateLargestPossibleRegion();
      newImage = mitk::ImportItkImage(thFilter->GetOutput())->Clone();
      nameAddition << "_Threshold";
      std::cout << "Thresholding successful." << std::endl;
      break;
    }
  case INVERSION:
    {
      InversionFilterType::Pointer invFilter = InversionFilterType::New();
      mitk::ScalarType min = newImage->GetStatistics()->GetScalarValueMin();
      mitk::ScalarType max = newImage->GetStatistics()->GetScalarValueMax();
      invFilter->SetMaximum( max + min );
      invFilter->SetInput(itkImage);
      invFilter->UpdateLargestPossibleRegion();
      newImage = mitk::ImportItkImage(invFilter->GetOutput())->Clone();
      nameAddition << "_Inverted";
      std::cout << "Image inversion successful." << std::endl;
      break;
    }
  case DOWNSAMPLING:
    {
      ResampleImageFilterType::Pointer downsampler = ResampleImageFilterType::New();
      downsampler->SetInput( itkImage );

      NearestInterpolatorType::Pointer interpolator = NearestInterpolatorType::New();
      downsampler->SetInterpolator( interpolator );

      downsampler->SetDefaultPixelValue( 0 );

      ResampleImageFilterType::SpacingType spacing = itkImage->GetSpacing();
      spacing *= (double) param1;
      downsampler->SetOutputSpacing( spacing );

      downsampler->SetOutputOrigin( itkImage->GetOrigin() );
      downsampler->SetOutputDirection( itkImage->GetDirection() );

      ResampleImageFilterType::SizeType size = itkImage->GetLargestPossibleRegion().GetSize();
      for ( int i = 0; i < 3; ++i )
      {
        size[i] /= param1;
      }
      downsampler->SetSize( size );
      downsampler->UpdateLargestPossibleRegion();

      newImage = mitk::ImportItkImage(downsampler->GetOutput())->Clone();
      nameAddition << "_Downsampled_by_" << param1;
      std::cout << "Downsampling successful." << std::endl;
      break;
    }
  case FLIPPING:
    {
      FlipImageFilterType::Pointer flipper = FlipImageFilterType::New();
      flipper->SetInput( itkImage );
      itk::FixedArray<bool, 3> flipAxes;
      for(int i=0; i<3; ++i)
      {
        if(i == param1)
        {
          flipAxes[i] = true;
        }
        else
        {
          flipAxes[i] = false;
        }
      }
      flipper->SetFlipAxes(flipAxes);
      flipper->UpdateLargestPossibleRegion();
      newImage = mitk::ImportItkImage(flipper->GetOutput())->Clone();
      std::cout << "Image flipping successful." << std::endl;
      break;
    }
  case RESAMPLING:
    {
      std::string selectedInterpolator;
      ResampleImageFilterType::Pointer resampler = ResampleImageFilterType::New();
      switch (m_SelectedInterpolation)
      {
      case LINEAR:
        {
          LinearInterpolatorType::Pointer interpolator = LinearInterpolatorType::New();
          resampler->SetInterpolator(interpolator);
          selectedInterpolator = "Linear";
          break;
        }
      case NEAREST:
        {
          NearestInterpolatorType::Pointer interpolator = NearestInterpolatorType::New();
          resampler->SetInterpolator(interpolator);
          selectedInterpolator = "Nearest";
          break;
        }
      default:
        {
          LinearInterpolatorType::Pointer interpolator = LinearInterpolatorType::New();
          resampler->SetInterpolator(interpolator);
          selectedInterpolator = "Linear";
          break;
        }
      }
      resampler->SetInput( itkImage );
      resampler->SetOutputOrigin( itkImage->GetOrigin() );

      ImageType::SizeType input_size = itkImage->GetLargestPossibleRegion().GetSize();
      ImageType::SpacingType input_spacing = itkImage->GetSpacing();

      ImageType::SizeType output_size;
      ImageType::SpacingType output_spacing;

      output_size[0] = input_size[0] * (input_spacing[0] / dparam1);
      output_size[1] = input_size[1] * (input_spacing[1] / dparam2);
      output_size[2] = input_size[2] * (input_spacing[2] / dparam3);
      output_spacing [0] = dparam1;
      output_spacing [1] = dparam2;
      output_spacing [2] = dparam3;

      resampler->SetSize( output_size );
      resampler->SetOutputSpacing( output_spacing );
      resampler->SetOutputDirection( itkImage->GetDirection() );

      resampler->UpdateLargestPossibleRegion();

      ImageType::Pointer resampledImage = resampler->GetOutput();

      newImage = mitk::ImportItkImage( resampledImage )->Clone();
      nameAddition << "_Resampled_" << selectedInterpolator;
      std::cout << "Resampling successful." << std::endl;
      break;
    }
  case RESCALE:
    {
      DoubleImageType::Pointer floatImage = DoubleImageType::New();
      CastToItkImage( newImage, floatImage );
      itk::RescaleIntensityImageFilter<DoubleImageType,DoubleImageType>::Pointer filter = itk::RescaleIntensityImageFilter<DoubleImageType,DoubleImageType>::New();
      filter->SetInput(0, floatImage);
      filter->SetOutputMinimum(dparam1);
      filter->SetOutputMaximum(dparam2);
      filter->Update();
      floatImage = filter->GetOutput();

      newImage = mitk::Image::New();
      newImage->InitializeByItk(floatImage.GetPointer());
      newImage->SetVolume(floatImage->GetBufferPointer());
      nameAddition << "_Rescaled";
      std::cout << "Rescaling successful." << std::endl;

      break;
    }
  case RESCALE2:
  {
      DoubleImageType::Pointer floatImage = DoubleImageType::New();
      CastToItkImage( newImage, floatImage );
      itk::ShiftScaleImageFilter<DoubleImageType,DoubleImageType>::Pointer filter = itk::ShiftScaleImageFilter<DoubleImageType,DoubleImageType>::New();
      filter->SetInput(0, floatImage);
      filter->SetScale(dparam1);

      filter->Update();
      floatImage = filter->GetOutput();

      newImage = mitk::Image::New();
      newImage->InitializeByItk(floatImage.GetPointer());
      newImage->SetVolume(floatImage->GetBufferPointer());
      nameAddition << "_Rescaled";
      std::cout << "Rescaling successful." << std::endl;
      break;
  }

  default:
    this->BusyCursorOff();
    return;
  }
  }
  catch (...)
  {
    this->BusyCursorOff();
    QMessageBox::warning(nullptr, "Warning", "Problem when applying filter operation. Check your input...");
    return;
  }

  newImage->DisconnectPipeline();

  // adjust level/window to new image
  mitk::LevelWindow levelwindow;
  levelwindow.SetAuto( newImage );
  auto levWinProp = mitk::LevelWindowProperty::New();
  levWinProp->SetLevelWindow( levelwindow );

  // compose new image name
  std::string name = selectedNode->GetName();
  if (name.find(".nrrd") == name.size() -5 )
  {
    name = name.substr(0,name.size() -5);
  }
  name.append( nameAddition.str() );

  // create final result MITK data storage node
  auto result = mitk::DataNode::New();
  result->SetProperty( "levelwindow", levWinProp );
  result->SetProperty( "name", mitk::StringProperty::New( name.c_str() ) );
  result->SetData( newImage );

  // for vector images, a different mapper is needed
  if(isVectorImage > 1)
  {
    auto mapper = mitk::VectorImageMapper2D::New();
    result->SetMapper(1,mapper);
  }

  // add new image to data storage and set as active to ease further processing
  GetDataStorage()->Add(result, selectedNode);
  if (m_Controls->cbHideOrig->isChecked() == true)
  {
    selectedNode->SetProperty("visible", mitk::BoolProperty::New(false));
  }

  // show the results
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  this->BusyCursorOff();
}

void QmitkBasicImageProcessing::SelectAction2(int operation)
{
  switch (operation)
  {
  case 2:
    m_SelectedOperation = ADD;
    break;
  case 3:
    m_SelectedOperation = SUBTRACT;
    break;
  case 4:
    m_SelectedOperation = MULTIPLY;
    break;
  case 5:
    m_SelectedOperation = DIVIDE;
    break;
  case 6:
    m_SelectedOperation = RESAMPLE_TO;
    break;
  case 8:
    m_SelectedOperation = AND;
    break;
  case 9:
    m_SelectedOperation = OR;
    break;
  case 10:
    m_SelectedOperation = XOR;
    break;
  default:
    return;
  }

  m_Controls->selectedImageLabel_2->setEnabled(true);
  m_Controls->selectedImageWidget_2->setEnabled(true);
  m_Controls->btnDoIt2->setEnabled(true);
}

void QmitkBasicImageProcessing::StartButton2Clicked()
{
  auto selectedNode = m_Controls->selectedImageWidget->GetSelectedNode();
  if (selectedNode.IsNull())
  {
    return;
  }

  auto selectedNode2 = m_Controls->selectedImageWidget_2->GetSelectedNode();
  if (selectedNode2.IsNull())
  {
    return;
  }

  mitk::Image::Pointer newImage1 = dynamic_cast<mitk::Image*>(selectedNode->GetData());
  mitk::Image::Pointer newImage2 = dynamic_cast<mitk::Image*>(selectedNode2->GetData());

  // check if images are valid
  if(newImage1.IsNull() || newImage2.IsNull() || false == newImage1->IsInitialized() || false == newImage2->IsInitialized())
  {
    itkGenericExceptionMacro(<< "At least one of the input images is broken or not initialized.");
    return;
  }

  this->BusyCursorOn();

  // check if 4D image and use filter on correct time step
  if(newImage1->GetDimension() > 3)
  {
    auto timeSelector = mitk::ImageTimeSelector::New();

    auto sn_widget = static_cast<QmitkSliceNavigationWidget*>(m_Controls->timeSliceNavigationWidget);
    int time = 0;

    if( sn_widget != nullptr )
        time = sn_widget->GetPos();

    timeSelector->SetInput(newImage1);
    timeSelector->SetTimeNr( time );
    timeSelector->UpdateLargestPossibleRegion();
    newImage1 = timeSelector->GetOutput();
    newImage1->DisconnectPipeline();

    timeSelector->SetInput(newImage2);
    timeSelector->SetTimeNr( time );
    timeSelector->UpdateLargestPossibleRegion();
    newImage2 = timeSelector->GetOutput();
    newImage2->DisconnectPipeline();
  }

  auto itkImage1 = ImageType::New();
  auto itkImage2 = ImageType::New();

  CastToItkImage( newImage1, itkImage1 );
  CastToItkImage( newImage2, itkImage2 );

  std::string nameAddition = "";
  try
  {
  switch (m_SelectedOperation)
  {
  case ADD:
    {
      AddFilterType::Pointer addFilter = AddFilterType::New();
      addFilter->SetInput1( itkImage1 );
      addFilter->SetInput2( itkImage2 );
      addFilter->UpdateLargestPossibleRegion();
      newImage1 = mitk::ImportItkImage(addFilter->GetOutput())->Clone();
      nameAddition = "_Added";
    }
    break;
  case SUBTRACT:
    {
      SubtractFilterType::Pointer subFilter = SubtractFilterType::New();
      subFilter->SetInput1( itkImage1 );
      subFilter->SetInput2( itkImage2 );
      subFilter->UpdateLargestPossibleRegion();
      newImage1 = mitk::ImportItkImage(subFilter->GetOutput())->Clone();
      nameAddition = "_Subtracted";
    }
    break;
  case MULTIPLY:
    {
      MultiplyFilterType::Pointer multFilter = MultiplyFilterType::New();
      multFilter->SetInput1( itkImage1 );
      multFilter->SetInput2( itkImage2 );
      multFilter->UpdateLargestPossibleRegion();
      newImage1 = mitk::ImportItkImage(multFilter->GetOutput())->Clone();
      nameAddition = "_Multiplied";
    }
    break;
  case DIVIDE:
    {
      DivideFilterType::Pointer divFilter = DivideFilterType::New();
      divFilter->SetInput1( itkImage1 );
      divFilter->SetInput2( itkImage2 );
      divFilter->UpdateLargestPossibleRegion();
      newImage1 = mitk::ImportItkImage<DoubleImageType>(divFilter->GetOutput())->Clone();
      nameAddition = "_Divided";
    }
    break;
  case AND:
    {
      AndImageFilterType::Pointer andFilter = AndImageFilterType::New();
      andFilter->SetInput1( itkImage1 );
      andFilter->SetInput2( itkImage2 );
      andFilter->UpdateLargestPossibleRegion();
      newImage1 = mitk::ImportItkImage(andFilter->GetOutput())->Clone();
      nameAddition = "_AND";
      break;
    }
  case OR:
    {
      OrImageFilterType::Pointer orFilter = OrImageFilterType::New();
      orFilter->SetInput1( itkImage1 );
      orFilter->SetInput2( itkImage2 );
      orFilter->UpdateLargestPossibleRegion();
      newImage1 = mitk::ImportItkImage(orFilter->GetOutput())->Clone();
      nameAddition = "_OR";
      break;
    }
  case XOR:
    {
      XorImageFilterType::Pointer xorFilter = XorImageFilterType::New();
      xorFilter->SetInput1( itkImage1 );
      xorFilter->SetInput2( itkImage2 );
      xorFilter->UpdateLargestPossibleRegion();
      newImage1 = mitk::ImportItkImage(xorFilter->GetOutput())->Clone();
      nameAddition = "_XOR";
      break;
    }
  case RESAMPLE_TO:
    {
      itk::BSplineInterpolateImageFunction<DoubleImageType, double>::Pointer bspl_interpolator
        = itk::BSplineInterpolateImageFunction<DoubleImageType, double>::New();
      bspl_interpolator->SetSplineOrder( 3 );

      itk::NearestNeighborInterpolateImageFunction< DoubleImageType >::Pointer nn_interpolator
          = itk::NearestNeighborInterpolateImageFunction< DoubleImageType>::New();

      DoubleImageType::Pointer itkImage1 = DoubleImageType::New();
      DoubleImageType::Pointer itkImage2 = DoubleImageType::New();

      CastToItkImage( newImage1, itkImage1 );
      CastToItkImage( newImage2, itkImage2 );

      itk::ResampleImageFilter< DoubleImageType, DoubleImageType >::Pointer resampleFilter = itk::ResampleImageFilter< DoubleImageType, DoubleImageType >::New();
      resampleFilter->SetInput( itkImage1 );
      resampleFilter->SetReferenceImage( itkImage2 );
      resampleFilter->SetUseReferenceImage( true );

      // use NN interp with binary images
      if(selectedNode->GetProperty("binary") )
        resampleFilter->SetInterpolator( nn_interpolator );
      else
        resampleFilter->SetInterpolator( bspl_interpolator );

      resampleFilter->SetDefaultPixelValue( 0 );

      try
      {
        resampleFilter->UpdateLargestPossibleRegion();
      }
      catch( const itk::ExceptionObject &e)
      {
        MITK_WARN << "Updating resampling filter failed. ";
        MITK_WARN << "REASON: " << e.what();
      }

      DoubleImageType::Pointer resampledImage = resampleFilter->GetOutput();

      newImage1 = mitk::ImportItkImage( resampledImage )->Clone();
      nameAddition = "_Resampled";
      break;
    }

  default:
    std::cout << "Something went wrong..." << std::endl;
    this->BusyCursorOff();
    return;
  }
  }
  catch (const itk::ExceptionObject& e )
  {
    this->BusyCursorOff();
    QMessageBox::warning(nullptr, "ITK Exception", e.what() );
    QMessageBox::warning(nullptr, "Warning", tr("Problem when applying arithmetic operation to two images. Check dimensions of input images."));
    return;
  }

  // disconnect pipeline; images will not be reused
  newImage1->DisconnectPipeline();
  itkImage1 = nullptr;
  itkImage2 = nullptr;

  // adjust level/window to new image and compose new image name
  mitk::LevelWindow levelwindow;
  levelwindow.SetAuto( newImage1 );
  auto levWinProp = mitk::LevelWindowProperty::New();
  levWinProp->SetLevelWindow( levelwindow );
  std::string name = selectedNode->GetName();
  if (name.find(".nrrd") == name.size() -5 )
  {
    name = name.substr(0,name.size() -5);
  }

  // create final result MITK data storage node
  auto result = mitk::DataNode::New();
  result->SetProperty( "levelwindow", levWinProp );
  result->SetProperty( "name", mitk::StringProperty::New( (name + nameAddition ).c_str() ));
  result->SetData( newImage1 );
  this->GetDataStorage()->Add(result, selectedNode);

  if (m_Controls->cbHideOrig->isChecked() == true)
  {
    selectedNode->SetProperty("visible", mitk::BoolProperty::New(false));
    selectedNode2->SetProperty("visible", mitk::BoolProperty::New(false));
  }

  // show the newly created image
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  this->BusyCursorOff();
}

void QmitkBasicImageProcessing::SelectInterpolator(int interpolator)
{
  switch (interpolator)
  {
  case 0:
    {
      m_SelectedInterpolation = LINEAR;
      break;
    }
  case 1:
    {
      m_SelectedInterpolation = NEAREST;
      break;
    }
  }
}
