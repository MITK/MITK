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


#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>

#include "QmitkFiberFitView.h"

#include <mitkNodePredicateDataType.h>
#include <mitkImageCast.h>
#include <mitkImageAccessByItk.h>
#include <mitkImage.h>
#include <mitkPeakImage.h>
#include <mitkFiberBundle.h>
#include <itkFitFibersToImageFilter.h>
#include <mitkDiffusionPropertyHelper.h>
#include <mitkTensorModel.h>
#include <mitkITKImageImport.h>
#include <mitkNodePredicateDimension.h>
#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateOr.h>

const std::string QmitkFiberFitView::VIEW_ID = "org.mitk.views.fiberfit";
using namespace mitk;

QmitkFiberFitView::QmitkFiberFitView()
  : QmitkAbstractView()
  , m_Controls( nullptr )
{

}

// Destructor
QmitkFiberFitView::~QmitkFiberFitView()
{

}

void QmitkFiberFitView::CreateQtPartControl( QWidget *parent )
{
  // build up qt view, unless already done
  if ( !m_Controls )
  {
    // create GUI widgets from the Qt Designer's .ui file
    m_Controls = new Ui::QmitkFiberFitViewControls;
    m_Controls->setupUi( parent );

    connect( m_Controls->m_StartButton, SIGNAL(clicked()), this, SLOT(StartFit()) );

    connect( m_Controls->m_ImageBox, SIGNAL(currentIndexChanged(int)), this, SLOT(DataSelectionChanged()) );
    connect( m_Controls->m_TractBox, SIGNAL(currentIndexChanged(int)), this, SLOT(DataSelectionChanged()) );

    mitk::TNodePredicateDataType<mitk::FiberBundle>::Pointer isFib = mitk::TNodePredicateDataType<mitk::FiberBundle>::New();
    mitk::TNodePredicateDataType<mitk::Image>::Pointer isImage = mitk::TNodePredicateDataType<mitk::Image>::New();
    mitk::NodePredicateDimension::Pointer is3D = mitk::NodePredicateDimension::New(3);

    m_Controls->m_TractBox->SetDataStorage(this->GetDataStorage());
    m_Controls->m_TractBox->SetPredicate(isFib);

    m_Controls->m_ImageBox->SetDataStorage(this->GetDataStorage());
    m_Controls->m_ImageBox->SetPredicate( mitk::NodePredicateOr::New( mitk::NodePredicateAnd::New(isImage, is3D), mitk::TNodePredicateDataType<mitk::PeakImage>::New()) );

    DataSelectionChanged();
  }
}

void QmitkFiberFitView::DataSelectionChanged()
{
  if (m_Controls->m_TractBox->GetSelectedNode().IsNull() || m_Controls->m_ImageBox->GetSelectedNode().IsNull())
    m_Controls->m_StartButton->setEnabled(false);
  else
    m_Controls->m_StartButton->setEnabled(true);
}

void QmitkFiberFitView::SetFocus()
{
  DataSelectionChanged();
}

