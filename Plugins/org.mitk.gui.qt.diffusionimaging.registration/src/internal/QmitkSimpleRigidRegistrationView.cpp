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
#include "QmitkSimpleRigidRegistrationView.h"

// MITK
#include <mitkNodePredicateDataType.h>
#include <mitkDiffusionPropertyHelper.h>
#include <mitkImageMappingHelper.h>
#include <mitkITKImageImport.h>
#include <mitkImageCast.h>
#include <itkExtractDwiChannelFilter.h>

// Qt
#include <QThreadPool>

#define _USE_MATH_DEFINES
#include <math.h>

const std::string QmitkSimpleRigidRegistrationView::VIEW_ID = "org.mitk.views.simplerigidregistrationview";

QmitkSimpleRigidRegistrationView::QmitkSimpleRigidRegistrationView()
  : QmitkAbstractView()
  , m_Controls( 0 )
{

}

// Destructor
QmitkSimpleRigidRegistrationView::~QmitkSimpleRigidRegistrationView()
{

}

void QmitkSimpleRigidRegistrationView::StartRegistration()
{
  typedef itk::Image< float, 3 > ItkFloatImageType;
  mitk::MultiModalRigidDefaultRegistrationAlgorithm< ItkFloatImageType >::Pointer algo = mitk::MultiModalRigidDefaultRegistrationAlgorithm< ItkFloatImageType >::New();

  QmitkRegistrationJob* pJob = new QmitkRegistrationJob(algo);
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

//  connect(pJob, SIGNAL(AlgorithmInfo(QString)), this, SLOT(OnAlgorithmInfo(QString)));
//  connect(pJob, SIGNAL(AlgorithmStatusChanged(QString)), this, SLOT(OnAlgorithmStatusChanged(QString)));
//  connect(pJob, SIGNAL(AlgorithmIterated(QString, bool, unsigned long)), this, SLOT(OnAlgorithmIterated(QString, bool, unsigned long)));
//  connect(pJob, SIGNAL(LevelChanged(QString, bool, unsigned long)), this, SLOT(OnLevelChanged(QString, bool, unsigned long)));

  QThreadPool* threadPool = QThreadPool::globalInstance();
  threadPool->start(pJob);

  m_Controls->m_RegistrationStartButton->setEnabled(false);
  m_Controls->m_RegistrationStartButton->setText("Registration in progress ...");
}

void QmitkSimpleRigidRegistrationView::OnRegResultIsAvailable(mitk::MAPRegistrationWrapper::Pointer spResultRegistration, const QmitkRegistrationJob* )
{
  mitk::Image::Pointer movingImage = dynamic_cast<mitk::Image*>(m_MovingImageNode->GetData());
  mitk::Image::Pointer image = mitk::ImageMappingHelper::refineGeometry(movingImage, spResultRegistration, true);

  if (mitk::DiffusionPropertyHelper::IsDiffusionWeightedImage(image))
  {
    mitk::DiffusionPropertyHelper propertyHelper( image );
    propertyHelper.InitializeImage();
  }

  mitk::DataNode::Pointer resultNode = mitk::DataNode::New();
  resultNode->SetData(image);

  if (m_MovingImageNode.IsNotNull())
  {
    m_MovingImageNode->SetVisibility(false);
    QString name = m_MovingImageNode->GetName().c_str();
    resultNode->SetName((name+"_registered").toStdString().c_str());
  }
  else
    resultNode->SetName("Registered");
  resultNode->SetOpacity(0.6);
  resultNode->SetColor(0.0, 0.0, 1.0);

  GetDataStorage()->Add(resultNode);

  this->GetRenderWindowPart()->RequestUpdate();

  m_Controls->m_RegistrationStartButton->setEnabled(true);
  m_Controls->m_RegistrationStartButton->setText("Start Registration");

  m_MovingImageNode = nullptr;
}

void QmitkSimpleRigidRegistrationView::CreateQtPartControl( QWidget *parent )
{
  // build up qt view, unless already done
  if ( !m_Controls )
  {
    // create GUI widgets from the Qt Designer's .ui file
    m_Controls = new Ui::QmitkSimpleRigidRegistrationViewControls;
    m_Controls->setupUi( parent );

    m_Controls->m_FixedImageBox->SetDataStorage(this->GetDataStorage());
    m_Controls->m_MovingImageBox->SetDataStorage(this->GetDataStorage());

    mitk::TNodePredicateDataType<mitk::Image>::Pointer isImagePredicate = mitk::TNodePredicateDataType<mitk::Image>::New();
    m_Controls->m_FixedImageBox->SetPredicate(isImagePredicate);
    m_Controls->m_MovingImageBox->SetPredicate(isImagePredicate);

    connect( m_Controls->m_FixedImageBox, SIGNAL(currentIndexChanged(int)), this, SLOT(FixedImageChanged()) );
    connect( m_Controls->m_MovingImageBox, SIGNAL(currentIndexChanged(int)), this, SLOT(MovingImageChanged()) );
    connect( m_Controls->m_RegistrationStartButton, SIGNAL(clicked()), this, SLOT(StartRegistration()) );

    FixedImageChanged();
    MovingImageChanged();
  }
}

void QmitkSimpleRigidRegistrationView::FixedImageChanged()
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

void QmitkSimpleRigidRegistrationView::MovingImageChanged()
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

void QmitkSimpleRigidRegistrationView::OnSelectionChanged(berry::IWorkbenchPart::Pointer, const QList<mitk::DataNode::Pointer>& )
{
  FixedImageChanged();
  MovingImageChanged();
}


void QmitkSimpleRigidRegistrationView::SetFocus()
{
  m_Controls->m_RegistrationStartButton->setFocus();
  FixedImageChanged();
  MovingImageChanged();
}

