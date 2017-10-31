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
    mitk::TNodePredicateDataType<mitk::PeakImage>::Pointer isPeak = mitk::TNodePredicateDataType<mitk::PeakImage>::New();

    m_Controls->m_TractBox->SetDataStorage(this->GetDataStorage());
    m_Controls->m_TractBox->SetPredicate(isFib);

    m_Controls->m_ImageBox->SetDataStorage(this->GetDataStorage());
    m_Controls->m_ImageBox->SetPredicate(isPeak);

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

  mitk::DataNode::Pointer node = m_Controls->m_ImageBox->GetSelectedNode();

  mitk::PeakImage::Pointer mitk_peak_image = dynamic_cast<mitk::PeakImage*>(node->GetData());

  typedef mitk::ImageToItk< mitk::PeakImage::ItkPeakImageType > CasterType;
  CasterType::Pointer caster = CasterType::New();
  caster->SetInput(mitk_peak_image);
  caster->Update();
  mitk::PeakImage::ItkPeakImageType::Pointer peak_image = caster->GetOutput();

  mitk::FiberBundle::Pointer input_tracts = dynamic_cast<mitk::FiberBundle*>(m_Controls->m_TractBox->GetSelectedNode()->GetData());

  itk::FitFibersToImageFilter::Pointer fitter = itk::FitFibersToImageFilter::New();
  fitter->SetPeakImage(peak_image);
  fitter->SetTractograms({input_tracts});
  fitter->SetFitIndividualFibers(true);
  fitter->SetMaxIterations(20);
  fitter->SetVerbose(true);
  fitter->SetGradientTolerance(1e-5);
  fitter->SetLambda(m_Controls->m_ReguBox->value());
  fitter->SetFilterOutliers(m_Controls->m_OutliersBox->isChecked());
  fitter->Update();

  mitk::FiberBundle::Pointer output_tracts = fitter->GetTractograms().at(0);
  mitk::DataNode::Pointer new_node = mitk::DataNode::New();
  new_node->SetData(output_tracts);
  new_node->SetName("Fitted");
  this->GetDataStorage()->Add(new_node, node);
  m_Controls->m_TractBox->GetSelectedNode()->SetVisibility(false);

  if (m_Controls->m_ResidualsBox->isChecked())
  {
    {
      mitk::PeakImage::ItkPeakImageType::Pointer itk_image = fitter->GetFittedImage();

      mitk::Image::Pointer mitk_image = dynamic_cast<Image*>(PeakImage::New().GetPointer());
      mitk::CastToMitkImage(itk_image, mitk_image);
      mitk_image->SetVolume(itk_image->GetBufferPointer());

      mitk::DataNode::Pointer new_node = mitk::DataNode::New();
      new_node->SetData(mitk_image);
      new_node->SetName("Explained");
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
}

void QmitkFiberFitView::OnSelectionChanged(berry::IWorkbenchPart::Pointer /*part*/, const QList<mitk::DataNode::Pointer>& )
{

}
