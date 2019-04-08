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

//#define MBILOG_ENABLE_DEBUG

#include "QmitkPreprocessingView.h"
#include "mitkDiffusionImagingConfigure.h"

// qt includes
#include <QMessageBox>

// itk includes
#include "itkTimeProbe.h"
#include "itkB0ImageExtractionImageFilter.h"
#include "itkB0ImageExtractionToSeparateImageFilter.h"
#include "itkBrainMaskExtractionImageFilter.h"
#include "itkCastImageFilter.h"
#include "itkVectorContainer.h"
#include <itkElectrostaticRepulsionDiffusionGradientReductionFilter.h>
#include <itkMergeDiffusionImagesFilter.h>
#include <itkDwiGradientLengthCorrectionFilter.h>
#include <itkDwiNormilzationFilter.h>
#include <mitkTensorImage.h>
#include <mitkOdfImage.h>

// Multishell includes
#include <itkRadialMultishellToSingleshellImageFilter.h>

// Multishell Functors
#include <itkADCAverageFunctor.h>
#include <itkKurtosisFitFunctor.h>
#include <itkBiExpFitFunctor.h>
#include <itkADCFitFunctor.h>

// mitk includes
#include "QmitkDataStorageComboBox.h"
#include "mitkProgressBar.h"
#include "mitkStatusBar.h"
#include "mitkNodePredicateDataType.h"
#include "mitkProperties.h"
#include "mitkVtkResliceInterpolationProperty.h"
#include "mitkLookupTable.h"
#include "mitkLookupTableProperty.h"
#include "mitkTransferFunction.h"
#include "mitkTransferFunctionProperty.h"
#include "mitkDataNodeObject.h"
#include "mitkOdfNormalizationMethodProperty.h"
#include "mitkOdfScaleByProperty.h"
#include <mitkPointSet.h>
#include <itkAdcImageFilter.h>
#include <itkBrainMaskExtractionImageFilter.h>
#include <mitkImageCast.h>
#include <mitkRotationOperation.h>
#include <QTableWidgetItem>
#include <QTableWidget>
#include <mitkInteractionConst.h>
#include <mitkImageAccessByItk.h>
#include <itkResampleDwiImageFilter.h>
#include <itkResampleImageFilter.h>
#include <itkImageDuplicator.h>
#include <itkMaskImageFilter.h>
#include <mitkNodePredicateProperty.h>
#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateOr.h>
#include <itkCropImageFilter.h>
#include <itkDiffusionTensor3DReconstructionImageFilter.h>
#include <itkRemoveDwiChannelFilter.h>
#include <itkExtractDwiChannelFilter.h>
#include <mitkBValueMapProperty.h>
#include <mitkGradientDirectionsProperty.h>
#include <mitkMeasurementFrameProperty.h>
#include <itkImageDuplicator.h>
#include <itkFlipImageFilter.h>
#include <mitkITKImageImport.h>

const std::string QmitkPreprocessingView::VIEW_ID =
    "org.mitk.views.diffusionpreprocessing";

#define DI_INFO MITK_INFO("DiffusionImaging")


typedef float TTensorPixelType;


QmitkPreprocessingView::QmitkPreprocessingView()
  : QmitkAbstractView(),
    m_Controls(nullptr)
{
}

QmitkPreprocessingView::~QmitkPreprocessingView()
{

}

void QmitkPreprocessingView::CreateQtPartControl(QWidget *parent)
{
  if (!m_Controls)
  {
    // create GUI widgets
    m_Controls = new Ui::QmitkPreprocessingViewControls;
    m_Controls->setupUi(parent);
    this->CreateConnections();

    m_Controls->m_MeasurementFrameTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_Controls->m_MeasurementFrameTable->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_Controls->m_DirectionMatrixTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_Controls->m_DirectionMatrixTable->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  }
}

void QmitkPreprocessingView::SetFocus()
{
  m_Controls->m_MirrorGradientToHalfSphereButton->setFocus();
}

void QmitkPreprocessingView::CreateConnections()
{
  if ( m_Controls )
  {
    m_Controls->m_NormalizationMaskBox->SetDataStorage(this->GetDataStorage());
    m_Controls->m_SelctedImageComboBox->SetDataStorage(this->GetDataStorage());
    m_Controls->m_MergeDwiBox->SetDataStorage(this->GetDataStorage());
    m_Controls->m_AlignImageBox->SetDataStorage(this->GetDataStorage());

    mitk::TNodePredicateDataType<mitk::Image>::Pointer isMitkImage = mitk::TNodePredicateDataType<mitk::Image>::New();

    mitk::NodePredicateDataType::Pointer isDti = mitk::NodePredicateDataType::New("TensorImage");
    mitk::NodePredicateDataType::Pointer isOdf = mitk::NodePredicateDataType::New("OdfImage");
    mitk::NodePredicateOr::Pointer isDiffusionImage = mitk::NodePredicateOr::New(isOdf, isDti);

    mitk::NodePredicateAnd::Pointer noDiffusionImage = mitk::NodePredicateAnd::New(isMitkImage, mitk::NodePredicateNot::New(isDiffusionImage));
    mitk::NodePredicateProperty::Pointer isBinaryPredicate = mitk::NodePredicateProperty::New("binary", mitk::BoolProperty::New(true));
    mitk::NodePredicateAnd::Pointer binaryNoDiffusionImage = mitk::NodePredicateAnd::New(noDiffusionImage, isBinaryPredicate);

    m_Controls->m_NormalizationMaskBox->SetPredicate(binaryNoDiffusionImage);
    m_Controls->m_SelctedImageComboBox->SetPredicate(isMitkImage);
    m_Controls->m_MergeDwiBox->SetPredicate(isMitkImage);
    m_Controls->m_AlignImageBox->SetPredicate(isMitkImage);

    m_Controls->m_ExtractBrainMask->setVisible(false);
    m_Controls->m_BrainMaskIterationsBox->setVisible(false);
    m_Controls->m_ResampleIntFrame->setVisible(false);
    connect( (QObject*)(m_Controls->m_ButtonAverageGradients), SIGNAL(clicked()), this, SLOT(AverageGradients()) );
    connect( (QObject*)(m_Controls->m_ButtonExtractB0), SIGNAL(clicked()), this, SLOT(ExtractB0()) );
    connect( (QObject*)(m_Controls->m_ReduceGradientsButton), SIGNAL(clicked()), this, SLOT(DoReduceGradientDirections()) );
    connect( (QObject*)(m_Controls->m_ShowGradientsButton), SIGNAL(clicked()), this, SLOT(DoShowGradientDirections()) );
    connect( (QObject*)(m_Controls->m_MirrorGradientToHalfSphereButton), SIGNAL(clicked()), this, SLOT(DoHalfSphereGradientDirections()) );
    connect( (QObject*)(m_Controls->m_MergeDwisButton), SIGNAL(clicked()), this, SLOT(MergeDwis()) );
    connect( (QObject*)(m_Controls->m_ProjectSignalButton), SIGNAL(clicked()), this, SLOT(DoProjectSignal()) );
    connect( (QObject*)(m_Controls->m_B_ValueMap_Rounder_SpinBox), SIGNAL(valueChanged(int)), this, SLOT(UpdateDwiBValueMapRounder(int)));
    connect( (QObject*)(m_Controls->m_CreateLengthCorrectedDwi), SIGNAL(clicked()), this, SLOT(DoLengthCorrection()) );
    connect( (QObject*)(m_Controls->m_NormalizeImageValuesButton), SIGNAL(clicked()), this, SLOT(DoDwiNormalization()) );
    connect( (QObject*)(m_Controls->m_ResampleImageButton), SIGNAL(clicked()), this, SLOT(DoResampleImage()) );
    connect( (QObject*)(m_Controls->m_ResampleTypeBox), SIGNAL(currentIndexChanged(int)), this, SLOT(DoUpdateInterpolationGui(int)) );
    connect( (QObject*)(m_Controls->m_CropImageButton), SIGNAL(clicked()), this, SLOT(DoCropImage()) );
    connect( (QObject*)(m_Controls->m_RemoveGradientButton), SIGNAL(clicked()), this, SLOT(DoRemoveGradient()) );
    connect( (QObject*)(m_Controls->m_ExtractGradientButton), SIGNAL(clicked()), this, SLOT(DoExtractGradient()) );
    connect( (QObject*)(m_Controls->m_FlipAxis), SIGNAL(clicked()), this, SLOT(DoFlipAxis()) );
    connect( (QObject*)(m_Controls->m_FlipGradientsButton), SIGNAL(clicked()), this, SLOT(DoFlipGradientDirections()) );
    connect( (QObject*)(m_Controls->m_ClearRotationButton), SIGNAL(clicked()), this, SLOT(DoClearRotationOfGradients()) );
    connect( (QObject*)(m_Controls->m_ModifyHeader), SIGNAL(clicked()), this, SLOT(DoApplyHeader()) );
    connect( (QObject*)(m_Controls->m_AlignImageButton), SIGNAL(clicked()), this, SLOT(DoAlignImages()) );



    connect( (QObject*)(m_Controls->m_SelctedImageComboBox), SIGNAL(OnSelectionChanged(const mitk::DataNode*)),
             this, SLOT(OnImageSelectionChanged()) );

    m_Controls->m_NormalizationMaskBox->SetZeroEntryText("--");

  }
}

