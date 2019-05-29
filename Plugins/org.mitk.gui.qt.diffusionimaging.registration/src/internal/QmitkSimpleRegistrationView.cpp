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

//misc
#define _USE_MATH_DEFINES
#include <math.h>

// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>

// Qmitk
#include "QmitkSimpleRegistrationView.h"

// MITK
#include <mitkNodePredicateDataType.h>
#include <mitkDiffusionPropertyHelper.h>
#include <mitkImageMappingHelper.h>
#include <mitkITKImageImport.h>
#include <mitkImageCast.h>
#include <itkExtractDwiChannelFilter.h>
#include <mitkMultiModalRigidDefaultRegistrationAlgorithm.h>
#include <mitkMultiModalAffineDefaultRegistrationAlgorithm.h>
#include <itkComposeImageFilter.h>
#include <mitkFiberBundle.h>
#include <mitkRegistrationHelper.h>
#include <mitkDiffusionImageCorrectionFilter.h>

// Qt
#include <QThreadPool>

#define _USE_MATH_DEFINES
#include <math.h>

const std::string QmitkSimpleRegistrationView::VIEW_ID = "org.mitk.views.simpleregistrationview";

QmitkSimpleRegistrationView::QmitkSimpleRegistrationView()
  : QmitkAbstractView()
  , m_Controls( 0 )
  , m_RegistrationType(0)
{

}

// Destructor
QmitkSimpleRegistrationView::~QmitkSimpleRegistrationView()
{

}

void QmitkSimpleRegistrationView::StartRegistration()
{
  QmitkRegistrationJob* pJob;

  if (m_Controls->m_RegBox->currentIndex()==0)
  {
    mitk::MultiModalRigidDefaultRegistrationAlgorithm< ItkFloatImageType >::Pointer algo = mitk::MultiModalRigidDefaultRegistrationAlgorithm< ItkFloatImageType >::New();
    pJob = new QmitkRegistrationJob(algo);
    m_RegistrationType = 0;
  }
  else
  {
    mitk::MultiModalAffineDefaultRegistrationAlgorithm< ItkFloatImageType >::Pointer algo = mitk::MultiModalAffineDefaultRegistrationAlgorithm< ItkFloatImageType >::New();
    pJob = new QmitkRegistrationJob(algo);
    m_RegistrationType = 1;
  }

  pJob->setAutoDelete(true);

  m_MovingImageNode = m_Controls->m_MovingImageBox->GetSelectedNode();
  mitk::Image::Pointer movingImage = dynamic_cast<mitk::Image*>(m_MovingImageNode->GetData());
  if (mitk::DiffusionPropertyHelper::IsDiffusionWeightedImage(movingImage))
  {
    ItkDwiType::Pointer itkVectorImagePointer = ItkDwiType::New();
    mitk::CastToItkImage(movingImage, itkVectorImagePointer);

    itk::ExtractDwiChannelFilter< short >::Pointer filter = itk::ExtractDwiChannelFilter< short >::New();
    filter->SetInput( itkVectorImagePointer);
    filter->SetChannelIndex(m_Controls->m_MovingChannelBox->value());
    filter->Update();

    mitk::Image::Pointer newImage = mitk::Image::New();
    newImage->InitializeByItk( filter->GetOutput() );
    newImage->SetImportChannel( filter->GetOutput()->GetBufferPointer() );

    pJob->m_spMovingData = newImage;
  }
  else
    pJob->m_spMovingData = movingImage;


  mitk::Image::Pointer fixedImage = dynamic_cast<mitk::Image*>(m_Controls->m_FixedImageBox->GetSelectedNode()->GetData());
  if (mitk::DiffusionPropertyHelper::IsDiffusionWeightedImage(fixedImage))
  {
    ItkDwiType::Pointer itkVectorImagePointer = ItkDwiType::New();
    mitk::CastToItkImage(fixedImage, itkVectorImagePointer);

    itk::ExtractDwiChannelFilter< short >::Pointer filter = itk::ExtractDwiChannelFilter< short >::New();
    filter->SetInput( itkVectorImagePointer);
    filter->SetChannelIndex(m_Controls->m_MovingChannelBox->value());
    filter->Update();

    mitk::Image::Pointer newImage = mitk::Image::New();
    newImage->InitializeByItk( filter->GetOutput() );
    newImage->SetImportChannel( filter->GetOutput()->GetBufferPointer() );

    pJob->m_spTargetData = newImage;
  }
  else
    pJob->m_spTargetData = fixedImage;

  pJob->m_TargetDataUID = mitk::EnsureUID(m_Controls->m_FixedImageBox->GetSelectedNode()->GetData());
  pJob->m_MovingDataUID = mitk::EnsureUID(m_Controls->m_MovingImageBox->GetSelectedNode()->GetData());

  connect(pJob, SIGNAL(RegResultIsAvailable(mitk::MAPRegistrationWrapper::Pointer, const QmitkRegistrationJob*)), this,
          SLOT(OnRegResultIsAvailable(mitk::MAPRegistrationWrapper::Pointer, const QmitkRegistrationJob*)),
          Qt::BlockingQueuedConnection);

  QThreadPool* threadPool = QThreadPool::globalInstance();
  threadPool->start(pJob);

  m_Controls->m_RegistrationStartButton->setEnabled(false);
  m_Controls->m_RegistrationStartButton->setText("Registration in progress ...");
}

