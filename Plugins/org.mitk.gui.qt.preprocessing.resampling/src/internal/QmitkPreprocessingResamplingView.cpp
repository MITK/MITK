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

#include "QmitkPreprocessingResamplingView.h"

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

// Resampling
#include <itkResampleImageFilter.h>
#include <itkNearestNeighborInterpolateImageFunction.h>
#include <itkBSplineInterpolateImageFunction.h>
#include <itkCastImageFilter.h>
#include <itkLinearInterpolateImageFunction.h>

#include <itkRescaleIntensityImageFilter.h>
#include <itkShiftScaleImageFilter.h>


// Convenient Definitions
typedef itk::Image<short, 3>                                                            ImageType;
typedef itk::Image<unsigned char, 3>                                                    SegmentationImageType;
typedef itk::Image<double, 3>                                                           DoubleImageType;
typedef itk::Image<itk::Vector<float,3>, 3>                                             VectorImageType;

typedef itk::ResampleImageFilter< ImageType, ImageType >                                ResampleImageFilterType;
typedef itk::ResampleImageFilter< ImageType, ImageType >                                ResampleImageFilterType2;
typedef itk::CastImageFilter< ImageType, DoubleImageType >                               ImagePTypeToFloatPTypeCasterType;

typedef itk::LinearInterpolateImageFunction< ImageType, double >                        LinearInterpolatorType;
typedef itk::NearestNeighborInterpolateImageFunction< ImageType, double >               NearestInterpolatorType;
typedef itk::BSplineInterpolateImageFunction<ImageType, double>                         BSplineInterpolatorType;


QmitkPreprocessingResampling::QmitkPreprocessingResampling()
: QmitkAbstractView(),
  m_Controls(NULL),
  m_SelectedImageNode(NULL),
  m_TimeStepperAdapter(NULL)
{
}

QmitkPreprocessingResampling::~QmitkPreprocessingResampling()
{
}

void QmitkPreprocessingResampling::CreateQtPartControl(QWidget *parent)
{
  if (m_Controls == NULL)
  {
    m_Controls = new Ui::QmitkPreprocessingResamplingViewControls;
    m_Controls->setupUi(parent);
    this->CreateConnections();

    mitk::NodePredicateDimension::Pointer dimensionPredicate = mitk::NodePredicateDimension::New(3);
    mitk::NodePredicateDataType::Pointer imagePredicate = mitk::NodePredicateDataType::New("Image");
  }

  m_SelectedImageNode = mitk::DataStorageSelection::New(this->GetDataStorage(), false);

  // Setup Controls
  this->m_Controls->cbParam4->clear();
  this->m_Controls->cbParam4->insertItem(LINEAR, "Linear");
  this->m_Controls->cbParam4->insertItem(NEAREST, "Nearest neighbor");
  this->m_Controls->cbParam4->insertItem(SPLINE, "B-Spline");

}

void QmitkPreprocessingResampling::CreateConnections()
{
  if ( m_Controls )
  {
    connect( (QObject*)(m_Controls->btnDoIt), SIGNAL(clicked()),(QObject*) this, SLOT(StartButtonClicked()));
    connect( (QObject*)(m_Controls->cbParam4), SIGNAL( activated(int) ), this, SLOT( SelectInterpolator(int) ) );
  }
}

void QmitkPreprocessingResampling::InternalGetTimeNavigationController()
{
  auto renwin_part = GetRenderWindowPart();
  if( renwin_part != nullptr )
  {
    auto tnc = renwin_part->GetTimeNavigationController();
    if( tnc != nullptr )
    {
      m_TimeStepperAdapter = new QmitkStepperAdapter((QObject*) m_Controls->sliceNavigatorTime, tnc->GetTime(), "sliceNavigatorTimeFromBIP");
    }
  }
}

void QmitkPreprocessingResampling::SetFocus()
{
}

