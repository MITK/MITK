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

#include "QmitkBasicImageProcessingView.h"

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
#include "QmitkStdMultiWidget.h"
#include "QmitkDataNodeSelectionProvider.h"
#include "mitkDataNodeObject.h"

// MITK includes (general)
#include "mitkNodePredicateDataType.h"
#include "mitkNodePredicateDimension.h"
#include "mitkNodePredicateAnd.h"
#include "mitkImageTimeSelector.h"
#include "mitkVectorImageMapper2D.h"
#include "mitkProperties.h"

// Includes for image casting between ITK and MITK
#include "mitkImageCast.h"
#include "mitkITKImageImport.h"

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


// Convenient Definitions
typedef itk::Image<short, 3>                                                            ImageType;
typedef itk::Image<unsigned char, 3>                                                    SegmentationImageType;
typedef itk::Image<double, 3>                                                           FloatImageType;
typedef itk::Image<itk::Vector<float,3>, 3>                                             VectorImageType;

typedef itk::BinaryBallStructuringElement<ImageType::PixelType, 3>                      BallType;
typedef itk::GrayscaleDilateImageFilter<ImageType, ImageType, BallType>                 DilationFilterType;
typedef itk::GrayscaleErodeImageFilter<ImageType, ImageType, BallType>                  ErosionFilterType;
typedef itk::GrayscaleMorphologicalOpeningImageFilter<ImageType, ImageType, BallType>   OpeningFilterType;
typedef itk::GrayscaleMorphologicalClosingImageFilter<ImageType, ImageType, BallType>   ClosingFilterType;

typedef itk::MedianImageFilter< ImageType, ImageType >                                  MedianFilterType;
typedef itk::DiscreteGaussianImageFilter< ImageType, ImageType>                         GaussianFilterType;
typedef itk::TotalVariationDenoisingImageFilter<FloatImageType, FloatImageType>         TotalVariationFilterType;
typedef itk::TotalVariationDenoisingImageFilter<VectorImageType, VectorImageType>       VectorTotalVariationFilterType;

typedef itk::BinaryThresholdImageFilter< ImageType, ImageType >                         ThresholdFilterType;
typedef itk::InvertIntensityImageFilter< ImageType, ImageType >                         InversionFilterType;

typedef itk::GradientMagnitudeRecursiveGaussianImageFilter< ImageType, ImageType >      GradientFilterType;
typedef itk::LaplacianImageFilter< FloatImageType, FloatImageType >                     LaplacianFilterType;
typedef itk::SobelEdgeDetectionImageFilter< FloatImageType, FloatImageType >            SobelFilterType;

typedef itk::ResampleImageFilter< ImageType, ImageType >                                ResampleImageFilterType;
typedef itk::ResampleImageFilter< ImageType, ImageType >                                ResampleImageFilterType2;
typedef itk::CastImageFilter< ImageType, FloatImageType >                               ImagePTypeToFloatPTypeCasterType;

typedef itk::AddImageFilter< ImageType, ImageType, ImageType >                          AddFilterType;
typedef itk::SubtractImageFilter< ImageType, ImageType, ImageType >                     SubtractFilterType;
typedef itk::MultiplyImageFilter< ImageType, ImageType, ImageType >                     MultiplyFilterType;
typedef itk::DivideImageFilter< ImageType, ImageType, FloatImageType >                  DivideFilterType;

typedef itk::OrImageFilter< ImageType, ImageType >                                      OrImageFilterType;
typedef itk::AndImageFilter< ImageType, ImageType >                                     AndImageFilterType;
typedef itk::XorImageFilter< ImageType, ImageType >                                     XorImageFilterType;

typedef itk::FlipImageFilter< ImageType >                                               FlipImageFilterType;

typedef itk::LinearInterpolateImageFunction< ImageType, double >                        LinearInterpolatorType;
typedef itk::NearestNeighborInterpolateImageFunction< ImageType, double >               NearestInterpolatorType;


QmitkBasicImageProcessing::QmitkBasicImageProcessing()
: QmitkFunctionality(),
  m_Controls(NULL),
  m_SelectedImageNode(NULL),
  m_TimeStepperAdapter(NULL),
  m_SelectionListener(NULL)
{
}