void QmitkFiberFitView::StartFit()
{
  if (m_Controls->m_TractBox->GetSelectedNode().IsNull() || m_Controls->m_ImageBox->GetSelectedNode().IsNull())
    return;

  mitk::FiberBundle::Pointer input_tracts = dynamic_cast<mitk::FiberBundle*>(m_Controls->m_TractBox->GetSelectedNode()->GetData())->GetDeepCopy();

  mitk::DataNode::Pointer node = m_Controls->m_ImageBox->GetSelectedNode();
  itk::FitFibersToImageFilter::Pointer fitter = itk::FitFibersToImageFilter::New();

  mitk::Image::Pointer mitk_image = dynamic_cast<mitk::Image*>(node->GetData());
  mitk::PeakImage::Pointer mitk_peak_image = dynamic_cast<mitk::PeakImage*>(node->GetData());
  if (mitk_peak_image.IsNotNull())
  {
    typedef mitk::ImageToItk< mitk::PeakImage::ItkPeakImageType > CasterType;
    CasterType::Pointer caster = CasterType::New();
    caster->SetInput(mitk_peak_image);
    caster->Update();
    mitk::PeakImage::ItkPeakImageType::Pointer peak_image = caster->GetOutput();
    fitter->SetPeakImage(peak_image);
  }
  else
  {
    if (mitk::DiffusionPropertyHelper::IsDiffusionWeightedImage(mitk_image))
    {
      fitter->SetDiffImage(mitk::DiffusionPropertyHelper::GetItkVectorImage(mitk_image));
      mitk::TensorModel<>* model = new mitk::TensorModel<>();
      model->SetBvalue(1000);
      model->SetDiffusivity1(0.0010);
      model->SetDiffusivity2(0.00015);
      model->SetDiffusivity3(0.00015);
      model->SetGradientList(mitk::DiffusionPropertyHelper::GetGradientContainer(mitk_image));
      fitter->SetSignalModel(model);
    }
    else
    {
      itk::FitFibersToImageFilter::DoubleImgType::Pointer scalar_image = itk::FitFibersToImageFilter::DoubleImgType::New();
      mitk::CastToItkImage(mitk_image, scalar_image);
      fitter->SetScalarImage(scalar_image);
    }
  }

  if (m_Controls->m_ReguTypeBox->currentIndex()==0)
    fitter->SetRegularization(VnlCostFunction::REGU::VOXEL_VARIANCE);
  else if (m_Controls->m_ReguTypeBox->currentIndex()==1)
    fitter->SetRegularization(VnlCostFunction::REGU::VARIANCE);
  else if (m_Controls->m_ReguTypeBox->currentIndex()==2)
    fitter->SetRegularization(VnlCostFunction::REGU::MSM);
  else if (m_Controls->m_ReguTypeBox->currentIndex()==3)
    fitter->SetRegularization(VnlCostFunction::REGU::LASSO);
  else if (m_Controls->m_ReguTypeBox->currentIndex()==4)
    fitter->SetRegularization(VnlCostFunction::REGU::NONE);

  fitter->SetTractograms({input_tracts});
  fitter->SetFitIndividualFibers(true);
  fitter->SetMaxIterations(20);
  fitter->SetVerbose(true);
  fitter->SetGradientTolerance(1e-5);
  fitter->SetLambda(m_Controls->m_ReguBox->value());
  fitter->SetFilterOutliers(m_Controls->m_OutliersBox->isChecked());
  fitter->Update();

  mitk::FiberBundle::Pointer output_tracts = fitter->GetTractograms().at(0);
  output_tracts->ColorFibersByFiberWeights(false, true);
  mitk::DataNode::Pointer new_node = mitk::DataNode::New();
  new_node->SetData(output_tracts);
  new_node->SetName("Fitted");
  this->GetDataStorage()->Add(new_node, node);
  m_Controls->m_TractBox->GetSelectedNode()->SetVisibility(false);

  if (m_Controls->m_ResidualsBox->isChecked() && mitk_peak_image.IsNotNull())
  {
    {
      mitk::PeakImage::ItkPeakImageType::Pointer itk_image = fitter->GetFittedImage();

      mitk::Image::Pointer mitk_image = dynamic_cast<Image*>(PeakImage::New().GetPointer());
      mitk::CastToMitkImage(itk_image, mitk_image);
      mitk_image->SetVolume(itk_image->GetBufferPointer());

      mitk::DataNode::Pointer new_node = mitk::DataNode::New();
      new_node->SetData(mitk_image);
      new_node->SetName("Fitted");
      this->GetDataStorage()->Add(new_node, node);
    }

    {
      mitk::PeakImage::ItkPeakImageType::Pointer itk_image = fitter->GetResidualImage();

      mitk::Image::Pointer mitk_image = dynamic_cast<Image*>(PeakImage::New().GetPointer());
      mitk::CastToMitkImage(itk_image, mitk_image);
      mitk_image->SetVolume(itk_image->GetBufferPointer());

      mitk::DataNode::Pointer new_node = mitk::DataNode::New();
      new_node->SetData(mitk_image);
      new_node->SetName("Residual");
      this->GetDataStorage()->Add(new_node, node);
    }

    {
      mitk::PeakImage::ItkPeakImageType::Pointer itk_image = fitter->GetUnderexplainedImage();

      mitk::Image::Pointer mitk_image = dynamic_cast<Image*>(PeakImage::New().GetPointer());
      mitk::CastToMitkImage(itk_image, mitk_image);
      mitk_image->SetVolume(itk_image->GetBufferPointer());

      mitk::DataNode::Pointer new_node = mitk::DataNode::New();
      new_node->SetData(mitk_image);
      new_node->SetName("Underexplained");
      this->GetDataStorage()->Add(new_node, node);
    }

    {
      mitk::PeakImage::ItkPeakImageType::Pointer itk_image = fitter->GetOverexplainedImage();

      mitk::Image::Pointer mitk_image = dynamic_cast<Image*>(PeakImage::New().GetPointer());
      mitk::CastToMitkImage(itk_image, mitk_image);
      mitk_image->SetVolume(itk_image->GetBufferPointer());

      mitk::DataNode::Pointer new_node = mitk::DataNode::New();
      new_node->SetData(mitk_image);
      new_node->SetName("Overexplained");
      this->GetDataStorage()->Add(new_node, node);
    }
  }
  else if (m_Controls->m_ResidualsBox->isChecked() && mitk::DiffusionPropertyHelper::IsDiffusionWeightedImage(mitk_image))
  {
    {
      mitk::Image::Pointer outImage = mitk::GrabItkImageMemory( fitter->GetFittedImageDiff().GetPointer() );
      mitk::DiffusionPropertyHelper::CopyProperties(mitk_image, outImage, true);
      mitk::DiffusionPropertyHelper::InitializeImage( outImage );

      mitk::DataNode::Pointer new_node = mitk::DataNode::New();
      new_node->SetData(outImage);
      new_node->SetName("Fitted");
      this->GetDataStorage()->Add(new_node, node);
    }

    {
      mitk::Image::Pointer outImage = mitk::GrabItkImageMemory( fitter->GetResidualImageDiff().GetPointer() );
      mitk::DiffusionPropertyHelper::CopyProperties(mitk_image, outImage, true);
      mitk::DiffusionPropertyHelper::InitializeImage( outImage );

      mitk::DataNode::Pointer new_node = mitk::DataNode::New();
      new_node->SetData(outImage);
      new_node->SetName("Residual");
      this->GetDataStorage()->Add(new_node, node);
    }
  }
  else if (m_Controls->m_ResidualsBox->isChecked())
  {
    {
      mitk::Image::Pointer outImage = mitk::GrabItkImageMemory( fitter->GetFittedImageScalar().GetPointer() );
      mitk::DataNode::Pointer new_node = mitk::DataNode::New();
      new_node->SetData(outImage);
      new_node->SetName("Fitted");
      this->GetDataStorage()->Add(new_node, node);
    }

    {
      mitk::Image::Pointer outImage = mitk::GrabItkImageMemory( fitter->GetResidualImageScalar().GetPointer() );
      mitk::DataNode::Pointer new_node = mitk::DataNode::New();
      new_node->SetData(outImage);
      new_node->SetName("Residual");
      this->GetDataStorage()->Add(new_node, node);
    }
  }
}

void QmitkFiberFitView::OnSelectionChanged(berry::IWorkbenchPart::Pointer /*part*/, const QList<mitk::DataNode::Pointer>& )
{

}
