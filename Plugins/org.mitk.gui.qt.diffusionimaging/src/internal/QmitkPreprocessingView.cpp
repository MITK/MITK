/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date: 2009-05-28 17:19:30 +0200 (Do, 28 Mai 2009) $
Version:   $Revision: 17495 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

//#define MBILOG_ENABLE_DEBUG

#include "QmitkPreprocessingView.h"
#include "mitkDiffusionImagingConfigure.h"

// qt includes
#include <QMessageBox>

// itk includes
#include "itkTimeProbe.h"
#include "itkB0ImageExtractionImageFilter.h"
#include "itkBrainMaskExtractionImageFilter.h"
#include "itkCastImageFilter.h"
#include "itkVectorContainer.h"
#include <itkReduceDirectionGradientsFilter.h>

// mitk includes
#include "QmitkDataStorageComboBox.h"
#include "QmitkStdMultiWidget.h"
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

#include <QTableWidgetItem>
#include <QTableWidget>

const std::string QmitkPreprocessingView::VIEW_ID =
"org.mitk.views.preprocessing";

#define DI_INFO MITK_INFO("DiffusionImaging")


typedef float TTensorPixelType;


QmitkPreprocessingView::QmitkPreprocessingView()
: QmitkFunctionality(),
m_Controls(NULL),
m_MultiWidget(NULL),
m_DiffusionImage(NULL)
{
}

QmitkPreprocessingView::QmitkPreprocessingView(const QmitkPreprocessingView& other)
{
  Q_UNUSED(other)
  throw std::runtime_error("Copy constructor not implemented");
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

    m_Controls->m_MeasurementFrameTable->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
    m_Controls->m_MeasurementFrameTable->verticalHeader()->setResizeMode(QHeaderView::Stretch);
  }
}

void QmitkPreprocessingView::StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget)
{
  m_MultiWidget = &stdMultiWidget;
}

void QmitkPreprocessingView::StdMultiWidgetNotAvailable()
{
  m_MultiWidget = NULL;
}

void QmitkPreprocessingView::CreateConnections()
{
  if ( m_Controls )
  {
    connect( (QObject*)(m_Controls->m_ButtonAverageGradients), SIGNAL(clicked()), this, SLOT(AverageGradients()) );
    connect( (QObject*)(m_Controls->m_ButtonExtractB0), SIGNAL(clicked()), this, SLOT(ExtractB0()) );
    connect( (QObject*)(m_Controls->m_ButtonBrainMask), SIGNAL(clicked()), this, SLOT(BrainMask()) );
    connect( (QObject*)(m_Controls->m_ModifyMeasurementFrame), SIGNAL(clicked()), this, SLOT(DoApplyMesurementFrame()) );
    connect( (QObject*)(m_Controls->m_ReduceGradientsButton), SIGNAL(clicked()), this, SLOT(DoReduceGradientDirections()) );
    connect( (QObject*)(m_Controls->m_ShowGradientsButton), SIGNAL(clicked()), this, SLOT(DoShowGradientDirections()) );
    connect( (QObject*)(m_Controls->m_MirrorGradientToHalfSphereButton), SIGNAL(clicked()), this, SLOT(DoHalfSphereGradientDirections()) );
  }
}

void QmitkPreprocessingView::OnSelectionChanged( std::vector<mitk::DataNode*> nodes )
{
  bool foundDwiVolume = false;
  m_DiffusionImage = NULL;
  m_SelectedDiffusionNodes = mitk::DataStorage::SetOfObjects::New();

  // iterate selection
  for( std::vector<mitk::DataNode*>::iterator it = nodes.begin(); it != nodes.end(); ++it )
  {
    mitk::DataNode::Pointer node = *it;

    if( node.IsNotNull() && dynamic_cast<mitk::DiffusionImage<short>*>(node->GetData()) )
    {
      foundDwiVolume = true;
      m_DiffusionImage = dynamic_cast<mitk::DiffusionImage<DiffusionPixelType>*>(node->GetData());
      m_Controls->m_DiffusionImageLabel->setText(node->GetName().c_str());
      m_SelectedDiffusionNodes->push_back(node);
    }
  }

  m_Controls->m_ButtonBrainMask->setEnabled(foundDwiVolume);
  m_Controls->m_ButtonAverageGradients->setEnabled(foundDwiVolume);
  m_Controls->m_ButtonExtractB0->setEnabled(foundDwiVolume);
  m_Controls->m_ModifyMeasurementFrame->setEnabled(foundDwiVolume);
  m_Controls->m_MeasurementFrameTable->setEnabled(foundDwiVolume);
  m_Controls->m_ReduceGradientsButton->setEnabled(foundDwiVolume);
  m_Controls->m_ShowGradientsButton->setEnabled(foundDwiVolume);
  m_Controls->m_MirrorGradientToHalfSphereButton->setEnabled(foundDwiVolume);

  if (foundDwiVolume)
  {
    vnl_matrix_fixed< double, 3, 3 > mf = m_DiffusionImage->GetMeasurementFrame();
    for (int r=0; r<3; r++)
      for (int c=0; c<3; c++)
      {
        QTableWidgetItem* item = m_Controls->m_MeasurementFrameTable->item(r,c);
        delete item;
        item = new QTableWidgetItem();
        item->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
        item->setText(QString::number(mf.get(r,c)));
        m_Controls->m_MeasurementFrameTable->setItem(r,c,item);
      }
    m_Controls->m_GradientsLabel->setText(QString::number(m_DiffusionImage->GetNumDirections()));

    if (m_DiffusionImage->IsMultiBval())
      m_Controls->m_BvalLabel->setText("Acquisition with multiple b-values!");
    else
      m_Controls->m_BvalLabel->setText(QString::number(m_DiffusionImage->GetB_Value()));
  }
  else
  {
    for (int r=0; r<3; r++)
      for (int c=0; c<3; c++)
      {
        QTableWidgetItem* item = m_Controls->m_MeasurementFrameTable->item(r,c);
        delete item;
        item = new QTableWidgetItem();
        m_Controls->m_MeasurementFrameTable->setItem(r,c,item);
      }
    m_Controls->m_GradientsLabel->setText("-");
    m_Controls->m_BvalLabel->setText("-");
    m_Controls->m_DiffusionImageLabel->setText("-");
  }
}