QmitkBasicImageProcessing::~QmitkBasicImageProcessing()
{
  //berry::ISelectionService* s = GetSite()->GetWorkbenchWindow()->GetSelectionService();
  //if(s)
  //  s->RemoveSelectionListener(m_SelectionListener);
}

void QmitkBasicImageProcessing::CreateQtPartControl(QWidget *parent)
{
  if (m_Controls == NULL)
  {
    m_Controls = new Ui::QmitkBasicImageProcessingViewControls;
    m_Controls->setupUi(parent);
    this->CreateConnections();


    //setup predictaes for combobox

    mitk::NodePredicateDimension::Pointer dimensionPredicate = mitk::NodePredicateDimension::New(3);
    mitk::NodePredicateDataType::Pointer imagePredicate = mitk::NodePredicateDataType::New("Image");
    m_Controls->m_ImageSelector2->SetDataStorage(this->GetDefaultDataStorage());
    m_Controls->m_ImageSelector2->SetPredicate(mitk::NodePredicateAnd::New(dimensionPredicate, imagePredicate));
  }
  m_Controls->gbTwoImageOps->hide();

  m_SelectedImageNode = mitk::DataStorageSelection::New(this->GetDefaultDataStorage(), false);
}

void QmitkBasicImageProcessing::CreateConnections()
{
  if ( m_Controls )
  {
    connect( (QObject*)(m_Controls->cbWhat1), SIGNAL( activated(int) ), this, SLOT( SelectAction(int) ) );
    connect( (QObject*)(m_Controls->btnDoIt), SIGNAL(clicked()),(QObject*) this, SLOT(StartButtonClicked()));

    connect( (QObject*)(m_Controls->cbWhat2), SIGNAL( activated(int) ), this, SLOT( SelectAction2(int) ) );
    connect( (QObject*)(m_Controls->btnDoIt2), SIGNAL(clicked()),(QObject*) this, SLOT(StartButton2Clicked()));

    connect( (QObject*)(m_Controls->rBOneImOp), SIGNAL( clicked() ), this, SLOT( ChangeGUI() ) );
    connect( (QObject*)(m_Controls->rBTwoImOp), SIGNAL( clicked() ), this, SLOT( ChangeGUI() ) );

    connect( (QObject*)(m_Controls->cbParam4), SIGNAL( activated(int) ), this, SLOT( SelectInterpolator(int) ) );
  }

  m_TimeStepperAdapter = new QmitkStepperAdapter((QObject*) m_Controls->sliceNavigatorTime,
    GetActiveStdMultiWidget()->GetTimeNavigationController()->GetTime(), "sliceNavigatorTimeFromBIP");
}