void QmitkPreprocessingView::DoAlignImages()
{
  mitk::DataNode::Pointer node = m_Controls->m_SelctedImageComboBox->GetSelectedNode();
  if (node.IsNull()) { return; }

  mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(node->GetData());
  if ( image == nullptr ) { return; }


  mitk::DataNode::Pointer target_node = m_Controls->m_AlignImageBox->GetSelectedNode();
  if (target_node.IsNull()) { return; }

  mitk::Image::Pointer target_image = dynamic_cast<mitk::Image*>(target_node->GetData());
  if ( target_image == nullptr ) { return; }

  image->SetOrigin(target_image->GetGeometry()->GetOrigin());
  mitk::RenderingManager::GetInstance()->InitializeViews( image->GetTimeGeometry(),
                                                          mitk::RenderingManager::REQUEST_UPDATE_ALL,
                                                          true );
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkPreprocessingView::DoFlipAxis()
{
  mitk::DataNode::Pointer node = m_Controls->m_SelctedImageComboBox->GetSelectedNode();
  if (node.IsNull()) { return; }

  mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(node->GetData());
  if ( image == nullptr ) { return; }

  bool isDiffusionImage( PropHelper::IsDiffusionWeightedImage(node) );

  if (isDiffusionImage)
  {
    ItkDwiType::Pointer itkVectorImagePointer = ItkDwiType::New();
    mitk::CastToItkImage(image, itkVectorImagePointer);

    itk::FixedArray<bool, 3> flipAxes;
    flipAxes[0] = m_Controls->m_FlipX->isChecked();
    flipAxes[1] = m_Controls->m_FlipY->isChecked();
    flipAxes[2] = m_Controls->m_FlipZ->isChecked();

    itk::FlipImageFilter< ItkDwiType >::Pointer flipper = itk::FlipImageFilter< ItkDwiType >::New();
    flipper->SetInput(itkVectorImagePointer);
    flipper->SetFlipAxes(flipAxes);
    flipper->Update();

    auto oldGradients = PropHelper::GetGradientContainer(image);
    auto newGradients = GradProp::GradientDirectionsContainerType::New();

    for (unsigned int i=0; i<oldGradients->Size(); i++)
    {
      GradProp::GradientDirectionType g = oldGradients->GetElement(i);
      GradProp::GradientDirectionType newG = g;
      if (flipAxes[0]) { newG[0] *= -1; }

      if (flipAxes[1]) { newG[1] *= -1; }

      if (flipAxes[2]) { newG[2] *= -1; }

      newGradients->InsertElement(i, newG);
    }

    mitk::Image::Pointer newImage = mitk::GrabItkImageMemory( flipper->GetOutput() );
    PropHelper::CopyProperties(image, newImage, true);
    PropHelper::SetGradientContainer(newImage, newGradients);
    PropHelper::InitializeImage(newImage);
    newImage->GetGeometry()->SetOrigin(image->GetGeometry()->GetOrigin());

    mitk::DataNode::Pointer imageNode = mitk::DataNode::New();
    imageNode->SetData( newImage );
    QString name = node->GetName().c_str();

    imageNode->SetName((name+"_flipped").toStdString().c_str());
    GetDataStorage()->Add(imageNode, node);

    mitk::RenderingManager::GetInstance()->InitializeViews( imageNode->GetData()->GetTimeGeometry(), mitk::RenderingManager::REQUEST_UPDATE_ALL, true );
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
  else if( image->GetPixelType().GetNumberOfComponents() == 1 )
  {
    AccessFixedDimensionByItk(image, TemplatedFlipAxis,3);
  }
  else
  {
    QMessageBox::warning(nullptr,"Warning", QString("Operation not supported in multi-component images") );
  }
}


template < typename TPixel, unsigned int VImageDimension >
void QmitkPreprocessingView::TemplatedFlipAxis(itk::Image<TPixel, VImageDimension>* itkImage)
{
  mitk::DataNode::Pointer node = m_Controls->m_SelctedImageComboBox->GetSelectedNode();
  if (node.IsNull()) { return; }

  mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(node->GetData());
  if ( image == nullptr ) { return; }

  itk::FixedArray<bool, 3> flipAxes;
  flipAxes[0] = m_Controls->m_FlipX->isChecked();
  flipAxes[1] = m_Controls->m_FlipY->isChecked();
  flipAxes[2] = m_Controls->m_FlipZ->isChecked();

  typename itk::FlipImageFilter< itk::Image<TPixel, VImageDimension> >::Pointer flipper
      = itk::FlipImageFilter< itk::Image<TPixel, VImageDimension> >::New();
  flipper->SetInput(itkImage);
  flipper->SetFlipAxes(flipAxes);
  flipper->Update();

  mitk::Image::Pointer newImage = mitk::GrabItkImageMemory( flipper->GetOutput() );
  newImage->GetGeometry()->SetOrigin(image->GetGeometry()->GetOrigin());

  mitk::DataNode::Pointer imageNode = mitk::DataNode::New();
  imageNode->SetData( newImage );
  QString name = node->GetName().c_str();

  imageNode->SetName((name+"_flipped").toStdString().c_str());
  GetDataStorage()->Add(imageNode, node);

  mitk::RenderingManager::GetInstance()->InitializeViews( imageNode->GetData()->GetTimeGeometry(),
                                                          mitk::RenderingManager::REQUEST_UPDATE_ALL,
                                                          true );
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkPreprocessingView::DoRemoveGradient()
{
  mitk::DataNode::Pointer node = m_Controls->m_SelctedImageComboBox->GetSelectedNode();
  if (node.IsNull()) { return; }

  mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(node->GetData());
  if ( image == nullptr ) { return; }

  bool isDiffusionImage( PropHelper::IsDiffusionWeightedImage(image) );
  if ( !isDiffusionImage )
  {
    return;
  }

  std::vector< unsigned int > channelsToRemove;
  channelsToRemove.push_back(m_Controls->m_RemoveGradientBox->value());

  ItkDwiType::Pointer itkVectorImagePointer = ItkDwiType::New();
  mitk::CastToItkImage(image, itkVectorImagePointer);

  itk::RemoveDwiChannelFilter< short >::Pointer filter = itk::RemoveDwiChannelFilter< short >::New();
  filter->SetInput(itkVectorImagePointer);
  filter->SetChannelIndices(channelsToRemove);
  filter->SetDirections(PropHelper::GetGradientContainer(image));
  filter->Update();

  mitk::Image::Pointer newImage = mitk::GrabItkImageMemory( filter->GetOutput() );
  PropHelper::CopyProperties(image, newImage, true);
  PropHelper::SetGradientContainer(newImage, filter->GetNewDirections());
  PropHelper::InitializeImage( newImage );

  mitk::DataNode::Pointer imageNode = mitk::DataNode::New();
  imageNode->SetData( newImage );
  QString name = node->GetName().c_str();

  imageNode->SetName((name+"_removedgradients").toStdString().c_str());
  GetDataStorage()->Add(imageNode, node);

  mitk::RenderingManager::GetInstance()->InitializeViews( imageNode->GetData()->GetTimeGeometry(),
                                                          mitk::RenderingManager::REQUEST_UPDATE_ALL,
                                                          true );

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkPreprocessingView::DoExtractGradient()
{
  mitk::DataNode::Pointer node = m_Controls->m_SelctedImageComboBox->GetSelectedNode();
  if (node.IsNull()) { return; }

  mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(node->GetData());
  if ( image == nullptr ) { return; }

  bool isDiffusionImage( PropHelper::IsDiffusionWeightedImage(image) );
  if ( !isDiffusionImage ) { return; }

  ItkDwiType::Pointer itkVectorImagePointer = ItkDwiType::New();
  mitk::CastToItkImage(image, itkVectorImagePointer);

  unsigned int channel = m_Controls->m_ExtractGradientBox->value();
  itk::ExtractDwiChannelFilter< short >::Pointer filter = itk::ExtractDwiChannelFilter< short >::New();
  filter->SetInput( itkVectorImagePointer);
  filter->SetChannelIndex(channel);
  filter->Update();

  mitk::Image::Pointer newImage = mitk::Image::New();
  newImage->InitializeByItk( filter->GetOutput() );
  newImage->SetImportChannel( filter->GetOutput()->GetBufferPointer() );

  mitk::DataNode::Pointer imageNode = mitk::DataNode::New();
  imageNode->SetData( newImage );

  QString name = node->GetName().c_str();
  imageNode->SetName( (name+"_direction-"+QString::number(channel)).toStdString().c_str() );
  GetDataStorage()->Add(imageNode, node);

  mitk::RenderingManager::GetInstance()->InitializeViews( imageNode->GetData()->GetTimeGeometry(),
                                                          mitk::RenderingManager::REQUEST_UPDATE_ALL,
                                                          true);
}

void QmitkPreprocessingView::DoCropImage()
{
  mitk::DataNode::Pointer node = m_Controls->m_SelctedImageComboBox->GetSelectedNode();
  if (node.IsNull()) { return; }

  mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(node->GetData());
  if ( image == nullptr ) { return; }

  bool isDiffusionImage( PropHelper::IsDiffusionWeightedImage(image) );
  if ( isDiffusionImage )
  {
    ItkDwiType::Pointer itkVectorImagePointer = ItkDwiType::New();
    mitk::CastToItkImage(image, itkVectorImagePointer);

    ItkDwiType::SizeType lower;
    ItkDwiType::SizeType upper;
    lower[0] = m_Controls->m_XstartBox->value();
    lower[1] = m_Controls->m_YstartBox->value();
    lower[2] = m_Controls->m_ZstartBox->value();
    upper[0] = m_Controls->m_XendBox->value();
    upper[1] = m_Controls->m_YendBox->value();
    upper[2] = m_Controls->m_ZendBox->value();

    itk::CropImageFilter< ItkDwiType, ItkDwiType >::Pointer cropper = itk::CropImageFilter< ItkDwiType, ItkDwiType >::New();
    cropper->SetLowerBoundaryCropSize(lower);
    cropper->SetUpperBoundaryCropSize(upper);
    cropper->SetInput( itkVectorImagePointer );
    cropper->Update();

    ItkDwiType::Pointer itkOutImage = cropper->GetOutput();
    ItkDwiType::DirectionType dir = itkOutImage->GetDirection();
    itk::Point<double,3> origin = itkOutImage->GetOrigin();

    itk::Point<double,3> t; t[0] = lower[0]*itkOutImage->GetSpacing()[0]; t[1] = lower[1]*itkOutImage->GetSpacing()[1]; t[2] = lower[2]*itkOutImage->GetSpacing()[2];
    t= dir*t;

    origin[0] += t[0];
    origin[1] += t[1];
    origin[2] += t[2];

    itkOutImage->SetOrigin(origin);

    mitk::Image::Pointer newimage = mitk::GrabItkImageMemory( itkOutImage );
    PropHelper::CopyProperties(image, newimage, false);
    PropHelper::InitializeImage( newimage );

    mitk::DataNode::Pointer imageNode = mitk::DataNode::New();
    imageNode->SetData( newimage );
    QString name = node->GetName().c_str();
    imageNode->SetName((name+"_cropped").toStdString().c_str());
    GetDataStorage()->Add(imageNode, node);
    mitk::RenderingManager::GetInstance()->InitializeViews( imageNode->GetData()->GetTimeGeometry(),
                                                            mitk::RenderingManager::REQUEST_UPDATE_ALL,
                                                            true );
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();

    mitk::RenderingManager::GetInstance()->InitializeViews( imageNode->GetData()->GetTimeGeometry(),
                                                            mitk::RenderingManager::REQUEST_UPDATE_ALL,
                                                            true );
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
  else if( image->GetPixelType().GetNumberOfComponents() )
  {
    AccessFixedDimensionByItk(image, TemplatedCropImage,3);
  }
  else
  {
    QMessageBox::warning(nullptr,"Warning", QString("Operation not supported in multi-component images") );
  }
}

template < typename TPixel, unsigned int VImageDimension >
void QmitkPreprocessingView::TemplatedCropImage( itk::Image<TPixel, VImageDimension>* itkImage)
{
  mitk::DataNode::Pointer node = m_Controls->m_SelctedImageComboBox->GetSelectedNode();
  if (node.IsNull()) { return; }

  ItkDwiType::SizeType lower;
  ItkDwiType::SizeType upper;
  lower[0] = m_Controls->m_XstartBox->value();
  lower[1] = m_Controls->m_YstartBox->value();
  lower[2] = m_Controls->m_ZstartBox->value();
  upper[0] = m_Controls->m_XendBox->value();
  upper[1] = m_Controls->m_YendBox->value();
  upper[2] = m_Controls->m_ZendBox->value();

  typedef itk::Image<TPixel, VImageDimension> ImageType;
  typename itk::CropImageFilter< ImageType, ImageType >::Pointer cropper
      = itk::CropImageFilter< ImageType, ImageType >::New();
  cropper->SetLowerBoundaryCropSize(lower);
  cropper->SetUpperBoundaryCropSize(upper);
  cropper->SetInput(itkImage);
  cropper->Update();

  typename ImageType::Pointer itkOutImage = cropper->GetOutput();
  typename ImageType::DirectionType dir = itkOutImage->GetDirection();
  itk::Point<double,3> origin = itkOutImage->GetOrigin();

  itk::Point<double,3> t; t[0] = lower[0]*itkOutImage->GetSpacing()[0]; t[1] = lower[1]*itkOutImage->GetSpacing()[1]; t[2] = lower[2]*itkOutImage->GetSpacing()[2];
  t= dir*t;

  origin[0] += t[0];
  origin[1] += t[1];
  origin[2] += t[2];

  itkOutImage->SetOrigin(origin);
  mitk::Image::Pointer image = mitk::Image::New();
  image->InitializeByItk( itkOutImage.GetPointer() );
  image->SetVolume( itkOutImage->GetBufferPointer() );
  mitk::DataNode::Pointer imageNode = mitk::DataNode::New();
  imageNode->SetData( image );
  QString name = node->GetName().c_str();

  imageNode->SetName((name+"_cropped").toStdString().c_str());
  GetDataStorage()->Add(imageNode, node);

  mitk::RenderingManager::GetInstance()->InitializeViews( imageNode->GetData()->GetTimeGeometry(),
                                                          mitk::RenderingManager::REQUEST_UPDATE_ALL,
                                                          true );
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkPreprocessingView::DoUpdateInterpolationGui(int i)
{
  mitk::DataNode::Pointer node = m_Controls->m_SelctedImageComboBox->GetSelectedNode();
  if (node.IsNull()) { return; }

  mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(node->GetData());
  if ( image == nullptr ) { return; }

  switch (i)
  {
    case 0:
    {
      m_Controls->m_ResampleIntFrame->setVisible(false);
      m_Controls->m_ResampleDoubleFrame->setVisible(true);
      break;
    }
    case 1:
    {
      m_Controls->m_ResampleIntFrame->setVisible(false);
      m_Controls->m_ResampleDoubleFrame->setVisible(true);

      mitk::BaseGeometry* geom = image->GetGeometry();
      m_Controls->m_ResampleDoubleX->setValue(geom->GetSpacing()[0]);
      m_Controls->m_ResampleDoubleY->setValue(geom->GetSpacing()[1]);
      m_Controls->m_ResampleDoubleZ->setValue(geom->GetSpacing()[2]);
      break;
    }
    case 2:
    {
      m_Controls->m_ResampleIntFrame->setVisible(true);
      m_Controls->m_ResampleDoubleFrame->setVisible(false);

      mitk::BaseGeometry* geom = image->GetGeometry();
      m_Controls->m_ResampleIntX->setValue(geom->GetExtent(0));
      m_Controls->m_ResampleIntY->setValue(geom->GetExtent(1));
      m_Controls->m_ResampleIntZ->setValue(geom->GetExtent(2));
      break;
    }
    default:
    {
      m_Controls->m_ResampleIntFrame->setVisible(false);
      m_Controls->m_ResampleDoubleFrame->setVisible(true);
    }
  }
}

void QmitkPreprocessingView::DoExtractBrainMask()
{
}

void QmitkPreprocessingView::DoResampleImage()
{
  mitk::DataNode::Pointer node = m_Controls->m_SelctedImageComboBox->GetSelectedNode();
  if (node.IsNull()) { return; }

  mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(node->GetData());
  if ( image == nullptr ) { return; }

  bool isDiffusionImage( PropHelper::IsDiffusionWeightedImage(image) );
  if ( isDiffusionImage )
  {
    ItkDwiType::Pointer itkVectorImagePointer = ItkDwiType::New();
    mitk::CastToItkImage(image, itkVectorImagePointer);

    typedef itk::ResampleDwiImageFilter< short > ResampleFilter;
    ResampleFilter::Pointer resampler = ResampleFilter::New();
    resampler->SetInput( itkVectorImagePointer );

    switch (m_Controls->m_ResampleTypeBox->currentIndex())
    {
      case 0:
      {
        itk::Vector< double, 3 > samplingFactor;
        samplingFactor[0] = m_Controls->m_ResampleDoubleX->value();
        samplingFactor[1] = m_Controls->m_ResampleDoubleY->value();
        samplingFactor[2] = m_Controls->m_ResampleDoubleZ->value();
        resampler->SetSamplingFactor(samplingFactor);
        break;
      }
      case 1:
      {
        itk::Vector< double, 3 > newSpacing;
        newSpacing[0] = m_Controls->m_ResampleDoubleX->value();
        newSpacing[1] = m_Controls->m_ResampleDoubleY->value();
        newSpacing[2] = m_Controls->m_ResampleDoubleZ->value();
        resampler->SetNewSpacing(newSpacing);
        break;
      }
      case 2:
      {
        itk::ImageRegion<3> newRegion;
        newRegion.SetSize(0, m_Controls->m_ResampleIntX->value());
        newRegion.SetSize(1, m_Controls->m_ResampleIntY->value());
        newRegion.SetSize(2, m_Controls->m_ResampleIntZ->value());
        resampler->SetNewImageSize(newRegion);
        break;
      }
      default:
      {
        MITK_WARN << "Unknown resampling parameters!";
        return;
      }
    }

    QString outAdd;
    switch (m_Controls->m_InterpolatorBox->currentIndex())
    {
      case 0:
      {
        resampler->SetInterpolation(ResampleFilter::Interpolate_NearestNeighbour);
        outAdd = "NearestNeighbour";
        break;
      }
      case 1:
      {
        resampler->SetInterpolation(ResampleFilter::Interpolate_Linear);
        outAdd = "Linear";
        break;
      }
      case 2:
      {
        resampler->SetInterpolation(ResampleFilter::Interpolate_BSpline);
        outAdd = "BSpline";
        break;
      }
      case 3:
      {
        resampler->SetInterpolation(ResampleFilter::Interpolate_WindowedSinc);
        outAdd = "WindowedSinc";
        break;
      }
      default:
      {
        resampler->SetInterpolation(ResampleFilter::Interpolate_NearestNeighbour);
        outAdd = "NearestNeighbour";
      }
    }

    resampler->Update();

    mitk::Image::Pointer newImage = mitk::GrabItkImageMemory( resampler->GetOutput() );
    PropHelper::CopyProperties(image, newImage, false);
    PropHelper::InitializeImage( newImage );

    mitk::DataNode::Pointer imageNode = mitk::DataNode::New();
    imageNode->SetData( newImage );
    QString name = node->GetName().c_str();

    imageNode->SetName((name+"_resampled_"+outAdd).toStdString().c_str());
    imageNode->SetVisibility(false);
    GetDataStorage()->Add(imageNode, node);
  }
  else if( image->GetPixelType().GetNumberOfComponents() )
  {
    AccessFixedDimensionByItk(image, TemplatedResampleImage,3);
  }
  else
  {
    QMessageBox::warning(nullptr,"Warning", QString("Operation not supported in multi-component images") );
  }
}


template < typename TPixel, unsigned int VImageDimension >
void QmitkPreprocessingView::TemplatedResampleImage( itk::Image<TPixel, VImageDimension>* itkImage)
{
  mitk::DataNode::Pointer node = m_Controls->m_SelctedImageComboBox->GetSelectedNode();
  if (node.IsNull()) { return; }

  itk::Vector< double, 3 > newSpacing;
  itk::ImageRegion<3>   newRegion;

  switch (m_Controls->m_ResampleTypeBox->currentIndex())
  {
    case 0:
    {
      itk::Vector< double, 3 > sampling;
      sampling[0] = m_Controls->m_ResampleDoubleX->value();
      sampling[1] = m_Controls->m_ResampleDoubleY->value();
      sampling[2] = m_Controls->m_ResampleDoubleZ->value();

      newSpacing = itkImage->GetSpacing();
      newSpacing[0] /= sampling[0];
      newSpacing[1] /= sampling[1];
      newSpacing[2] /= sampling[2];
      newRegion = itkImage->GetLargestPossibleRegion();
      newRegion.SetSize(0, newRegion.GetSize(0)*sampling[0]);
      newRegion.SetSize(1, newRegion.GetSize(1)*sampling[1]);
      newRegion.SetSize(2, newRegion.GetSize(2)*sampling[2]);

      break;
    }
    case 1:
    {
      newSpacing[0] = m_Controls->m_ResampleDoubleX->value();
      newSpacing[1] = m_Controls->m_ResampleDoubleY->value();
      newSpacing[2] = m_Controls->m_ResampleDoubleZ->value();

      itk::Vector< double, 3 > oldSpacing = itkImage->GetSpacing();
      itk::Vector< double, 3 > sampling;
      sampling[0] = oldSpacing[0]/newSpacing[0];
      sampling[1] = oldSpacing[1]/newSpacing[1];
      sampling[2] = oldSpacing[2]/newSpacing[2];
      newRegion = itkImage->GetLargestPossibleRegion();
      newRegion.SetSize(0, newRegion.GetSize(0)*sampling[0]);
      newRegion.SetSize(1, newRegion.GetSize(1)*sampling[1]);
      newRegion.SetSize(2, newRegion.GetSize(2)*sampling[2]);
      break;
    }
    case 2:
    {
      newRegion.SetSize(0, m_Controls->m_ResampleIntX->value());
      newRegion.SetSize(1, m_Controls->m_ResampleIntY->value());
      newRegion.SetSize(2, m_Controls->m_ResampleIntZ->value());

      itk::ImageRegion<3> oldRegion = itkImage->GetLargestPossibleRegion();
      itk::Vector< double, 3 > sampling;
      sampling[0] = (double)newRegion.GetSize(0)/oldRegion.GetSize(0);
      sampling[1] = (double)newRegion.GetSize(1)/oldRegion.GetSize(1);
      sampling[2] = (double)newRegion.GetSize(2)/oldRegion.GetSize(2);

      newSpacing = itkImage->GetSpacing();
      newSpacing[0] /= sampling[0];
      newSpacing[1] /= sampling[1];
      newSpacing[2] /= sampling[2];
      break;
    }
    default:
    {
      MITK_WARN << "Unknown resampling parameters!";
      return;
    }
  }

  itk::Point<double,3> origin = itkImage->GetOrigin();
  origin[0] -= itkImage->GetSpacing()[0]/2;
  origin[1] -= itkImage->GetSpacing()[1]/2;
  origin[2] -= itkImage->GetSpacing()[2]/2;
  origin[0] += newSpacing[0]/2;
  origin[1] += newSpacing[1]/2;
  origin[2] += newSpacing[2]/2;

  typedef itk::Image<TPixel, VImageDimension> ImageType;
  typename ImageType::Pointer outImage = ImageType::New();
  outImage->SetSpacing( newSpacing );
  outImage->SetOrigin( origin );
  outImage->SetDirection( itkImage->GetDirection() );
  outImage->SetLargestPossibleRegion( newRegion );
  outImage->SetBufferedRegion( newRegion );
  outImage->SetRequestedRegion( newRegion );
  outImage->Allocate();

  typedef itk::ResampleImageFilter<ImageType, ImageType> ResampleFilter;
  typename ResampleFilter::Pointer resampler = ResampleFilter::New();
  resampler->SetInput(itkImage);
  resampler->SetOutputParametersFromImage(outImage);

  QString outAdd;
  switch (m_Controls->m_InterpolatorBox->currentIndex())
  {
    case 0:
    {
      typename itk::NearestNeighborInterpolateImageFunction<ImageType>::Pointer interp
          = itk::NearestNeighborInterpolateImageFunction<ImageType>::New();
      resampler->SetInterpolator(interp);
      outAdd = "NearestNeighbour";
      break;
    }
    case 1:
    {
      typename itk::LinearInterpolateImageFunction<ImageType>::Pointer interp
          = itk::LinearInterpolateImageFunction<ImageType>::New();
      resampler->SetInterpolator(interp);
      outAdd = "Linear";
      break;
    }
    case 2:
    {
      typename itk::BSplineInterpolateImageFunction<ImageType>::Pointer interp
          = itk::BSplineInterpolateImageFunction<ImageType>::New();
      resampler->SetInterpolator(interp);
      outAdd = "BSpline";
      break;
    }
    case 3:
    {
      typename itk::WindowedSincInterpolateImageFunction<ImageType, 3>::Pointer interp
          = itk::WindowedSincInterpolateImageFunction<ImageType, 3>::New();
      resampler->SetInterpolator(interp);
      outAdd = "WindowedSinc";
      break;
    }
    default:
    {
      typename itk::NearestNeighborInterpolateImageFunction<ImageType>::Pointer interp
          = itk::NearestNeighborInterpolateImageFunction<ImageType>::New();
      resampler->SetInterpolator(interp);
      outAdd = "NearestNeighbour";
    }
  }

  resampler->Update();

  mitk::Image::Pointer image = mitk::Image::New();
  image->InitializeByItk( resampler->GetOutput() );
  image->SetVolume( resampler->GetOutput()->GetBufferPointer() );
  mitk::DataNode::Pointer imageNode = mitk::DataNode::New();
  imageNode->SetData( image );
  QString name = node->GetName().c_str();

  imageNode->SetName((name+"_resampled_"+outAdd).toStdString().c_str());
  GetDataStorage()->Add(imageNode, node);
}

void QmitkPreprocessingView::DoApplyHeader()
{
  mitk::DataNode::Pointer node = m_Controls->m_SelctedImageComboBox->GetSelectedNode();
  if (node.IsNull()) { return; }

  mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(node->GetData());
  if ( image == nullptr ) { return; }

  bool isDiffusionImage( PropHelper::IsDiffusionWeightedImage(image) );

  bool isDti = false;
  bool isOdf = false;
  bool isPeak = false;
  if (!isDiffusionImage)
  {
    if ( dynamic_cast<mitk::TensorImage*>(node->GetData()) )
      isDti = true;
    else if ( dynamic_cast<mitk::OdfImage*>(node->GetData()) )
      isOdf = true;
    else if ( dynamic_cast<mitk::PeakImage*>(node->GetData()) )
      isPeak = true;
  }
  mitk::Image::Pointer newImage = image->Clone();

  mitk::Vector3D spacing;
  spacing[0] = m_Controls->m_HeaderSpacingX->value();
  spacing[1] = m_Controls->m_HeaderSpacingY->value();
  spacing[2] = m_Controls->m_HeaderSpacingZ->value();
  newImage->GetGeometry()->SetSpacing( spacing );

  mitk::Point3D origin;
  origin[0] = m_Controls->m_HeaderOriginX->value();
  origin[1] = m_Controls->m_HeaderOriginY->value();
  origin[2] = m_Controls->m_HeaderOriginZ->value();
  newImage->GetGeometry()->SetOrigin(origin);

  vtkSmartPointer< vtkMatrix4x4 > matrix = vtkSmartPointer< vtkMatrix4x4 >::New();
  matrix->SetElement(0,3,newImage->GetGeometry()->GetOrigin()[0]);
  matrix->SetElement(1,3,newImage->GetGeometry()->GetOrigin()[1]);
  matrix->SetElement(2,3,newImage->GetGeometry()->GetOrigin()[2]);
  for (int r=0; r<3; r++)
  {
    for (int c=0; c<3; c++)
    {
      QTableWidgetItem* item = m_Controls->m_DirectionMatrixTable->item(r,c);
      if (!item)
        continue;
      matrix->SetElement(r,c,item->text().toDouble());
    }
  }
  newImage->GetGeometry()->SetIndexToWorldTransformByVtkMatrixWithoutChangingSpacing(matrix);

  if ( isDiffusionImage )
  {
    vnl_matrix_fixed< double, 3, 3 > mf;
    for (int r=0; r<3; r++)
    {
      for (int c=0; c<3; c++)
      {
        QTableWidgetItem* item = m_Controls->m_MeasurementFrameTable->item(r,c);
        if (!item)
          continue;
        mf[r][c] = item->text().toDouble();
      }
    }
    PropHelper::SetMeasurementFrame(newImage, mf);
    PropHelper::InitializeImage( newImage );
  }

  mitk::DataNode::Pointer imageNode = mitk::DataNode::New();
  if (isOdf)
  {
    mitk::OdfImage::ItkOdfImageType::Pointer itk_img = mitk::OdfImage::ItkOdfImageType::New();
    mitk::CastToItkImage(newImage, itk_img);

    mitk::Image::Pointer odfImage = dynamic_cast<mitk::Image*>(mitk::OdfImage::New().GetPointer());
    mitk::CastToMitkImage(itk_img, odfImage);
    odfImage->SetVolume(itk_img->GetBufferPointer());
    imageNode->SetData( odfImage );
  }
  else if (isDti)
  {
    mitk::TensorImage::ItkTensorImageType::Pointer itk_img = mitk::ImageToItkImage<mitk::TensorImage::PixelType,3>(newImage);
    mitk::Image::Pointer tensorImage = dynamic_cast<mitk::Image*>(mitk::TensorImage::New().GetPointer());
    mitk::CastToMitkImage(itk_img, tensorImage);
    tensorImage->SetVolume(itk_img->GetBufferPointer());
    imageNode->SetData( tensorImage );
  }
  else if (isPeak)
  {
    mitk::PeakImage::ItkPeakImageType::Pointer itk_img = mitk::ImageToItkImage<float,4>(newImage);
    mitk::Image::Pointer peakImage = dynamic_cast<mitk::Image*>(mitk::PeakImage::New().GetPointer());
    mitk::CastToMitkImage(itk_img, peakImage);
    peakImage->SetVolume(itk_img->GetBufferPointer());
    imageNode->SetData( peakImage );
  }
  else
    imageNode->SetData( newImage );

  QString name = node->GetName().c_str();
  imageNode->SetName((name+"_newheader").toStdString().c_str());
  GetDataStorage()->Add(imageNode, node);
  mitk::RenderingManager::GetInstance()->InitializeViews( imageNode->GetData()->GetTimeGeometry(),
                                                          mitk::RenderingManager::REQUEST_UPDATE_ALL,
                                                          true );
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkPreprocessingView::DoProjectSignal()
{
  switch(m_Controls->m_ProjectionMethodBox->currentIndex())
  {
    case 0:
      DoADCAverage();
    break;
    case 1:
      DoAKCFit();
    break;
    case 2:
      DoBiExpFit();
    break;
    default:
      DoADCAverage();
  }
}

void QmitkPreprocessingView::DoDwiNormalization()
{
  mitk::DataNode::Pointer node = m_Controls->m_SelctedImageComboBox->GetSelectedNode();
  if (node.IsNull()) { return; }

  mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(node->GetData());
  if ( image == nullptr ) { return; }

  bool isDiffusionImage( PropHelper::IsDiffusionWeightedImage(image) );
  if ( ! isDiffusionImage ) { return; }

  auto gradientContainer = PropHelper::GetGradientContainer(image);

  int b0Index = -1;
  for (unsigned int i=0; i<gradientContainer->size(); i++)
  {
    GradientDirectionType g = gradientContainer->GetElement(i);
    if (g.magnitude()<0.001)
    {
      b0Index = i;
      break;
    }
  }
  if (b0Index==-1) { return; }

  typedef itk::DwiNormilzationFilter<short>  FilterType;

  ItkDwiType::Pointer itkVectorImagePointer = ItkDwiType::New();
  mitk::CastToItkImage(image, itkVectorImagePointer);

  FilterType::Pointer filter = FilterType::New();
  filter->SetInput( itkVectorImagePointer );
  filter->SetGradientDirections(PropHelper::GetGradientContainer(image));
  filter->SetNewMean(m_Controls->m_NewMean->value());
  filter->SetNewStdev(m_Controls->m_NewStdev->value());

  UcharImageType::Pointer itkImage = nullptr;
  if (m_Controls->m_NormalizationMaskBox->GetSelectedNode().IsNotNull())
  {
    itkImage = UcharImageType::New();
    if (  dynamic_cast<mitk::Image*>(m_Controls->m_NormalizationMaskBox->GetSelectedNode()->GetData()) != nullptr )
    {
      mitk::CastToItkImage( dynamic_cast<mitk::Image*>(m_Controls->m_NormalizationMaskBox->GetSelectedNode()->GetData()), itkImage );
    }
    filter->SetMaskImage(itkImage);
  }
  filter->Update();

  mitk::Image::Pointer newImage = mitk::GrabItkImageMemory( filter->GetOutput() );
  PropHelper::CopyProperties(image, newImage, false);
  PropHelper::InitializeImage( newImage );

  mitk::DataNode::Pointer imageNode = mitk::DataNode::New();
  imageNode->SetData( newImage );
  QString name = node->GetName().c_str();

  imageNode->SetName((name+"_normalized").toStdString().c_str());
  GetDataStorage()->Add(imageNode, node);
}

void QmitkPreprocessingView::DoLengthCorrection()
{
  mitk::DataNode::Pointer node = m_Controls->m_SelctedImageComboBox->GetSelectedNode();
  if (node.IsNull())
    return;

  mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(node->GetData());
  if (image == nullptr )
    return;

  if (!PropHelper::IsDiffusionWeightedImage(image))
    return;

  typedef itk::DwiGradientLengthCorrectionFilter FilterType;

  ItkDwiType::Pointer itkVectorImagePointer = PropHelper::GetItkVectorImage(image);

  FilterType::Pointer filter = FilterType::New();
  filter->SetRoundingValue( m_Controls->m_B_ValueMap_Rounder_SpinBox->value());
  filter->SetReferenceBValue(PropHelper::GetReferenceBValue(image));
  filter->SetReferenceGradientDirectionContainer(PropHelper::GetGradientContainer(image));
  filter->Update();

  mitk::Image::Pointer newImage = mitk::Image::New();
  newImage->InitializeByItk( itkVectorImagePointer.GetPointer() );
  newImage->SetImportVolume( itkVectorImagePointer->GetBufferPointer(), 0, 0, mitk::Image::CopyMemory);
  itkVectorImagePointer->GetPixelContainer()->ContainerManageMemoryOff();

  PropHelper::CopyProperties(image, newImage, true);
  PropHelper::SetGradientContainer(newImage, filter->GetOutputGradientDirectionContainer());
  PropHelper::SetReferenceBValue(newImage, filter->GetNewBValue());
  PropHelper::InitializeImage( newImage );

  mitk::DataNode::Pointer imageNode = mitk::DataNode::New();
  imageNode->SetData( newImage );
  QString name = node->GetName().c_str();

  imageNode->SetName((name+"_rounded").toStdString().c_str());
  GetDataStorage()->Add(imageNode, node);
}

void QmitkPreprocessingView::UpdateDwiBValueMapRounder(int i)
{
  mitk::DataNode::Pointer node = m_Controls->m_SelctedImageComboBox->GetSelectedNode();
  if (node.IsNull()) { return; }

  bool isDiffusionImage( PropHelper::IsDiffusionWeightedImage(node) );
  if ( ! isDiffusionImage ) { return; }

  UpdateBValueTableWidget(i);
  UpdateGradientDetails();
}

void QmitkPreprocessingView::
CallMultishellToSingleShellFilter( itk::DWIVoxelFunctor * functor,
                                   mitk::Image::Pointer image,
                                   QString imageName,
                                   mitk::DataNode* parent )
{
  typedef itk::RadialMultishellToSingleshellImageFilter<DiffusionPixelType, DiffusionPixelType> FilterType;

  // filter input parameter
  const mitk::BValueMapProperty::BValueMap& originalShellMap = PropHelper::GetBValueMap(image);

  ItkDwiType::Pointer itkVectorImagePointer = PropHelper::GetItkVectorImage(image);
  ItkDwiType* vectorImage = itkVectorImagePointer.GetPointer();
  const GradProp::GradientDirectionsContainerType::Pointer gradientContainer = PropHelper::GetGradientContainer(image);
  const unsigned int& bValue = PropHelper::GetReferenceBValue(image);

  mitk::DataNode::Pointer imageNode = 0;

  // filter call
  FilterType::Pointer filter = FilterType::New();
  filter->SetInput(vectorImage);
  filter->SetOriginalGradientDirections(gradientContainer);
  filter->SetOriginalBValueMap(originalShellMap);
  filter->SetOriginalBValue(bValue);
  filter->SetFunctor(functor);
  filter->Update();

  // create new DWI image
  mitk::Image::Pointer outImage = mitk::GrabItkImageMemory( filter->GetOutput() );
  PropHelper::CopyProperties(image, outImage, true);

  PropHelper::SetGradientContainer(outImage, filter->GetTargetGradientDirections());
  PropHelper::SetReferenceBValue(outImage, m_Controls->m_targetBValueSpinBox->value());
  PropHelper::InitializeImage( outImage );

  imageNode = mitk::DataNode::New();
  imageNode->SetData( outImage );
  imageNode->SetName(imageName.toStdString().c_str());
  GetDataStorage()->Add(imageNode, parent);
}

void QmitkPreprocessingView::DoBiExpFit()
{
  mitk::DataNode::Pointer node = m_Controls->m_SelctedImageComboBox->GetSelectedNode();
  if (node.IsNull()) { return; }

  mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(node->GetData());
  if ( image == nullptr ) { return; }

  bool isDiffusionImage( PropHelper::IsDiffusionWeightedImage(image) );
  if ( ! isDiffusionImage ) { return; }

  itk::BiExpFitFunctor::Pointer functor = itk::BiExpFitFunctor::New();

  QString name(node->GetName().c_str());

  const mitk::BValueMapProperty::BValueMap& originalShellMap = PropHelper::GetBValueMap(image);

  mitk::BValueMapProperty::BValueMap::const_iterator it = originalShellMap.begin();
  ++it;/* skip b=0*/ unsigned int s = 0; /*shell index */
  vnl_vector<double> bValueList(originalShellMap.size()-1);

  while( it != originalShellMap.end() ) { bValueList.put(s++,(it++)->first); }

  const double targetBValue = m_Controls->m_targetBValueSpinBox->value();
  functor->setListOfBValues(bValueList);
  functor->setTargetBValue(targetBValue);
  CallMultishellToSingleShellFilter(functor,image,name + "_BiExp", node);
}

void QmitkPreprocessingView::DoAKCFit()
{
  mitk::DataNode::Pointer node = m_Controls->m_SelctedImageComboBox->GetSelectedNode();
  if (node.IsNull()) { return; }

  mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(node->GetData());
  if ( image == nullptr ) { return; }

  bool isDiffusionImage( PropHelper::IsDiffusionWeightedImage(image) );
  if ( ! isDiffusionImage ) { return; }

  itk::KurtosisFitFunctor::Pointer functor = itk::KurtosisFitFunctor::New();

  QString name(node->GetName().c_str());

  const mitk::BValueMapProperty::BValueMap& originalShellMap = PropHelper::GetBValueMap(image);

  mitk::BValueMapProperty::BValueMap::const_iterator it = originalShellMap.begin();
  ++it;/* skip b=0*/ unsigned int s = 0; /*shell index */
  vnl_vector<double> bValueList(originalShellMap.size()-1);
  while(it != originalShellMap.end())
    bValueList.put(s++,(it++)->first);

  const double targetBValue = m_Controls->m_targetBValueSpinBox->value();
  functor->setListOfBValues(bValueList);
  functor->setTargetBValue(targetBValue);
  CallMultishellToSingleShellFilter(functor,image,name + "_AKC", node);
}

void QmitkPreprocessingView::DoADCFit()
{
  // later
}

void QmitkPreprocessingView::DoADCAverage()
{
  mitk::DataNode::Pointer node = m_Controls->m_SelctedImageComboBox->GetSelectedNode();
  if (node.IsNull()) { return; }
  mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(node->GetData());
  if ( image == nullptr ) { return; }

  bool isDiffusionImage( PropHelper::IsDiffusionWeightedImage(image) );
  if ( ! isDiffusionImage )
    return;

  itk::ADCAverageFunctor::Pointer functor = itk::ADCAverageFunctor::New();

  QString name(node->GetName().c_str());

  const mitk::BValueMapProperty::BValueMap &originalShellMap  = PropHelper::GetBValueMap(image);

  mitk::BValueMapProperty::BValueMap::const_iterator it = originalShellMap.begin();
  ++it;/* skip b=0*/ unsigned int s = 0; /*shell index */
  vnl_vector<double> bValueList(originalShellMap.size()-1);
  while(it != originalShellMap.end())
    bValueList.put(s++,(it++)->first);

  const double targetBValue = m_Controls->m_targetBValueSpinBox->value();
  functor->setListOfBValues(bValueList);
  functor->setTargetBValue(targetBValue);
  CallMultishellToSingleShellFilter(functor,image,name + "_ADC", node);
}

void QmitkPreprocessingView::CleanBValueTableWidget()
{
  m_Controls->m_B_ValueMap_TableWidget->clear();
  m_Controls->m_B_ValueMap_TableWidget->setRowCount(1);
  QStringList headerList;
  headerList << "b-Value" << "Number of gradients";
  m_Controls->m_B_ValueMap_TableWidget->setHorizontalHeaderLabels(headerList);
  m_Controls->m_B_ValueMap_TableWidget->setItem(0,0,new QTableWidgetItem("-"));
  m_Controls->m_B_ValueMap_TableWidget->setItem(0,1,new QTableWidgetItem("-"));
}

void QmitkPreprocessingView::UpdateGradientDetails()
{
  mitk::DataNode::Pointer node = m_Controls->m_SelctedImageComboBox->GetSelectedNode();
  mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(node->GetData());
  if ( image == nullptr ) { return; }

  bool isDiffusionImage(false);

  isDiffusionImage = PropHelper::IsDiffusionWeightedImage(image);

  if ( ! isDiffusionImage )
  {
    m_Controls->m_WorkingGradientsText->clear();
    m_Controls->m_OriginalGradientsText->clear();
    return;
  }

  auto gradientContainer = PropHelper::GetGradientContainer(image);
  QString text = "";
  for (unsigned int j=0; j<gradientContainer->Size(); j++)
  {
    auto g = gradientContainer->at(j);
    g = g.normalize();
    text += QString::number(g[0]);
    if (j<gradientContainer->Size()-1)
      text += " ";
    else
      text += "\n";
  }
  for (unsigned int j=0; j<gradientContainer->Size(); j++)
  {
    auto g = gradientContainer->at(j);
    g = g.normalize();
    text += QString::number(g[1]);
    if (j<gradientContainer->Size()-1)
      text += " ";
    else
      text += "\n";
  }
  for (unsigned int j=0; j<gradientContainer->Size(); j++)
  {
    auto g = gradientContainer->at(j);
    g = g.normalize();
    text += QString::number(g[2]);
    if (j<gradientContainer->Size()-1)
      text += " ";
    else
      text += "\n";
  }

  m_Controls->m_WorkingGradientsText->setPlainText(text);

  gradientContainer = PropHelper::GetOriginalGradientContainer(image);
  text = "";
  for (unsigned int j=0; j<gradientContainer->Size(); j++)
  {
    auto g = gradientContainer->at(j);
    g = g.normalize();
    text += QString::number(g[0]);
    if (j<gradientContainer->Size()-1)
      text += " ";
    else
      text += "\n";
  }
  for (unsigned int j=0; j<gradientContainer->Size(); j++)
  {
    auto g = gradientContainer->at(j);
    g = g.normalize();
    text += QString::number(g[1]);
    if (j<gradientContainer->Size()-1)
      text += " ";
    else
      text += "\n";
  }
  for (unsigned int j=0; j<gradientContainer->Size(); j++)
  {
    auto g = gradientContainer->at(j);
    g = g.normalize();
    text += QString::number(g[2]);
    if (j<gradientContainer->Size()-1)
      text += " ";
    else
      text += "\n";
  }
  m_Controls->m_OriginalGradientsText->setPlainText(text);
}

void QmitkPreprocessingView::UpdateBValueTableWidget(int)
{
  mitk::DataNode::Pointer node = m_Controls->m_SelctedImageComboBox->GetSelectedNode();

  if (node.IsNull())
  {
    CleanBValueTableWidget();
    return;
  }

  mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(node->GetData());
  if ( image == nullptr ) { return; }

  bool isDiffusionImage(false);

  isDiffusionImage = PropHelper::IsDiffusionWeightedImage(image);

  if ( ! isDiffusionImage )
  {
    CleanBValueTableWidget();
  }
  else
  {
    typedef mitk::BValueMapProperty::BValueMap BValueMap;
    typedef mitk::BValueMapProperty::BValueMap::iterator BValueMapIterator;

    BValueMapIterator it;

    BValueMap roundedBValueMap = PropHelper::GetBValueMap(image);

    m_Controls->m_B_ValueMap_TableWidget->clear();
    m_Controls->m_B_ValueMap_TableWidget->setRowCount(roundedBValueMap.size() );
    QStringList headerList;
    headerList << "b-Value" << "Number of gradients";
    m_Controls->m_B_ValueMap_TableWidget->setHorizontalHeaderLabels(headerList);

    int i = 0 ;
    for(it = roundedBValueMap.begin() ;it != roundedBValueMap.end(); it++)
    {
      m_Controls->m_B_ValueMap_TableWidget->setItem(i,0,new QTableWidgetItem(QString::number(it->first)));
      QTableWidgetItem* item = m_Controls->m_B_ValueMap_TableWidget->item(i,0);
      item->setFlags(item->flags() & ~Qt::ItemIsEditable);
      m_Controls->m_B_ValueMap_TableWidget->setItem(i,1,new QTableWidgetItem(QString::number(it->second.size())));
      i++;
    }
  }
}

void QmitkPreprocessingView::OnSelectionChanged(berry::IWorkbenchPart::Pointer , const QList<mitk::DataNode::Pointer>& nodes)
{
  (void) nodes;
  this->OnImageSelectionChanged();
}

void QmitkPreprocessingView::OnImageSelectionChanged()
{
  for (int r=0; r<3; r++)
    for (int c=0; c<3; c++)
    {
      {
        QTableWidgetItem* item = m_Controls->m_MeasurementFrameTable->item( r, c );
        delete item;
        item = new QTableWidgetItem();
        m_Controls->m_MeasurementFrameTable->setItem( r, c, item );
      }
      {
        QTableWidgetItem* item = m_Controls->m_DirectionMatrixTable->item( r, c );
        delete item;
        item = new QTableWidgetItem();
        m_Controls->m_DirectionMatrixTable->setItem( r, c, item );
      }
    }

  bool foundImageVolume = true;
  mitk::DataNode::Pointer node = m_Controls->m_SelctedImageComboBox->GetSelectedNode();
  if (node.IsNull()) { return; }

  bool foundDwiVolume( PropHelper::IsDiffusionWeightedImage( node ) );

  mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(node->GetData());
  if ( image == nullptr ) { return; }

  bool multiComponentVolume = (image->GetPixelType().GetNumberOfComponents() > 1);
  bool threeDplusTVolume = (image->GetTimeSteps() > 1);

  m_Controls->m_ButtonAverageGradients->setEnabled(foundDwiVolume);
  m_Controls->m_ButtonExtractB0->setEnabled(foundDwiVolume);
  m_Controls->m_CheckExtractAll->setEnabled(foundDwiVolume);
  m_Controls->m_MeasurementFrameTable->setEnabled(foundDwiVolume);
  m_Controls->m_ReduceGradientsButton->setEnabled(foundDwiVolume);
  m_Controls->m_ShowGradientsButton->setEnabled(foundDwiVolume);
  m_Controls->m_MirrorGradientToHalfSphereButton->setEnabled(foundDwiVolume);
  m_Controls->m_MergeDwisButton->setEnabled(foundDwiVolume);
  m_Controls->m_B_ValueMap_Rounder_SpinBox->setEnabled(foundDwiVolume);
  m_Controls->m_ProjectSignalButton->setEnabled(foundDwiVolume);
  m_Controls->m_CreateLengthCorrectedDwi->setEnabled(foundDwiVolume);
  m_Controls->m_targetBValueSpinBox->setEnabled(foundDwiVolume);
  m_Controls->m_NormalizeImageValuesButton->setEnabled(foundDwiVolume);
  m_Controls->m_RemoveGradientButton->setEnabled(foundDwiVolume);
  m_Controls->m_ExtractGradientButton->setEnabled(foundDwiVolume);
  m_Controls->m_FlipGradientsButton->setEnabled(foundDwiVolume);
  m_Controls->m_ClearRotationButton->setEnabled(foundDwiVolume);

  m_Controls->m_DirectionMatrixTable->setEnabled(foundImageVolume);
  m_Controls->m_ModifyHeader->setEnabled(foundImageVolume);
  m_Controls->m_AlignImageBox->setEnabled(foundImageVolume);

  // we do not support multi-component and 3D+t images for certain operations
  bool foundSingleImageVolume = foundDwiVolume || (foundImageVolume && !(multiComponentVolume || threeDplusTVolume));
  m_Controls->m_FlipAxis->setEnabled(foundSingleImageVolume);
  m_Controls->m_CropImageButton->setEnabled(foundSingleImageVolume);
  m_Controls->m_ExtractBrainMask->setEnabled(foundSingleImageVolume);
  m_Controls->m_ResampleImageButton->setEnabled(foundSingleImageVolume);

  // reset sampling frame to 1 and update all ealted components
  m_Controls->m_B_ValueMap_Rounder_SpinBox->setValue(1);

  UpdateBValueTableWidget(m_Controls->m_B_ValueMap_Rounder_SpinBox->value());
  DoUpdateInterpolationGui(m_Controls->m_ResampleTypeBox->currentIndex());
  UpdateGradientDetails();

  m_Controls->m_HeaderSpacingX->setValue(image->GetGeometry()->GetSpacing()[0]);
  m_Controls->m_HeaderSpacingY->setValue(image->GetGeometry()->GetSpacing()[1]);
  m_Controls->m_HeaderSpacingZ->setValue(image->GetGeometry()->GetSpacing()[2]);
  m_Controls->m_HeaderOriginX->setValue(image->GetGeometry()->GetOrigin()[0]);
  m_Controls->m_HeaderOriginY->setValue(image->GetGeometry()->GetOrigin()[1]);
  m_Controls->m_HeaderOriginZ->setValue(image->GetGeometry()->GetOrigin()[2]);
  m_Controls->m_XstartBox->setMaximum(image->GetGeometry()->GetExtent(0)-1);
  m_Controls->m_YstartBox->setMaximum(image->GetGeometry()->GetExtent(1)-1);
  m_Controls->m_ZstartBox->setMaximum(image->GetGeometry()->GetExtent(2)-1);
  m_Controls->m_XendBox->setMaximum(image->GetGeometry()->GetExtent(0)-1);
  m_Controls->m_YendBox->setMaximum(image->GetGeometry()->GetExtent(1)-1);
  m_Controls->m_ZendBox->setMaximum(image->GetGeometry()->GetExtent(2)-1);

  for (int r=0; r<3; r++)
    for (int c=0; c<3; c++)
    {
      // Direction matrix
      QTableWidgetItem* item = m_Controls->m_DirectionMatrixTable->item( r, c );
      delete item;
      item = new QTableWidgetItem();
      item->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
      double val = image->GetGeometry()->GetVtkMatrix()->GetElement(r,c)/image->GetGeometry()->GetSpacing()[c];
      item->setText(QString::number(val));
      m_Controls->m_DirectionMatrixTable->setItem( r, c, item );
    }

  if (foundDwiVolume)
  {
    m_Controls->m_InputData->setTitle("Input Data");
    vnl_matrix_fixed< double, 3, 3 >  mf = PropHelper::GetMeasurementFrame(image);

    for (int r=0; r<3; r++)
      for (int c=0; c<3; c++)
      {
        // Measurement frame
        QTableWidgetItem* item = m_Controls->m_MeasurementFrameTable->item( r, c );
        delete item;
        item = new QTableWidgetItem();
        item->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
        item->setText(QString::number(mf.get(r,c)));
        m_Controls->m_MeasurementFrameTable->setItem( r, c, item );
      }

    //calculate target bValue for MultishellToSingleShellfilter
    const mitk::BValueMapProperty::BValueMap & bValMap = PropHelper::GetBValueMap(image);

    mitk::BValueMapProperty::BValueMap::const_iterator it = bValMap.begin();
    unsigned int targetBVal = 0;
    while(it != bValMap.end()) { targetBVal += (it++)->first; }

    targetBVal /= (float)bValMap.size()-1;
    m_Controls->m_targetBValueSpinBox->setValue(targetBVal);

    m_Controls->m_RemoveGradientBox->setMaximum(PropHelper::GetGradientContainer(image)->Size()-1);
    m_Controls->m_ExtractGradientBox->setMaximum(PropHelper::GetGradientContainer(image)->Size()-1);
  }

}


void QmitkPreprocessingView::Activated()
{
}

void QmitkPreprocessingView::Deactivated()
{
  OnImageSelectionChanged();
}

void QmitkPreprocessingView::Visible()
{
  OnImageSelectionChanged();
}

void QmitkPreprocessingView::Hidden()
{
}

void QmitkPreprocessingView::DoClearRotationOfGradients()
{

  mitk::DataNode::Pointer node = m_Controls->m_SelctedImageComboBox->GetSelectedNode();
  if (node.IsNull())
    return;

  mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(node->GetData());
  if (image == nullptr)
    return;

  if(!PropHelper::IsDiffusionWeightedImage(image))
    return;

  mitk::Image::Pointer newDwi = image->Clone();

  PropHelper::SetApplyMatrixToGradients(newDwi, false);

  mitk::DataNode::Pointer imageNode = mitk::DataNode::New();
  imageNode->SetData( newDwi );

  QString name = node->GetName().c_str();
  imageNode->SetName( (name+"_OriginalGradients").toStdString().c_str() );
  GetDataStorage()->Add( imageNode, node );

}

void QmitkPreprocessingView::DoFlipGradientDirections()
{
  mitk::DataNode::Pointer node = m_Controls->m_SelctedImageComboBox->GetSelectedNode();
  if (node.IsNull()) { return; }

  mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(node->GetData());
  if ( image == nullptr ) { return; }

  mitk::Image::Pointer newDwi = image->Clone();

  auto gradientContainer = PropHelper::GetGradientContainer(newDwi);

  for (unsigned int j=0; j<gradientContainer->Size(); j++)
  {
      if (m_Controls->m_FlipGradBoxX->isChecked()) { gradientContainer->at(j)[0] *= -1; }
      if (m_Controls->m_FlipGradBoxY->isChecked()) { gradientContainer->at(j)[1] *= -1; }
      if (m_Controls->m_FlipGradBoxZ->isChecked()) { gradientContainer->at(j)[2] *= -1; }
  }

  PropHelper::CopyProperties(image, newDwi, true);
  PropHelper::SetGradientContainer(newDwi, gradientContainer);
  PropHelper::InitializeImage( newDwi );

  mitk::DataNode::Pointer imageNode = mitk::DataNode::New();
  imageNode->SetData( newDwi );

  QString name = node->GetName().c_str();
  imageNode->SetName( (name+"_GradientFlip").toStdString().c_str() );
  GetDataStorage()->Add( imageNode, node );
}

void QmitkPreprocessingView::DoHalfSphereGradientDirections()
{
  mitk::DataNode::Pointer node = m_Controls->m_SelctedImageComboBox->GetSelectedNode();
  if (node.IsNull()) { return; }

  mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(node->GetData());
  if ( image == nullptr ) { return; }

  mitk::Image::Pointer newDwi = image->Clone();
  auto gradientContainer = PropHelper::GetGradientContainer(newDwi);

  for (unsigned int j=0; j<gradientContainer->Size(); j++)
  {
    if (gradientContainer->at(j)[0]<0) { gradientContainer->at(j) = -gradientContainer->at(j); }
  }

  PropHelper::CopyProperties(image, newDwi, true);
  PropHelper::SetGradientContainer(newDwi, gradientContainer);
  PropHelper::InitializeImage( newDwi );

  mitk::DataNode::Pointer imageNode = mitk::DataNode::New();
  imageNode->SetData( newDwi );

  QString name = node->GetName().c_str();
  imageNode->SetName( (name+"_halfsphere").toStdString().c_str() );
  GetDataStorage()->Add( imageNode, node );
}

void QmitkPreprocessingView::DoShowGradientDirections()
{
  mitk::DataNode::Pointer node = m_Controls->m_SelctedImageComboBox->GetSelectedNode();
  if (node.IsNull()) { return; }

  mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(node->GetData());
  if ( image == nullptr ) { return; }

  bool isDiffusionImage( PropHelper::IsDiffusionWeightedImage(image) );
  if ( !isDiffusionImage ) { return; }

  int maxIndex = 0;
  unsigned int maxSize = image->GetDimension(0);
  if (maxSize<image->GetDimension(1))
  {
    maxSize = image->GetDimension(1);
    maxIndex = 1;
  }
  if (maxSize<image->GetDimension(2))
  {
    maxSize = image->GetDimension(2);
    maxIndex = 2;
  }

  mitk::Point3D origin = image->GetGeometry()->GetOrigin();
  mitk::PointSet::Pointer originSet = mitk::PointSet::New();

  typedef mitk::BValueMapProperty::BValueMap BValueMap;
  typedef mitk::BValueMapProperty::BValueMap::iterator BValueMapIterator;
  BValueMap bValMap = PropHelper::GetBValueMap(image);
  auto gradientContainer = PropHelper::GetGradientContainer(image);

  mitk::BaseGeometry::Pointer geometry = image->GetGeometry();
  int shellCount = 1;

  for(BValueMapIterator it = bValMap.begin(); it!=bValMap.end(); ++it)
  {
    mitk::PointSet::Pointer pointset = mitk::PointSet::New();
    for (unsigned int j=0; j<it->second.size(); j++)
    {
      mitk::Point3D ip;
      vnl_vector_fixed< double, 3 > v = gradientContainer->at(it->second[j]);
      if (v.magnitude()>mitk::eps)
      {
        ip[0] = v[0]*maxSize*geometry->GetSpacing()[maxIndex]/2
                + origin[0]-0.5*geometry->GetSpacing()[0]
                + geometry->GetSpacing()[0]*image->GetDimension(0)/2;

        ip[1] = v[1]*maxSize*geometry->GetSpacing()[maxIndex]/2
                + origin[1]-0.5*geometry->GetSpacing()[1]
                + geometry->GetSpacing()[1]*image->GetDimension(1)/2;

        ip[2] = v[2]*maxSize*geometry->GetSpacing()[maxIndex]/2
                + origin[2]-0.5*geometry->GetSpacing()[2]
                + geometry->GetSpacing()[2]*image->GetDimension(2)/2;

        pointset->InsertPoint(j, ip);
      }
      else if (originSet->IsEmpty())
      {
        ip[0] = v[0]*maxSize*geometry->GetSpacing()[maxIndex]/2
                + origin[0]-0.5*geometry->GetSpacing()[0]
                + geometry->GetSpacing()[0]*image->GetDimension(0)/2;

        ip[1] = v[1]*maxSize*geometry->GetSpacing()[maxIndex]/2
                + origin[1]-0.5*geometry->GetSpacing()[1]
                + geometry->GetSpacing()[1]*image->GetDimension(1)/2;

        ip[2] = v[2]*maxSize*geometry->GetSpacing()[maxIndex]/2
                + origin[2]-0.5*geometry->GetSpacing()[2]
                + geometry->GetSpacing()[2]*image->GetDimension(2)/2;

        originSet->InsertPoint(j, ip);
      }
    }
    if ( it->first < mitk::eps ) { continue; }

    // add shell to datastorage
    mitk::DataNode::Pointer newNode = mitk::DataNode::New();
    newNode->SetData(pointset);

    QString name = node->GetName().c_str();
    name += "_Shell_";
    name += QString::number( it->first );
    newNode->SetName( name.toStdString().c_str() );
    newNode->SetProperty( "pointsize", mitk::FloatProperty::New((float)maxSize / 50) );

    int b0 = shellCount % 2;
    int b1 = 0;
    int b2 = 0;
    if (shellCount>4) { b2 = 1; }

    if (shellCount%4 >= 2) { b1 = 1; }

    newNode->SetProperty("color", mitk::ColorProperty::New( b2, b1, b0 ));
    GetDataStorage()->Add( newNode, node );
    shellCount++;
  }

  // add origin to datastorage
  mitk::DataNode::Pointer newNode = mitk::DataNode::New();
  newNode->SetData(originSet);

  QString name = node->GetName().c_str();
  name += "_Origin";
  newNode->SetName(name.toStdString().c_str());
  newNode->SetProperty("pointsize", mitk::FloatProperty::New((float)maxSize/50));
  newNode->SetProperty("color", mitk::ColorProperty::New(1,1,1));
  GetDataStorage()->Add(newNode, node);
}

void QmitkPreprocessingView::DoReduceGradientDirections()
{
  mitk::DataNode::Pointer node = m_Controls->m_SelctedImageComboBox->GetSelectedNode();
  if (node.IsNull()) { return; }

  mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(node->GetData());
  if ( image == nullptr ) { return; }

  bool isDiffusionImage( PropHelper::IsDiffusionWeightedImage(image) );
  if ( !isDiffusionImage ) { return; }

  typedef itk::ElectrostaticRepulsionDiffusionGradientReductionFilter<DiffusionPixelType, DiffusionPixelType> FilterType;
  typedef mitk::BValueMapProperty::BValueMap BValueMap;

  // GetShellSelection from GUI
  BValueMap shellSlectionMap;
  BValueMap originalShellMap = PropHelper::GetBValueMap(image);

  std::vector<unsigned int> newNumGradientDirections;
  int shellCounter = 0;

  QString name = node->GetName().c_str();
  for (int i=0; i<m_Controls->m_B_ValueMap_TableWidget->rowCount(); i++)
  {
    double BValue = m_Controls->m_B_ValueMap_TableWidget->item(i,0)->text().toDouble();
    shellSlectionMap[BValue] = originalShellMap[BValue];
    unsigned int num = m_Controls->m_B_ValueMap_TableWidget->item(i,1)->text().toUInt();
    newNumGradientDirections.push_back(num);
    name += "_";
    name += QString::number(num);
    shellCounter++;
  }

  if (newNumGradientDirections.empty()) { return; }

  auto gradientContainer = PropHelper::GetGradientContainer(image);

  ItkDwiType::Pointer itkVectorImagePointer = ItkDwiType::New();
  mitk::CastToItkImage(image, itkVectorImagePointer);

  FilterType::Pointer filter = FilterType::New();
  filter->SetInput( itkVectorImagePointer );
  filter->SetOriginalGradientDirections(gradientContainer);
  filter->SetNumGradientDirections(newNumGradientDirections);
  filter->SetOriginalBValueMap(originalShellMap);
  filter->SetShellSelectionBValueMap(shellSlectionMap);
  filter->SetUseFirstN(m_Controls->m_KeepFirstNBox->isChecked());
  filter->Update();

  mitk::Image::Pointer newImage = mitk::GrabItkImageMemory( filter->GetOutput() );
  PropHelper::CopyProperties(image, newImage, true);
  PropHelper::SetGradientContainer(newImage, filter->GetGradientDirections());
  PropHelper::InitializeImage(newImage);

  mitk::DataNode::Pointer imageNode = mitk::DataNode::New();
  imageNode->SetData( newImage );

  imageNode->SetName(name.toStdString().c_str());
  GetDataStorage()->Add(imageNode, node);

  // update the b-value widget to remove the modified number of gradients used for extraction
  this->CleanBValueTableWidget();
  this->UpdateBValueTableWidget(0);
  this->UpdateGradientDetails();
}

void QmitkPreprocessingView::MergeDwis()
{
  typedef GradProp::GradientDirectionsContainerType GradientContainerType;

  mitk::DataNode::Pointer node = m_Controls->m_SelctedImageComboBox->GetSelectedNode();
  if (node.IsNull()) { return; }

  mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(node->GetData());
  if ( image == nullptr ) { return; }

  bool isDiffusionImage( PropHelper::IsDiffusionWeightedImage(image) );
  if ( !isDiffusionImage ) { return; }

  mitk::DataNode::Pointer node2 = m_Controls->m_MergeDwiBox->GetSelectedNode();
  if (node2.IsNull()) { return; }

  mitk::Image::Pointer image2 = dynamic_cast<mitk::Image*>(node2->GetData());
  if ( image2 == nullptr ) { return; }

  typedef itk::VectorImage<DiffusionPixelType,3> DwiImageType;
  typedef std::vector< DwiImageType::Pointer >   DwiImageContainerType;

  typedef std::vector< GradientContainerType::Pointer >  GradientListContainerType;

  DwiImageContainerType       imageContainer;
  GradientListContainerType   gradientListContainer;
  std::vector< double >       bValueContainer;

  QString name = "";
  {
    ItkDwiType::Pointer itkVectorImagePointer = ItkDwiType::New();
    mitk::CastToItkImage(image, itkVectorImagePointer);

    imageContainer.push_back( itkVectorImagePointer );
    gradientListContainer.push_back(PropHelper::GetGradientContainer(image));
    bValueContainer.push_back(PropHelper::GetReferenceBValue(image));

    name += node->GetName().c_str();
  }

  {
    ItkDwiType::Pointer itkVectorImagePointer = ItkDwiType::New();
    mitk::CastToItkImage(image2, itkVectorImagePointer);

    imageContainer.push_back( itkVectorImagePointer );
    gradientListContainer.push_back(PropHelper::GetGradientContainer(image2));
    bValueContainer.push_back(PropHelper::GetReferenceBValue(image2));

    name += "+";
    name += node2->GetName().c_str();
  }

  typedef itk::MergeDiffusionImagesFilter<short> FilterType;
  FilterType::Pointer filter = FilterType::New();
  filter->SetImageVolumes(imageContainer);
  filter->SetGradientLists(gradientListContainer);
  filter->SetBValues(bValueContainer);
  filter->Update();

  vnl_matrix_fixed< double, 3, 3 > mf; mf.set_identity();
  mitk::Image::Pointer newImage = mitk::GrabItkImageMemory( filter->GetOutput() );

  PropHelper::SetGradientContainer(newImage, filter->GetOutputGradients());
  PropHelper::SetMeasurementFrame(newImage, mf);
  PropHelper::SetReferenceBValue(newImage, filter->GetB_Value());
  PropHelper::InitializeImage( newImage );

  mitk::DataNode::Pointer imageNode = mitk::DataNode::New();
  imageNode->SetData( newImage );
  imageNode->SetName(name.toStdString().c_str());
  GetDataStorage()->Add(imageNode);
}

void QmitkPreprocessingView::ExtractB0()
{
  mitk::DataNode::Pointer node = m_Controls->m_SelctedImageComboBox->GetSelectedNode();
  if (node.IsNull()) { return; }

  mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(node->GetData());
  if ( image == nullptr ) { return; }

  bool isDiffusionImage( PropHelper::IsDiffusionWeightedImage(image) );
  if ( !isDiffusionImage ) { return; }

  // call the extraction withou averaging if the check-box is checked
  if( this->m_Controls->m_CheckExtractAll->isChecked() )
  {
    DoExtractBOWithoutAveraging();
    return;
  }

  ItkDwiType::Pointer itkVectorImagePointer = ItkDwiType::New();
  mitk::CastToItkImage(image, itkVectorImagePointer);

  // Extract image using found index
  typedef itk::B0ImageExtractionImageFilter<short, short> FilterType;
  FilterType::Pointer filter = FilterType::New();
  filter->SetInput( itkVectorImagePointer );
  filter->SetDirections(PropHelper::GetGradientContainer(image));

  filter->Update();

  mitk::Image::Pointer mitkImage = mitk::Image::New();
  mitkImage->InitializeByItk( filter->GetOutput() );
  mitkImage->SetVolume( filter->GetOutput()->GetBufferPointer() );
  mitk::DataNode::Pointer newNode=mitk::DataNode::New();
  newNode->SetData( mitkImage );
  newNode->SetProperty( "name", mitk::StringProperty::New(node->GetName() + "_B0"));

  GetDataStorage()->Add(newNode, node);
}

void QmitkPreprocessingView::DoExtractBOWithoutAveraging()
{
  mitk::DataNode::Pointer node = m_Controls->m_SelctedImageComboBox->GetSelectedNode();
  if (node.IsNull()) { return; }

  mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(node->GetData());
  if ( image == nullptr ) { return; }

  bool isDiffusionImage( PropHelper::IsDiffusionWeightedImage(image) );
  if ( !isDiffusionImage ) { return; }

  // typedefs
  typedef itk::B0ImageExtractionToSeparateImageFilter< short, short> FilterType;

  ItkDwiType::Pointer itkVectorImagePointer = ItkDwiType::New();
  mitk::CastToItkImage(image, itkVectorImagePointer);

  // Extract image using found index
  FilterType::Pointer filter = FilterType::New();
  filter->SetInput( itkVectorImagePointer );
  filter->SetDirections(PropHelper::GetGradientContainer(image));

  filter->Update();

  mitk::Image::Pointer mitkImage = mitk::Image::New();
  mitkImage->InitializeByItk( filter->GetOutput() );
  mitkImage->SetImportChannel( filter->GetOutput()->GetBufferPointer() );
  mitk::DataNode::Pointer newNode=mitk::DataNode::New();
  newNode->SetData( mitkImage );
  newNode->SetProperty( "name", mitk::StringProperty::New(node->GetName() + "_B0_ALL"));

  GetDataStorage()->Add(newNode, node);

  /*A reinitialization is needed to access the time channels via the ImageNavigationController
    The Global-Geometry can not recognize the time channel without a re-init.
    (for a new selection in datamanger a automatically updated of the Global-Geometry should be done
    - if it contains the time channel)*/
  mitk::RenderingManager::GetInstance()->InitializeViews( newNode->GetData()->GetTimeGeometry(),
                                                          mitk::RenderingManager::REQUEST_UPDATE_ALL,
                                                          true);
}

void QmitkPreprocessingView::AverageGradients()
{
  mitk::DataNode::Pointer node = m_Controls->m_SelctedImageComboBox->GetSelectedNode();
  if (node.IsNull()) { return; }

  mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(node->GetData());
  if ( image == nullptr ) { return; }

  bool isDiffusionImage( PropHelper::IsDiffusionWeightedImage(image) );
  if ( !isDiffusionImage )
    return;

  mitk::Image::Pointer newDwi = image->Clone();
  PropHelper::AverageRedundantGradients(newDwi, m_Controls->m_Blur->value());
  PropHelper::InitializeImage(newDwi);

  mitk::DataNode::Pointer imageNode = mitk::DataNode::New();
  imageNode->SetData( newDwi );
  QString name = node->GetName().c_str();
  imageNode->SetName((name+"_averaged").toStdString().c_str());
  GetDataStorage()->Add(imageNode, node);
}