void QmitkPreprocessingView::Activated()
{
  QmitkFunctionality::Activated();
}

void QmitkPreprocessingView::Deactivated()
{
  QmitkFunctionality::Deactivated();
}

void QmitkPreprocessingView::DoHalfSphereGradientDirections()
{
  if (m_DiffusionImage.IsNull())
    return;

  GradientDirectionContainerType::Pointer gradientContainer = m_DiffusionImage->GetOriginalDirections();

  for (int j=0; j<gradientContainer->Size(); j++)
    if (gradientContainer->at(j)[0]<0)
      gradientContainer->at(j) = -gradientContainer->at(j);
}

void QmitkPreprocessingView::DoApplyMesurementFrame()
{
  if (m_DiffusionImage.IsNull())
    return;
  vnl_matrix_fixed< double, 3, 3 > mf;
  for (int r=0; r<3; r++)
    for (int c=0; c<3; c++)
    {
      QTableWidgetItem* item = m_Controls->m_MeasurementFrameTable->item(r,c);
      if (!item)
        return;
      mf[r][c] = item->text().toDouble();
    }
  m_DiffusionImage->SetMeasurementFrame(mf);
}

void QmitkPreprocessingView::DoShowGradientDirections()
{
  if (m_DiffusionImage.IsNull())
    return;

  GradientDirectionContainerType::Pointer gradientContainer = m_DiffusionImage->GetOriginalDirections();

  mitk::PointSet::Pointer pointset = mitk::PointSet::New();
  for (int j=0; j<gradientContainer->Size(); j++)
  {
    mitk::Point3D p;
    vnl_vector_fixed< double, 3 > v = gradientContainer->at(j);
    if (fabs(v[0])>0.001 || fabs(v[1])>0.001 || fabs(v[2])>0.001)
    {
      p[0] = v[0];
      p[1] = v[1];
      p[2] = v[2];
      pointset->InsertPoint(j, p);
    }
  }
  mitk::DataNode::Pointer node = mitk::DataNode::New();
  node->SetData(pointset);
  node->SetName("gradient directions");
  node->SetProperty("pointsize", mitk::FloatProperty::New(0.05));
  node->SetProperty("color", mitk::ColorProperty::New(1,0,0));
  GetDefaultDataStorage()->Add(node);
}

void QmitkPreprocessingView::DoReduceGradientDirections()
{
  if (m_DiffusionImage.IsNull())
    return;

  typedef mitk::DiffusionImage<DiffusionPixelType>              DiffusionImageType;
  typedef itk::ReduceDirectionGradientsFilter<DiffusionPixelType, DiffusionPixelType> FilterType;

  GradientDirectionContainerType::Pointer gradientContainer = m_DiffusionImage->GetOriginalDirections();
  FilterType::Pointer filter = FilterType::New();
  filter->SetInput(m_DiffusionImage->GetVectorImage());
  filter->SetOriginalGradientDirections(gradientContainer);
  filter->SetNumGradientDirections(m_Controls->m_ReduceGradientsBox->value());
  filter->Update();

  DiffusionImageType::Pointer image = DiffusionImageType::New();
  image->SetVectorImage( filter->GetOutput() );
  image->SetB_Value(m_DiffusionImage->GetB_Value());
  image->SetDirections(filter->GetGradientDirections());
  image->SetOriginalDirections(filter->GetGradientDirections());
  image->SetMeasurementFrame(m_DiffusionImage->GetMeasurementFrame());
  image->InitializeFromVectorImage();
  mitk::DataNode::Pointer imageNode = mitk::DataNode::New();
  imageNode->SetData( image );
  imageNode->SetName("reduced_image");
  GetDefaultDataStorage()->Add(imageNode);
}