//datamanager selection changed
void QmitkPreprocessingResampling::OnSelectionChanged(berry::IWorkbenchPart::Pointer, const QList<mitk::DataNode::Pointer>& nodes)
{
  ResetOneImageOpPanel();
  //any nodes there?
  if (!nodes.empty())
  {
  // reset GUI
  m_Controls->sliceNavigatorTime->setEnabled(false);
  m_Controls->leImage1->setText(tr("Select an Image in Data Manager"));

  m_SelectedImageNode->RemoveAllNodes();
  //get the selected Node
  mitk::DataNode* _DataNode = nodes.front();
  *m_SelectedImageNode = _DataNode;
  //try to cast to image
  mitk::Image::Pointer tempImage = dynamic_cast<mitk::Image*>(m_SelectedImageNode->GetNode()->GetData());

    //no image
    if( tempImage.IsNull() || (tempImage->IsInitialized() == false) )
    {
      m_Controls->leImage1->setText(tr("Not an image."));
      return;
    }

    //2D image
    if( tempImage->GetDimension() < 3)
    {
      m_Controls->leImage1->setText(tr("2D images are not supported."));
      return;
    }

    //image
    m_Controls->leImage1->setText(QString(m_SelectedImageNode->GetNode()->GetName().c_str()));
    mitk::Vector3D aSpacing = tempImage->GetGeometry()->GetSpacing();
    std::string text("x-spacing (" + std::to_string(aSpacing[0]) + ")");
    m_Controls->tlParam1->setText(text.c_str());
    text = "y-spacing (" + std::to_string(aSpacing[1]) + ")";
    m_Controls->tlParam2->setText(text.c_str());
    text = "z-spacing (" + std::to_string(aSpacing[2]) + ")";
    m_Controls->tlParam3->setText(text.c_str());
    MITK_INFO << "Spacing of current Image : " << aSpacing;
//    m_Controls->tlParam1->setText("y-spacing");
//    m_Controls->tlParam1->setText("z-spacing");

    // button coding
    if ( tempImage->GetDimension() > 3 )
    {
      // try to retrieve the TNC (for 4-D Processing )
      this->InternalGetTimeNavigationController();

      m_Controls->sliceNavigatorTime->setEnabled(true);
      m_Controls->tlTime->setEnabled(true);
    }
    ResetParameterPanel();
  }
}

void QmitkPreprocessingResampling::ResetOneImageOpPanel()
{
  m_Controls->tlTime->setEnabled(false);
  m_Controls->btnDoIt->setEnabled(false);
  m_Controls->cbHideOrig->setEnabled(false);
  m_Controls->leImage1->setText(tr("Select an Image in Data Manager"));
  m_Controls->tlParam1->setText("x-spacing");
  m_Controls->tlParam1->setText("y-spacing");
  m_Controls->tlParam1->setText("z-spacing");
}

void QmitkPreprocessingResampling::ResetParameterPanel()
{
  m_Controls->btnDoIt->setEnabled(true);
  m_Controls->cbHideOrig->setEnabled(true);
}

void QmitkPreprocessingResampling::ResetTwoImageOpPanel()
{
}

void QmitkPreprocessingResampling::StartButtonClicked()
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
  QString exceptionString = tr("An error occured during image loading:\n");
  exceptionString.append( e.what() );
    QMessageBox::warning( NULL, "Preprocessing - Resampling: ", exceptionString , QMessageBox::Ok, QMessageBox::NoButton );
    this->BusyCursorOff();
    return;
  }

  // check if input image is valid, casting does not throw exception when casting from 'NULL-Object'
  if ( (! newImage) || (newImage->IsInitialized() == false) )
  {
    this->BusyCursorOff();

    QMessageBox::warning( NULL, "Preprocessing - Resampling", tr("Input image is broken or not initialized. Returning."), QMessageBox::Ok, QMessageBox::NoButton );
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

  double dparam1 = m_Controls->dsbParam1->value();
  double dparam2 = m_Controls->dsbParam2->value();
  double dparam3 = m_Controls->dsbParam3->value();

  try{

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
      case SPLINE:
      {
        BSplineInterpolatorType::Pointer interpolator = BSplineInterpolatorType::New();
        interpolator->SetSplineOrder(3);
        resampler->SetInterpolator(interpolator);
        selectedInterpolator = "B-Spline";
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
  GetDataStorage()->Add( result, m_SelectedImageNode->GetNode() );
  if ( m_Controls->cbHideOrig->isChecked() == true )
    m_SelectedImageNode->GetNode()->SetProperty( "visible", mitk::BoolProperty::New(false) );
  // TODO!! m_Controls->m_ImageSelector1->SetSelectedNode(result);

  // show the results
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  this->BusyCursorOff();
}

void QmitkPreprocessingResampling::SelectInterpolator(int interpolator)
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
  case 2:
  {
    m_SelectedInterpolation = SPLINE;
  }
  }
}