void QmitkSimpleRegistrationView::OnRegResultIsAvailable(mitk::MAPRegistrationWrapper::Pointer spResultRegistration, const QmitkRegistrationJob* job)
{
  mitk::Image::Pointer movingImage = dynamic_cast<mitk::Image*>(m_MovingImageNode->GetData());
  mitk::Image::Pointer image;

  if (m_RegistrationType==0 && !m_Controls->m_ResampleBox->isChecked())
  {
    image = mitk::ImageMappingHelper::refineGeometry(movingImage, spResultRegistration, true);

    mitk::DiffusionPropertyHelper::CopyProperties(movingImage, image, true);
    auto reg = spResultRegistration->GetRegistration();
    typedef mitk::DiffusionImageCorrectionFilter CorrectionFilterType;
    CorrectionFilterType::Pointer corrector = CorrectionFilterType::New();
    corrector->SetImage( image );
    corrector->CorrectDirections( mitk::MITKRegistrationHelper::getAffineMatrix(reg, false)->GetMatrix().GetVnlMatrix() );
  }
  else
  {
    if (!mitk::DiffusionPropertyHelper::IsDiffusionWeightedImage(movingImage))
    {
      image = mitk::ImageMappingHelper::map(movingImage, spResultRegistration, false, 0, job->m_spTargetData->GetGeometry(), false, 0, mitk::ImageMappingInterpolator::BSpline_3);
    }
    else
    {
      typedef itk::ComposeImageFilter < ITKDiffusionVolumeType > ComposeFilterType;
      ComposeFilterType::Pointer composer = ComposeFilterType::New();

      ItkDwiType::Pointer itkVectorImagePointer = mitk::DiffusionPropertyHelper::GetItkVectorImage(movingImage);
      for (unsigned int i=0; i<itkVectorImagePointer->GetVectorLength(); ++i)
      {
        itk::ExtractDwiChannelFilter< short >::Pointer filter = itk::ExtractDwiChannelFilter< short >::New();
        filter->SetInput( itkVectorImagePointer);
        filter->SetChannelIndex(i);
        filter->Update();

        mitk::Image::Pointer gradientVolume = mitk::Image::New();
        gradientVolume->InitializeByItk( filter->GetOutput() );
        gradientVolume->SetImportChannel( filter->GetOutput()->GetBufferPointer() );

        mitk::Image::Pointer registered_mitk_image = mitk::ImageMappingHelper::map(gradientVolume, spResultRegistration, false, 0, job->m_spTargetData->GetGeometry(), false, 0, mitk::ImageMappingInterpolator::BSpline_3);

        ITKDiffusionVolumeType::Pointer registered_itk_image = ITKDiffusionVolumeType::New();
        mitk::CastToItkImage(registered_mitk_image, registered_itk_image);
        composer->SetInput(i, registered_itk_image);
      }

      composer->Update();

      image = mitk::GrabItkImageMemory( composer->GetOutput() );
      mitk::DiffusionPropertyHelper::CopyProperties(movingImage, image, true);

      auto reg = spResultRegistration->GetRegistration();
      typedef mitk::DiffusionImageCorrectionFilter CorrectionFilterType;
      CorrectionFilterType::Pointer corrector = CorrectionFilterType::New();
      corrector->SetImage( image );
      corrector->CorrectDirections( mitk::MITKRegistrationHelper::getAffineMatrix(reg, false)->GetMatrix().GetVnlMatrix() );
    }
  }

  if (mitk::DiffusionPropertyHelper::IsDiffusionWeightedImage(image))
    mitk::DiffusionPropertyHelper::InitializeImage( image );

  mitk::DataNode::Pointer resultNode = mitk::DataNode::New();
  resultNode->SetData(image);

  if (m_MovingImageNode.IsNotNull())
  {
    m_MovingImageNode->SetVisibility(false);
    QString name = m_MovingImageNode->GetName().c_str();
    if (m_RegistrationType==0)
      resultNode->SetName((name+"_registered (rigid)").toStdString().c_str());
    else
      resultNode->SetName((name+"_registered (affine)").toStdString().c_str());
  }
  else
  {
    if (m_RegistrationType==0)
      resultNode->SetName("Registered (rigid)");
    else
      resultNode->SetName("Registered (affine)");
  }
//  resultNode->SetOpacity(0.6);
//  resultNode->SetColor(0.0, 0.0, 1.0);
  GetDataStorage()->Add(resultNode);

  mitk::RenderingManager::GetInstance()->InitializeViews( resultNode->GetData()->GetTimeGeometry(),
                                                          mitk::RenderingManager::REQUEST_UPDATE_ALL,
                                                          true);

  if (m_Controls->m_RegOutputBox->isChecked())
  {
    mitk::DataNode::Pointer registration_node = mitk::DataNode::New();
    registration_node->SetData(spResultRegistration);
    if (m_RegistrationType==0)
      registration_node->SetName("Registration Object (rigid)");
    else
      registration_node->SetName("Registration Object (affine)");
    GetDataStorage()->Add(registration_node, resultNode);
  }

  this->GetRenderWindowPart()->RequestUpdate();

  m_Controls->m_RegistrationStartButton->setEnabled(true);
  m_Controls->m_RegistrationStartButton->setText("Start Registration");

  m_MovingImageNode = nullptr;

  TractoChanged();
}