void QmitkPreprocessingView::ExtractB0()
{
  typedef mitk::DiffusionImage<DiffusionPixelType>              DiffusionImageType;
  typedef DiffusionImageType::GradientDirectionContainerType    GradientContainerType;

  int nrFiles = m_SelectedDiffusionNodes->size();
  if (!nrFiles) return;

  mitk::DataStorage::SetOfObjects::const_iterator itemiter( m_SelectedDiffusionNodes->begin() );
  mitk::DataStorage::SetOfObjects::const_iterator itemiterend( m_SelectedDiffusionNodes->end() );

  std::vector<mitk::DataNode::Pointer> nodes;
  while ( itemiter != itemiterend ) // for all items
  {

    DiffusionImageType* vols =
      static_cast<DiffusionImageType*>(
      (*itemiter)->GetData());

    std::string nodename;
    (*itemiter)->GetStringProperty("name", nodename);

    // Extract image using found index
    typedef itk::B0ImageExtractionImageFilter<short, short> FilterType;
    FilterType::Pointer filter = FilterType::New();
    filter->SetInput(vols->GetVectorImage());
    filter->SetDirections(vols->GetDirections());
    filter->Update();

    mitk::Image::Pointer mitkImage = mitk::Image::New();
    mitkImage->InitializeByItk( filter->GetOutput() );
    mitkImage->SetVolume( filter->GetOutput()->GetBufferPointer() );
    mitk::DataNode::Pointer node=mitk::DataNode::New();
    node->SetData( mitkImage );
    node->SetProperty( "name", mitk::StringProperty::New(nodename + "_B0"));

    GetDefaultDataStorage()->Add(node);

    ++itemiter;
  }
}

void QmitkPreprocessingView::AverageGradients()
{
  int nrFiles = m_SelectedDiffusionNodes->size();
  if (!nrFiles) return;

  mitk::DataStorage::SetOfObjects::const_iterator itemiter( m_SelectedDiffusionNodes->begin() );
  mitk::DataStorage::SetOfObjects::const_iterator itemiterend( m_SelectedDiffusionNodes->end() );

  std::vector<mitk::DataNode::Pointer> nodes;
  while ( itemiter != itemiterend ) // for all items
  {

    mitk::DiffusionImage<DiffusionPixelType>* vols =
      static_cast<mitk::DiffusionImage<DiffusionPixelType>*>(
      (*itemiter)->GetData());

    vols->AverageRedundantGradients(m_Controls->m_Blur->value());

    ++itemiter;
  }
}

void QmitkPreprocessingView::BrainMask()
{
  int nrFiles = m_SelectedDiffusionNodes->size();
  if (!nrFiles) return;

  mitk::DataStorage::SetOfObjects::const_iterator itemiter( m_SelectedDiffusionNodes->begin() );
  mitk::DataStorage::SetOfObjects::const_iterator itemiterend( m_SelectedDiffusionNodes->end() );

  while ( itemiter != itemiterend ) // for all items
  {

    mitk::DiffusionImage<DiffusionPixelType>* vols =
      static_cast<mitk::DiffusionImage<DiffusionPixelType>*>(
      (*itemiter)->GetData());

    std::string nodename;
    (*itemiter)->GetStringProperty("name", nodename);

    // Extract image using found index
    typedef itk::B0ImageExtractionImageFilter<short, short> FilterType;
    FilterType::Pointer filter = FilterType::New();
    filter->SetInput(vols->GetVectorImage());
    filter->SetDirections(vols->GetDirections());

    typedef itk::CastImageFilter<itk::Image<short,3>, itk::Image<unsigned short,3> > CastFilterType;
    CastFilterType::Pointer castfilter = CastFilterType::New();
    castfilter->SetInput(filter->GetOutput());

    typedef itk::BrainMaskExtractionImageFilter<unsigned char> MaskFilterType;
    MaskFilterType::Pointer maskfilter = MaskFilterType::New();
    maskfilter->SetInput(castfilter->GetOutput());
    maskfilter->Update();

    mitk::Image::Pointer mitkImage = mitk::Image::New();
    mitkImage->InitializeByItk( maskfilter->GetOutput() );
    mitkImage->SetVolume( maskfilter->GetOutput()->GetBufferPointer() );
    mitk::DataNode::Pointer node=mitk::DataNode::New();
    node->SetData( mitkImage );
    node->SetProperty( "name", mitk::StringProperty::New(nodename + "_Mask"));

    GetDefaultDataStorage()->Add(node);

    ++itemiter;
  }
}
