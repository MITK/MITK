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

// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>

// Qmitk
#include "QmitkDenoisingView.h"
#include <mitkImageCast.h>
#include <mitkDiffusionPropertyHelper.h>
#include <mitkITKImageImport.h>

#include <mitkDiffusionPropertyHelper.h>

#include <mitkNodePredicateDataType.h>
#include <mitkNodePredicateProperty.h>
#include <mitkNodePredicateAnd.h>
#include <itkTotalVariationDenoisingImageFilter.h>
#include <itkNonLocalMeansDenoisingFilter.h>
#include <itkDiscreteGaussianImageFilter.h>
#include <itkPatchBasedDenoisingImageFilter.h>
#include <itkMaskImageFilter.h>
#include <itkVectorImageToImageFilter.h>
#include <itkComposeImageFilter.h>
#include <itkGaussianRandomSpatialNeighborSubsampler.h>
#include <itkSpatialNeighborSubsampler.h>
#include <QMessageBox>
#include <mitkNodePredicateIsDWI.h>

const std::string QmitkDenoisingView::VIEW_ID = "org.mitk.views.denoisingview";

QmitkDenoisingView::QmitkDenoisingView()
  : QmitkAbstractView()
  , m_Controls( 0 )
  , m_ImageNode(nullptr)
{
}

QmitkDenoisingView::~QmitkDenoisingView()
{

}

void QmitkDenoisingView::CreateQtPartControl( QWidget *parent )
{
  // build up qt view, unless already done
  if ( !m_Controls )
  {
    // create GUI widgets from the Qt Designer's .ui file
    m_Controls = new Ui::QmitkDenoisingViewControls;
    m_Controls->setupUi( parent );

    connect( (QObject*)(m_Controls->m_ApplyButton), SIGNAL(clicked()), this, SLOT(StartDenoising()));
    connect( (QObject*)(m_Controls->m_SelectFilterComboBox), SIGNAL(activated(int)), this, SLOT(UpdateGui()));
    connect( (QObject*)(m_Controls->m_InputImageBox), SIGNAL(currentIndexChanged(int)), this, SLOT(UpdateGui()));


    m_Controls->m_InputImageBox->SetDataStorage(this->GetDataStorage());
    mitk::NodePredicateIsDWI::Pointer isImagePredicate = mitk::NodePredicateIsDWI::New();
    m_Controls->m_InputImageBox->SetPredicate( isImagePredicate );

    UpdateGui();
  }
}

void QmitkDenoisingView::SetFocus()
{
  m_Controls->m_SelectFilterComboBox->setFocus();
}

void QmitkDenoisingView::OnSelectionChanged(berry::IWorkbenchPart::Pointer /*part*/, const QList<mitk::DataNode::Pointer>& )
{

}