void QmitkSimpleRegistrationView::CreateQtPartControl( QWidget *parent )
{
  // build up qt view, unless already done
  if ( !m_Controls )
  {
    // create GUI widgets from the Qt Designer's .ui file
    m_Controls = new Ui::QmitkSimpleRegistrationViewControls;
    m_Controls->setupUi( parent );

    m_Controls->m_FixedImageBox->SetDataStorage(this->GetDataStorage());
    m_Controls->m_MovingImageBox->SetDataStorage(this->GetDataStorage());

    mitk::TNodePredicateDataType<mitk::Image>::Pointer isImagePredicate = mitk::TNodePredicateDataType<mitk::Image>::New();
    m_Controls->m_FixedImageBox->SetPredicate(isImagePredicate);
    m_Controls->m_MovingImageBox->SetPredicate(isImagePredicate);

    mitk::TNodePredicateDataType<mitk::FiberBundle>::Pointer isFib = mitk::TNodePredicateDataType<mitk::FiberBundle>::New();
    mitk::TNodePredicateDataType<mitk::MAPRegistrationWrapper>::Pointer isReg = mitk::TNodePredicateDataType<mitk::MAPRegistrationWrapper>::New();

    m_Controls->m_TractoBox->SetDataStorage(this->GetDataStorage());
    m_Controls->m_RegObjectBox->SetDataStorage(this->GetDataStorage());
    m_Controls->m_TractoBox->SetPredicate(isFib);
    m_Controls->m_RegObjectBox->SetPredicate(isReg);

    connect( m_Controls->m_FixedImageBox, SIGNAL(currentIndexChanged(int)), this, SLOT(FixedImageChanged()) );
    connect( m_Controls->m_MovingImageBox, SIGNAL(currentIndexChanged(int)), this, SLOT(MovingImageChanged()) );

    connect( m_Controls->m_TractoBox, SIGNAL(currentIndexChanged(int)), this, SLOT(TractoChanged()) );
    connect( m_Controls->m_RegObjectBox, SIGNAL(currentIndexChanged(int)), this, SLOT(TractoChanged()) );

    connect( m_Controls->m_RegistrationStartButton, SIGNAL(clicked()), this, SLOT(StartRegistration()) );
    connect( m_Controls->m_TractoRegistrationStartButton, SIGNAL(clicked()), this, SLOT(StartTractoRegistration()) );

    FixedImageChanged();
    MovingImageChanged();
    TractoChanged();
  }
}