void QmitkBasicImageProcessing::Activated()
{
  QmitkFunctionality::Activated();

  this->m_Controls->cbWhat1->clear();
  this->m_Controls->cbWhat1->insertItem( NOACTIONSELECTED, "Please select operation");
  this->m_Controls->cbWhat1->insertItem( CATEGORY_DENOISING, "--- Denoising ---");
  this->m_Controls->cbWhat1->insertItem( GAUSSIAN, "Gaussian");
  this->m_Controls->cbWhat1->insertItem( MEDIAN, "Median");
  this->m_Controls->cbWhat1->insertItem( TOTALVARIATION, "Total Variation");
  this->m_Controls->cbWhat1->insertItem( CATEGORY_MORPHOLOGICAL, "--- Morphological ---");
  this->m_Controls->cbWhat1->insertItem( DILATION, "Dilation");
  this->m_Controls->cbWhat1->insertItem( EROSION, "Erosion");
  this->m_Controls->cbWhat1->insertItem( OPENING, "Opening");
  this->m_Controls->cbWhat1->insertItem( CLOSING, "Closing");
  this->m_Controls->cbWhat1->insertItem( CATEGORY_EDGE_DETECTION, "--- Edge Detection ---");
  this->m_Controls->cbWhat1->insertItem( GRADIENT, "Gradient");
  this->m_Controls->cbWhat1->insertItem( LAPLACIAN, "Laplacian (2nd Derivative)");
  this->m_Controls->cbWhat1->insertItem( SOBEL, "Sobel Operator");
  this->m_Controls->cbWhat1->insertItem( CATEGORY_MISC, "--- Misc ---");
  this->m_Controls->cbWhat1->insertItem( THRESHOLD, "Threshold");
  this->m_Controls->cbWhat1->insertItem( INVERSION, "Image Inversion");
  this->m_Controls->cbWhat1->insertItem( DOWNSAMPLING, "Downsampling");
  this->m_Controls->cbWhat1->insertItem( FLIPPING, "Flipping");
  this->m_Controls->cbWhat1->insertItem( RESAMPLING, "Resample to");
  this->m_Controls->cbWhat1->insertItem( RESCALE, "Rescale image values");

  this->m_Controls->cbWhat2->clear();
  this->m_Controls->cbWhat2->insertItem( TWOIMAGESNOACTIONSELECTED, "Please select on operation" );
  this->m_Controls->cbWhat2->insertItem( CATEGORY_ARITHMETIC, "--- Arithmetric operations ---" );
  this->m_Controls->cbWhat2->insertItem( ADD, "Add to Image 1:" );
  this->m_Controls->cbWhat2->insertItem( SUBTRACT, "Subtract from Image 1:" );
  this->m_Controls->cbWhat2->insertItem( MULTIPLY, "Multiply with Image 1:" );
  this->m_Controls->cbWhat2->insertItem( RESAMPLE_TO, "Resample Image 1 to fit geometry:" );
  this->m_Controls->cbWhat2->insertItem( DIVIDE, "Divide Image 1 by:" );
  this->m_Controls->cbWhat2->insertItem( CATEGORY_BOOLEAN, "--- Boolean operations ---" );
  this->m_Controls->cbWhat2->insertItem( AND, "AND" );
  this->m_Controls->cbWhat2->insertItem( OR, "OR" );
  this->m_Controls->cbWhat2->insertItem( XOR, "XOR" );

  this->m_Controls->cbParam4->clear();
  this->m_Controls->cbParam4->insertItem( LINEAR, "Linear" );
  this->m_Controls->cbParam4->insertItem( NEAREST, "Nearest neighbor" );

  m_Controls->dsbParam1->hide();
  m_Controls->dsbParam2->hide();
  m_Controls->dsbParam3->hide();
  m_Controls->tlParam3->hide();
  m_Controls->tlParam4->hide();
  m_Controls->cbParam4->hide();
}