void QmitkDenoisingView::StartDenoising()
{
  typedef itk::DiscreteGaussianImageFilter < DwiVolumeType, DwiVolumeType >             GaussianFilterType;
  typedef itk::PatchBasedDenoisingImageFilter < DwiVolumeType, DwiVolumeType >          NlmFilterType;
  typedef itk::VectorImageToImageFilter < DiffusionPixelType >                          ExtractFilterType;
  typedef itk::ComposeImageFilter < itk::Image<DiffusionPixelType, 3> >                 ComposeFilterType;


  m_ImageNode = m_Controls->m_InputImageBox->GetSelectedNode();
  mitk::Image::Pointer input_image = dynamic_cast<mitk::Image*>(m_ImageNode->GetData());

  if (!mitk::DiffusionPropertyHelper::IsDiffusionWeightedImage(input_image))
  {
    QMessageBox::warning(nullptr, "Image not processed", "Input is not a diffusion-weighted image!");
    return;
  }

  DwiImageType::Pointer itkVectorImagePointer = mitk::DiffusionPropertyHelper::GetItkVectorImage(input_image);

  mitk::Image::Pointer denoised_image = nullptr;
  mitk::DataNode::Pointer denoised_image_node = mitk::DataNode::New();

  switch (m_Controls->m_SelectFilterComboBox->currentIndex())
  {
  case 0:
  {
    ComposeFilterType::Pointer composer = ComposeFilterType::New();
    for (unsigned int i=0; i<itkVectorImagePointer->GetVectorLength(); ++i)
    {
      ExtractFilterType::Pointer extractor = ExtractFilterType::New();
      extractor->SetInput( itkVectorImagePointer );
      extractor->SetIndex( i );
      extractor->Update();
      DwiVolumeType::Pointer gradient_volume = extractor->GetOutput();
      itk::TotalVariationDenoisingImageFilter< DwiVolumeType, DwiVolumeType >::Pointer filter = itk::TotalVariationDenoisingImageFilter< DwiVolumeType, DwiVolumeType >::New();
      filter->SetInput(gradient_volume);
      filter->SetLambda(m_Controls->m_TvLambdaBox->value());
      filter->SetNumberIterations(m_Controls->m_TvIterationsBox->value());
      filter->Update();
      composer->SetInput(i, filter->GetOutput());
    }
    composer->Update();
    denoised_image = mitk::GrabItkImageMemory(composer->GetOutput());
    denoised_image_node->SetName( "TotalVariation" );
    break;
  }
  case 1:
  {
    ExtractFilterType::Pointer extractor = ExtractFilterType::New();
    extractor->SetInput( itkVectorImagePointer );
    ComposeFilterType::Pointer composer = ComposeFilterType::New();
    for (unsigned int i = 0; i < itkVectorImagePointer->GetVectorLength(); ++i)
    {
      extractor->SetIndex(i);
      extractor->Update();
      GaussianFilterType::Pointer filter = GaussianFilterType::New();
      filter->SetVariance(m_Controls->m_GaussVarianceBox->value());
      filter->SetInput(extractor->GetOutput());
      filter->Update();
      composer->SetInput(i, filter->GetOutput());
    }
    composer->Update();
    denoised_image = mitk::GrabItkImageMemory(composer->GetOutput());
    denoised_image_node->SetName( "Gauss" );
    break;
  }
  case 2:
  {
    typedef itk::Statistics::GaussianRandomSpatialNeighborSubsampler< NlmFilterType::PatchSampleType, DwiVolumeType::RegionType > SamplerType;
    // sampling the image to find similar patches
    SamplerType::Pointer sampler = SamplerType::New();
    sampler->SetRadius( m_Controls->m_NlmSearchRadiusBox->value() );
    sampler->SetVariance( m_Controls->m_NlmSearchRadiusBox->value()*m_Controls->m_NlmSearchRadiusBox->value() );
    sampler->SetNumberOfResultsRequested( m_Controls->m_NlmNumPatchesBox->value() );

    MITK_INFO << "Starting NLM denoising";
    ExtractFilterType::Pointer extractor = ExtractFilterType::New();
    extractor->SetInput( itkVectorImagePointer );
    ComposeFilterType::Pointer composer = ComposeFilterType::New();
    for (unsigned int i = 0; i < itkVectorImagePointer->GetVectorLength(); ++i)
    {
      extractor->SetIndex(i);
      extractor->Update();
      NlmFilterType::Pointer filter = NlmFilterType::New();
      filter->SetInput(extractor->GetOutput());
      filter->SetPatchRadius(m_Controls->m_NlmPatchSizeBox->value());
      filter->SetNoiseModel(NlmFilterType::RICIAN);
      filter->UseSmoothDiscPatchWeightsOn();
      filter->UseFastTensorComputationsOn();

      filter->SetNumberOfIterations(m_Controls->m_NlmIterationsBox->value());
      filter->SetSmoothingWeight( 1 );
      filter->SetKernelBandwidthEstimation(true);

      filter->SetSampler( sampler );
      filter->Update();
      composer->SetInput(i, filter->GetOutput());
      MITK_INFO << "Gradient " << i << " finished";
    }
    composer->Update();
    denoised_image = mitk::GrabItkImageMemory(composer->GetOutput());
    denoised_image_node->SetName( "NLM" );
    break;
  }
  }

  mitk::DiffusionPropertyHelper::CopyProperties(input_image, denoised_image, true);
  mitk::DiffusionPropertyHelper::InitializeImage( denoised_image );

  denoised_image_node->SetData( denoised_image );
  GetDataStorage()->Add(denoised_image_node, m_ImageNode);
}

void QmitkDenoisingView::UpdateGui()
{
  m_Controls->m_ApplyButton->setEnabled(false);
  m_Controls->m_TvFrame->setVisible(false);
  m_Controls->m_NlmFrame->setVisible(false);
  m_Controls->m_GaussFrame->setVisible(false);

  switch (m_Controls->m_SelectFilterComboBox->currentIndex())
  {
  case 0:
  {
    m_Controls->m_TvFrame->setVisible(true);
    break;
  }
  case 1:
  {
    m_Controls->m_GaussFrame->setVisible(true);
    break;
  }
  case 2:
  {
    m_Controls->m_NlmFrame->setVisible(true);
    break;
  }
  }

  if (m_Controls->m_InputImageBox->GetSelectedNode().IsNotNull())
    m_Controls->m_ApplyButton->setEnabled(true);
}