void QmitkSimpleRegistrationView::StartTractoRegistration()
{
  mitk::FiberBundle::Pointer fib = dynamic_cast<mitk::FiberBundle*>(m_Controls->m_TractoBox->GetSelectedNode()->GetData());
  mitk::MAPRegistrationWrapper::Pointer reg = dynamic_cast<mitk::MAPRegistrationWrapper*>(m_Controls->m_RegObjectBox->GetSelectedNode()->GetData());

  mitk::MITKRegistrationHelper::Affine3DTransformType::Pointer affine = mitk::MITKRegistrationHelper::getAffineMatrix(reg, false);

  mitk::FiberBundle::Pointer fib_copy = fib->GetDeepCopy();
  fib_copy->TransformFibers(affine);

  mitk::DataNode::Pointer registration_node = mitk::DataNode::New();
  registration_node->SetData(fib_copy);
  QString name = m_Controls->m_TractoBox->GetSelectedNode()->GetName().c_str();
  registration_node->SetName((name+"_registered").toStdString().c_str());
  GetDataStorage()->Add(registration_node, m_Controls->m_TractoBox->GetSelectedNode());
}

void QmitkSimpleRegistrationView::TractoChanged()
{
  if (m_Controls->m_RegObjectBox->GetSelectedNode().IsNotNull() && m_Controls->m_TractoBox->GetSelectedNode().IsNotNull())
    m_Controls->m_TractoRegistrationStartButton->setEnabled(true);
  else
    m_Controls->m_TractoRegistrationStartButton->setEnabled(false);
}

void QmitkSimpleRegistrationView::FixedImageChanged()
{
  if (m_Controls->m_FixedImageBox->GetSelectedNode().IsNotNull())
  {
    mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(m_Controls->m_FixedImageBox->GetSelectedNode()->GetData());
    int channels = image->GetNumberOfChannels();
    int dims = image->GetDimension();
    int fourth_dim_size = image->GetTimeSteps();
    bool isdiff = mitk::DiffusionPropertyHelper::IsDiffusionWeightedImage(image);

    if (dims==4 || channels>1)
    {
      m_Controls->m_FixedChannelBox->setEnabled(false);
      m_Controls->m_RegistrationStartButton->setEnabled(false);
    }

    if (isdiff)
    {
      m_Controls->m_FixedChannelBox->setEnabled(true);
      if (fourth_dim_size>1)
        m_Controls->m_FixedChannelBox->setMaximum(fourth_dim_size-1);
      else if (isdiff)
        m_Controls->m_FixedChannelBox->setMaximum(mitk::DiffusionPropertyHelper::GetGradientContainer(image)->Size()-1);
    }
    else
    {
      m_Controls->m_FixedChannelBox->setEnabled(false);
    }

    m_Controls->m_RegistrationStartButton->setEnabled(true);
  }
  else
  {
    m_Controls->m_FixedChannelBox->setEnabled(false);
    m_Controls->m_RegistrationStartButton->setEnabled(false);
  }
}

void QmitkSimpleRegistrationView::MovingImageChanged()
{
  if (m_Controls->m_MovingImageBox->GetSelectedNode().IsNotNull())
  {
    mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(m_Controls->m_MovingImageBox->GetSelectedNode()->GetData());
    int channels = image->GetNumberOfChannels();
    int dims = image->GetDimension();
    int fourth_dim_size = image->GetTimeSteps();
    bool isdiff = mitk::DiffusionPropertyHelper::IsDiffusionWeightedImage(image);

    if (dims==4 || channels>1)
    {
      m_Controls->m_MovingChannelBox->setEnabled(false);
      m_Controls->m_RegistrationStartButton->setEnabled(false);
    }

    if (isdiff)
    {
      m_Controls->m_MovingChannelBox->setEnabled(true);
      if (fourth_dim_size>1)
        m_Controls->m_MovingChannelBox->setMaximum(fourth_dim_size-1);
      else if (isdiff)
        m_Controls->m_MovingChannelBox->setMaximum(mitk::DiffusionPropertyHelper::GetGradientContainer(image)->Size()-1);
    }
    else
    {
      m_Controls->m_MovingChannelBox->setEnabled(false);
    }

    m_Controls->m_RegistrationStartButton->setEnabled(true);
  }
  else
  {
    m_Controls->m_MovingChannelBox->setEnabled(false);
    m_Controls->m_RegistrationStartButton->setEnabled(false);
  }
}

void QmitkSimpleRegistrationView::OnSelectionChanged(berry::IWorkbenchPart::Pointer, const QList<mitk::DataNode::Pointer>& )
{
  FixedImageChanged();
  MovingImageChanged();
  TractoChanged();
}


void QmitkSimpleRegistrationView::SetFocus()
{
  m_Controls->m_RegistrationStartButton->setFocus();
  FixedImageChanged();
  MovingImageChanged();
}