//datamanager selection changed
void QmitkBasicImageProcessing::OnSelectionChanged(std::vector<mitk::DataNode*> nodes)
{
  //any nodes there?
  if (!nodes.empty())
  {
  // reset GUI
//  this->ResetOneImageOpPanel();
  m_Controls->sliceNavigatorTime->setEnabled(false);
  m_Controls->leImage1->setText("Select an Image in Data Manager");
  m_Controls->tlWhat1->setEnabled(false);
  m_Controls->cbWhat1->setEnabled(false);
  m_Controls->tlWhat2->setEnabled(false);
  m_Controls->cbWhat2->setEnabled(false);

  m_SelectedImageNode->RemoveAllNodes();
  //get the selected Node
  mitk::DataNode* _DataNode = nodes.front();
  *m_SelectedImageNode = _DataNode;
  //try to cast to image
  mitk::Image::Pointer tempImage = dynamic_cast<mitk::Image*>(m_SelectedImageNode->GetNode()->GetData());

    //no image
    if( tempImage.IsNull() || (tempImage->IsInitialized() == false) )
    {
      m_Controls->leImage1->setText("Not an image.");
      return;
    }

    //2D image
    if( tempImage->GetDimension() < 3)
    {
      m_Controls->leImage1->setText("2D images are not supported.");
      return;
    }

    //image
    m_Controls->leImage1->setText(QString(m_SelectedImageNode->GetNode()->GetName().c_str()));

    // button coding
    if ( tempImage->GetDimension() > 3 )
    {
      m_Controls->sliceNavigatorTime->setEnabled(true);
      m_Controls->tlTime->setEnabled(true);
    }
    m_Controls->tlWhat1->setEnabled(true);
    m_Controls->cbWhat1->setEnabled(true);
    m_Controls->tlWhat2->setEnabled(true);
    m_Controls->cbWhat2->setEnabled(true);
  }
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

void QmitkBasicImageProcessing::ResetOneImageOpPanel()
{
  m_Controls->tlParam1->setText("Param1");
  m_Controls->tlParam2->setText("Param2");

  m_Controls->cbWhat1->setCurrentIndex(0);

  m_Controls->tlTime->setEnabled(false);

  this->ResetParameterPanel();

  m_Controls->btnDoIt->setEnabled(false);
  m_Controls->cbHideOrig->setEnabled(false);
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

void QmitkBasicImageProcessing::ResetTwoImageOpPanel()
{
  m_Controls->cbWhat2->setCurrentIndex(0);

  m_Controls->tlImage2->setEnabled(false);
  m_Controls->m_ImageSelector2->setEnabled(false);

  m_Controls->btnDoIt2->setEnabled(false);
}

void QmitkBasicImageProcessing::SelectAction(int action)
{
  if ( ! m_SelectedImageNode->GetNode() ) return;

  // Prepare GUI
  this->ResetParameterPanel();
  m_Controls->btnDoIt->setEnabled(false);
  m_Controls->cbHideOrig->setEnabled(false);

  QString text1 = "No Parameters";
  QString text2 = "No Parameters";
  QString text3 = "No Parameters";
  QString text4 = "No Parameters";

  if (action != 19)
  {
    m_Controls->dsbParam1->hide();
    m_Controls->dsbParam2->hide();
    m_Controls->dsbParam3->hide();
    m_Controls->tlParam3->hide();
    m_Controls->tlParam4->hide();
    m_Controls->sbParam1->show();
    m_Controls->sbParam2->show();
    m_Controls->cbParam4->hide();
  }
  // check which operation the user has selected and set parameters and GUI accordingly
  switch (action)
  {
  case 2:
    {
      m_SelectedAction = GAUSSIAN;
      m_Controls->tlParam1->setEnabled(true);
      m_Controls->sbParam1->setEnabled(true);
      text1 = "&Variance:";

      m_Controls->sbParam1->setMinimum( 0 );
      m_Controls->sbParam1->setMaximum( 200 );
      m_Controls->sbParam1->setValue( 2 );
      break;
    }

  case 3:
    {
      m_SelectedAction = MEDIAN;
      m_Controls->tlParam1->setEnabled(true);
      m_Controls->sbParam1->setEnabled(true);
      text1 = "&Radius:";
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
      text1 = "Number Iterations:";
      text2 = "Regularization\n(Lambda/1000):";
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
      text1 = "&Radius:";
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
      text1 = "&Radius:";
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
      text1 = "&Radius:";
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
      text1 = "&Radius:";
      m_Controls->sbParam1->setMinimum( 0 );
      m_Controls->sbParam1->setMaximum( 200 );
      m_Controls->sbParam1->setValue( 3 );
      break;
    }

  case 11:
    {
      m_SelectedAction = GRADIENT;
      m_Controls->tlParam1->setEnabled(true);
      m_Controls->sbParam1->setEnabled(true);
      text1 = "Sigma of Gaussian Kernel:\n(in Image Spacing Units)";
      m_Controls->sbParam1->setMinimum( 0 );
      m_Controls->sbParam1->setMaximum( 200 );
      m_Controls->sbParam1->setValue( 2 );
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
      text1 = "Lower threshold:";
      text2 = "Upper threshold:";
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
      text1 = "Downsampling by Factor:";
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
      text1 = "Flip across axis:";
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

      text1 = "x-spacing:";
      text2 = "y-spacing:";
      text3 = "z-spacing:";
      text4 = "Interplation:";
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
      text1 = "Output minimum:";
      text2 = "Output maximum:";
      break;
    }

  default: return;
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
  if(!m_SelectedImageNode->GetNode()) return;

  this->BusyCursorOn();

  mitk::Image::Pointer newImage;

  try
  {
    newImage = dynamic_cast<mitk::Image*>(m_SelectedImageNode->GetNode()->GetData());
  }
  catch ( std::exception &e )
  {
  QString exceptionString = "An error occured during image loading:\n";
  exceptionString.append( e.what() );
    QMessageBox::warning( NULL, "Basic Image Processing", exceptionString , QMessageBox::Ok, QMessageBox::NoButton );
    this->BusyCursorOff();
    return;
  }

  // check if input image is valid, casting does not throw exception when casting from 'NULL-Object'
  if ( (! newImage) || (newImage->IsInitialized() == false) )
  {
    this->BusyCursorOff();

    QMessageBox::warning( NULL, "Basic Image Processing", "Input image is broken or not initialized. Returning.", QMessageBox::Ok, QMessageBox::NoButton );
    return;
  }

  // check if operation is done on 4D a image time step
  if(newImage->GetDimension() > 3)
  {
    mitk::ImageTimeSelector::Pointer timeSelector = mitk::ImageTimeSelector::New();
    timeSelector->SetInput(newImage);
    timeSelector->SetTimeNr( ((QmitkSliderNavigatorWidget*)m_Controls->sliceNavigatorTime)->GetPos() );
    timeSelector->Update();
    newImage = timeSelector->GetOutput();
  }



  // check if image or vector image
  ImageType::Pointer itkImage = ImageType::New();
  VectorImageType::Pointer itkVecImage = VectorImageType::New();

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

  try{

  switch (m_SelectedAction)
  {

  case GAUSSIAN:
    {
      GaussianFilterType::Pointer gaussianFilter = GaussianFilterType::New();
      gaussianFilter->SetInput( itkImage );
      gaussianFilter->SetVariance( param1 );
      gaussianFilter->UpdateLargestPossibleRegion();
      newImage = mitk::ImportItkImage(gaussianFilter->GetOutput())->Clone();
      nameAddition << "_Gaussian_var_" << param1;
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
        FloatImageType::Pointer fImage = floatCaster->GetOutput();

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
      gradientFilter->SetSigma( param1 );
      gradientFilter->UpdateLargestPossibleRegion();
      newImage = mitk::ImportItkImage(gradientFilter->GetOutput())->Clone();
      nameAddition << "_Gradient_sigma_" << param1;
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
      FloatImageType::Pointer fImage = caster->GetOutput();

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
      FloatImageType::Pointer fImage = caster->GetOutput();

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
      mitk::ScalarType min = newImage->GetScalarValueMin();
      mitk::ScalarType max = newImage->GetScalarValueMax();
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

      newImage = mitk::ImportItkImage( resampledImage );
      nameAddition << "_Resampled_" << selectedInterpolator;
      std::cout << "Resampling successful." << std::endl;
      break;
    }


  case RESCALE:
    {
      FloatImageType::Pointer floatImage = FloatImageType::New();
      CastToItkImage( newImage, floatImage );
      itk::RescaleIntensityImageFilter<FloatImageType,FloatImageType>::Pointer filter = itk::RescaleIntensityImageFilter<FloatImageType,FloatImageType>::New();
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

  default:
    this->BusyCursorOff();
    return;
  }
  }
  catch (...)
  {
    this->BusyCursorOff();
    QMessageBox::warning(NULL, "Warning", "Problem when applying filter operation. Check your input...");
    return;
  }

  newImage->DisconnectPipeline();

  // adjust level/window to new image
  mitk::LevelWindow levelwindow;
  levelwindow.SetAuto( newImage );
  mitk::LevelWindowProperty::Pointer levWinProp = mitk::LevelWindowProperty::New();
  levWinProp->SetLevelWindow( levelwindow );

  // compose new image name
  std::string name = m_SelectedImageNode->GetNode()->GetName();
  if (name.find(".pic.gz") == name.size() -7 )
  {
    name = name.substr(0,name.size() -7);
  }
  name.append( nameAddition.str() );

  // create final result MITK data storage node
  mitk::DataNode::Pointer result = mitk::DataNode::New();
  result->SetProperty( "levelwindow", levWinProp );
  result->SetProperty( "name", mitk::StringProperty::New( name.c_str() ) );
  result->SetData( newImage );

  // for vector images, a different mapper is needed
  if(isVectorImage > 1)
  {
    mitk::VectorImageMapper2D::Pointer mapper =
      mitk::VectorImageMapper2D::New();
    result->SetMapper(1,mapper);
  }

  // reset GUI to ease further processing
//  this->ResetOneImageOpPanel();

  // add new image to data storage and set as active to ease further processing
  GetDefaultDataStorage()->Add( result, m_SelectedImageNode->GetNode() );
  if ( m_Controls->cbHideOrig->isChecked() == true )
    m_SelectedImageNode->GetNode()->SetProperty( "visible", mitk::BoolProperty::New(false) );
  // TODO!! m_Controls->m_ImageSelector1->SetSelectedNode(result);

  // show the results
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  this->BusyCursorOff();
}

void QmitkBasicImageProcessing::SelectAction2(int operation)
{
  // check which operation the user has selected and set parameters and GUI accordingly
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
//    this->ResetTwoImageOpPanel();
    return;
  }
  m_Controls->tlImage2->setEnabled(true);
  m_Controls->m_ImageSelector2->setEnabled(true);
  m_Controls->btnDoIt2->setEnabled(true);
}

void QmitkBasicImageProcessing::StartButton2Clicked()
{
  mitk::Image::Pointer newImage1 = dynamic_cast<mitk::Image*>
    (m_SelectedImageNode->GetNode()->GetData());
  mitk::Image::Pointer newImage2 = dynamic_cast<mitk::Image*>
    (m_Controls->m_ImageSelector2->GetSelectedNode()->GetData());

  // check if images are valid
  if( (!newImage1) || (!newImage2) || (newImage1->IsInitialized() == false) || (newImage2->IsInitialized() == false) )
  {
    itkGenericExceptionMacro(<< "At least one of the input images are broken or not initialized. Returning");
    return;
  }

  this->BusyCursorOn();
//  this->ResetTwoImageOpPanel();

  // check if 4D image and use filter on correct time step
  int time = ((QmitkSliderNavigatorWidget*)m_Controls->sliceNavigatorTime)->GetPos();
  if(newImage1->GetDimension() > 3)
  {
    mitk::ImageTimeSelector::Pointer timeSelector = mitk::ImageTimeSelector::New();

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

  // reset GUI for better usability
//  this->ResetTwoImageOpPanel();

  ImageType::Pointer itkImage1 = ImageType::New();
  ImageType::Pointer itkImage2 = ImageType::New();

  CastToItkImage( newImage1, itkImage1 );
  CastToItkImage( newImage2, itkImage2 );

  // Remove temp image
  newImage2 = NULL;

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
      newImage1 = mitk::ImportItkImage<FloatImageType>(divFilter->GetOutput())->Clone();
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

      itk::NearestNeighborInterpolateImageFunction<ImageType>::Pointer nn_interpolator
        = itk::NearestNeighborInterpolateImageFunction<ImageType>::New();

      ResampleImageFilterType2::Pointer resampleFilter = ResampleImageFilterType2::New();
      resampleFilter->SetInput( itkImage1 );
      resampleFilter->SetReferenceImage( itkImage2 );
      resampleFilter->SetUseReferenceImage( true );
      resampleFilter->SetInterpolator( nn_interpolator );
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

      ImageType::Pointer resampledImage = resampleFilter->GetOutput();

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
    QMessageBox::warning(NULL, "ITK Exception", e.what() );
    QMessageBox::warning(NULL, "Warning", "Problem when applying arithmetic operation to two images. Check dimensions of input images.");
    return;
  }

  // disconnect pipeline; images will not be reused
  newImage1->DisconnectPipeline();
  itkImage1 = NULL;
  itkImage2 = NULL;

  // adjust level/window to new image and compose new image name
  mitk::LevelWindow levelwindow;
  levelwindow.SetAuto( newImage1 );
  mitk::LevelWindowProperty::Pointer levWinProp = mitk::LevelWindowProperty::New();
  levWinProp->SetLevelWindow( levelwindow );
  std::string name = m_SelectedImageNode->GetNode()->GetName();
  if (name.find(".pic.gz") == name.size() -7 )
  {
    name = name.substr(0,name.size() -7);
  }

  // create final result MITK data storage node
  mitk::DataNode::Pointer result = mitk::DataNode::New();
  result->SetProperty( "levelwindow", levWinProp );
  result->SetProperty( "name", mitk::StringProperty::New( (name + nameAddition ).c_str() ));
  result->SetData( newImage1 );
  GetDefaultDataStorage()->Add( result, m_SelectedImageNode->GetNode() );

  // show only the newly created image
  m_SelectedImageNode->GetNode()->SetProperty( "visible", mitk::BoolProperty::New(false) );
  m_Controls->m_ImageSelector2->GetSelectedNode()->SetProperty( "visible", mitk::BoolProperty::New(false) );

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
