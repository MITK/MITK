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

#include "QmitkQBallReconstructionView.h"

// qt includes
#include <QMessageBox>

// itk includes
#include "itkTimeProbe.h"

// mitk includes
#include "mitkProgressBar.h"
#include "mitkStatusBar.h"

#include "mitkNodePredicateDataType.h"
#include "QmitkDataStorageComboBox.h"

#include "itkDiffusionQballReconstructionImageFilter.h"
#include "itkAnalyticalDiffusionQballReconstructionImageFilter.h"
#include "itkDiffusionMultiShellQballReconstructionImageFilter.h"
#include "itkVectorContainer.h"
#include "itkB0ImageExtractionImageFilter.h"
#include <itkBinaryThresholdImageFilter.h>

#include "mitkOdfImage.h"
#include "mitkProperties.h"
#include "mitkVtkResliceInterpolationProperty.h"
#include "mitkLookupTable.h"
#include "mitkLookupTableProperty.h"
#include "mitkTransferFunction.h"
#include "mitkTransferFunctionProperty.h"
#include "mitkDataNodeObject.h"
#include "mitkOdfNormalizationMethodProperty.h"
#include "mitkOdfScaleByProperty.h"
#include <mitkImageCast.h>
#include "mitkDiffusionImagingConfigure.h"
#include <mitkNodePredicateIsDWI.h>

#include "berryIStructuredSelection.h"
#include "berryIWorkbenchWindow.h"
#include "berryISelectionService.h"
#include <mitkShImage.h>
#include <boost/version.hpp>
#include <itkShToOdfImageFilter.h>
#include <mitkImageCast.h>
#include <mitkDiffusionFunctionCollection.h>

const std::string QmitkQBallReconstructionView::VIEW_ID = "org.mitk.views.qballreconstruction";


typedef float TTensorPixelType;

const int QmitkQBallReconstructionView::nrconvkernels = 252;


struct QbrShellSelection
{
  typedef mitk::DiffusionPropertyHelper::GradientDirectionType            GradientDirectionType;
  typedef mitk::DiffusionPropertyHelper::GradientDirectionsContainerType  GradientDirectionContainerType;
  typedef mitk::DiffusionPropertyHelper::BValueMapType                    BValueMapType;
  typedef itk::VectorImage< DiffusionPixelType, 3 >                       ITKDiffusionImageType;

  QmitkQBallReconstructionView* m_View;
  mitk::DataNode * m_Node;
  std::string m_NodeName;

  std::vector<QCheckBox *> m_CheckBoxes;
  QLabel * m_Label;

  mitk::Image * m_Image;

  QbrShellSelection(QmitkQBallReconstructionView* view, mitk::DataNode * node)
    : m_View(view),
      m_Node(node),
      m_NodeName(node->GetName())
  {
    m_Image = dynamic_cast<mitk::Image * > (node->GetData());

    if(!m_Image)
    {
      MITK_ERROR << "QmitkQBallReconstructionView::QbrShellSelection : no image selected";
      return;
    }
    bool isDiffusionImage( mitk::DiffusionPropertyHelper::IsDiffusionWeightedImage( dynamic_cast<mitk::Image *>(m_Node->GetData())) );

    if( !isDiffusionImage )
    {
      MITK_ERROR <<
                    "QmitkQBallReconstructionView::QbrShellSelection : selected image contains no diffusion information";
      return;
    }

    GenerateCheckboxes();

  }

  void GenerateCheckboxes()
  {
    BValueMapType origMap = mitk::DiffusionPropertyHelper::GetBValueMap(m_Image);
    BValueMapType::iterator itStart = origMap.begin();
    itStart++;
    BValueMapType::iterator itEnd = origMap.end();

    m_Label = new QLabel(m_NodeName.c_str());
    m_Label->setVisible(true);
    m_View->m_Controls->m_QBallSelectionBox->layout()->addWidget(m_Label);

    for(BValueMapType::iterator it = itStart ; it!= itEnd; it++)
    {
      QCheckBox * box  = new QCheckBox(QString::number(it->first));
      m_View->m_Controls->m_QBallSelectionBox->layout()->addWidget(box);

      box->setChecked(true);
      box->setCheckable(true);
      // box->setVisible(true);
      m_CheckBoxes.push_back(box);
    }
  }

  void SetVisible(bool vis)
  {
    foreach(QCheckBox * box, m_CheckBoxes)
    {
      box->setVisible(vis);
    }
  }

  BValueMapType GetBValueSelctionMap()
  {
    BValueMapType inputMap = mitk::DiffusionPropertyHelper::GetBValueMap(m_Image);
    BValueMapType outputMap;

    unsigned int val = 0;

    if(inputMap.find(0) == inputMap.end()){
      return outputMap;
    }else{
      outputMap[val] = inputMap[val];
    }

    foreach(QCheckBox * box, m_CheckBoxes)
    {
      if(box->isChecked()){
        val = box->text().toUInt();
        outputMap[val] = inputMap[val];
        MITK_INFO << val;
      }
    }

    return outputMap;
  }

  ~QbrShellSelection()
  {
    m_View->m_Controls->m_QBallSelectionBox->layout()->removeWidget(m_Label);
    delete m_Label;

    for(std::vector<QCheckBox *>::iterator it = m_CheckBoxes.begin() ; it!= m_CheckBoxes.end(); it++)
    {
      m_View->m_Controls->m_QBallSelectionBox->layout()->removeWidget((*it));
      delete (*it);
    }
    m_CheckBoxes.clear();
  }
};

QmitkQBallReconstructionView::QmitkQBallReconstructionView()
  : QmitkAbstractView(),
    m_Controls(nullptr)
{
}

QmitkQBallReconstructionView::~QmitkQBallReconstructionView()
{

}

void QmitkQBallReconstructionView::CreateQtPartControl(QWidget *parent)
{
  if (!m_Controls)
  {
    // create GUI widgets
    m_Controls = new Ui::QmitkQBallReconstructionViewControls;
    m_Controls->setupUi(parent);
    this->CreateConnections();

    QStringList items;
    items << "2" << "4" << "6" << "8" << "10" << "12";
    m_Controls->m_QBallReconstructionMaxLLevelComboBox->addItems(items);
    m_Controls->m_QBallReconstructionMaxLLevelComboBox->setCurrentIndex(1);
    MethodChoosen(m_Controls->m_QBallReconstructionMethodComboBox->currentIndex());
  }
}

void QmitkQBallReconstructionView::SetFocus()
{
}

void QmitkQBallReconstructionView::CreateConnections()
{
  if ( m_Controls )
  {
    connect( static_cast<QObject*>(m_Controls->m_ButtonStandard), SIGNAL(clicked()), this, SLOT(ReconstructStandard()) );
    connect( static_cast<QObject*>(m_Controls->m_QBallReconstructionMethodComboBox), SIGNAL(currentIndexChanged(int)), this, SLOT(MethodChoosen(int)) );
    connect( static_cast<QObject*>(m_Controls->m_QBallReconstructionThreasholdEdit), SIGNAL(valueChanged(int)), this, SLOT(PreviewThreshold(int)) );
    connect( static_cast<QObject*>(m_Controls->m_ConvertButton), SIGNAL(clicked()), this, SLOT(ConvertShImage()) );

    m_Controls->m_ImageBox->SetDataStorage(this->GetDataStorage());
    mitk::NodePredicateIsDWI::Pointer isDwi = mitk::NodePredicateIsDWI::New();
    m_Controls->m_ImageBox->SetPredicate( isDwi );

    m_Controls->m_ShImageBox->SetDataStorage(this->GetDataStorage());
    mitk::TNodePredicateDataType<mitk::ShImage>::Pointer isSh = mitk::TNodePredicateDataType<mitk::ShImage>::New();
    m_Controls->m_ShImageBox->SetPredicate( isSh );

    connect( static_cast<QObject*>(m_Controls->m_ImageBox), SIGNAL(currentIndexChanged(int)), this, SLOT(UpdateGui()));
    connect( static_cast<QObject*>(m_Controls->m_ShImageBox), SIGNAL(currentIndexChanged(int)), this, SLOT(UpdateGui()));

    UpdateGui();
  }
}

void QmitkQBallReconstructionView::ConvertShImage()
{
  if (m_Controls->m_ShImageBox->GetSelectedNode().IsNotNull())
  {
    mitk::Image::Pointer mitkImg = dynamic_cast<mitk::Image*>(m_Controls->m_ShImageBox->GetSelectedNode()->GetData());
    auto img = mitk::convert::GetOdfFromShImage(mitkImg);
    mitk::DataNode::Pointer node= mitk::DataNode::New();
    node->SetData( img );
    node->SetName(m_Controls->m_ShImageBox->GetSelectedNode()->GetName());
    GetDataStorage()->Add(node, m_Controls->m_ShImageBox->GetSelectedNode());
  }
}

void QmitkQBallReconstructionView::UpdateGui()
{
  m_Controls->m_ButtonStandard->setEnabled(false);
  if (m_Controls->m_ImageBox->GetSelectedNode().IsNotNull())
  {
    m_Controls->m_ButtonStandard->setEnabled(true);
    GenerateShellSelectionUI(m_Controls->m_ImageBox->GetSelectedNode());
  }

  m_Controls->m_ConvertButton->setEnabled(m_Controls->m_ShImageBox->GetSelectedNode().IsNotNull());
}

void QmitkQBallReconstructionView::OnSelectionChanged(berry::IWorkbenchPart::Pointer /*part*/, const QList<mitk::DataNode::Pointer>& /*nodes*/)
{
  UpdateGui();
}

void QmitkQBallReconstructionView::Activated()
{
}

void QmitkQBallReconstructionView::Deactivated()
{
}

void QmitkQBallReconstructionView::Visible()
{
}

void QmitkQBallReconstructionView::Hidden()
{
}

void QmitkQBallReconstructionView::ReconstructStandard()
{
  int index = m_Controls->m_QBallReconstructionMethodComboBox->currentIndex();

  switch(index)
  {
  case 0:
  {
    // Numerical
    Reconstruct(0,0);
    break;
  }
  case 1:
  {
    // Standard
    Reconstruct(1,0);
    break;
  }
  case 2:
  {
    // Solid Angle
    Reconstruct(1,6);
    break;
  }
  case 3:
  {
    // ADC
    Reconstruct(1,4);
    break;
  }
  case 4:
  {
    // Raw Signal
    Reconstruct(1,5);
    break;
  }
  case 5:
  {
    // Q-Ball reconstruction
    Reconstruct(2,0);
    break;
  }
  }
}

void QmitkQBallReconstructionView::MethodChoosen(int method)
{
  m_Controls->m_QBallSelectionBox->setHidden(true);
  m_Controls->m_OutputOdfImage->setHidden(true);

  if (method==0)
    m_Controls->m_ShFrame->setVisible(false);
  else
    m_Controls->m_ShFrame->setVisible(true);

  switch(method)
  {
  case 0:
    m_Controls->m_Description->setText("Numerical recon. (Tuch 2004)");
    break;
  case 1:
    m_Controls->m_Description->setText("Spherical harmonics recon. (Descoteaux 2007)");
    m_Controls->m_OutputOdfImage->setHidden(false);
    break;
  case 2:
    m_Controls->m_Description->setText("SH recon. with solid angle consideration (Aganj 2009)");
    m_Controls->m_OutputOdfImage->setHidden(false);
    break;
  case 3:
    m_Controls->m_Description->setText("SH solid angle with non-neg. constraint (Goh 2009)");
    m_Controls->m_OutputOdfImage->setHidden(false);
    break;
  case 4:
    m_Controls->m_Description->setText("SH recon. of the plain ADC-profiles");
    m_Controls->m_OutputOdfImage->setHidden(false);
    break;
  case 5:
    m_Controls->m_Description->setText("SH recon. of the raw diffusion signal");
    m_Controls->m_OutputOdfImage->setHidden(false);
    break;
  case 6:
    m_Controls->m_Description->setText("SH recon. of the multi shell diffusion signal (Aganj 2010)");
    m_Controls->m_QBallSelectionBox->setHidden(false);
    m_Controls->m_OutputOdfImage->setHidden(false);
    break;
  }
}

void QmitkQBallReconstructionView::Reconstruct(int method, int normalization)
{
  if (m_Controls->m_ImageBox->GetSelectedNode().IsNotNull())
  {
    if(method == 0)
    {
      NumericalQBallReconstruction(m_Controls->m_ImageBox->GetSelectedNode(), normalization);
    }
    else if(method == 1)
    {
      AnalyticalQBallReconstruction(m_Controls->m_ImageBox->GetSelectedNode(), normalization);
    }
    else if(method == 2)
    {
      MultiQBallReconstruction(m_Controls->m_ImageBox->GetSelectedNode());
    }
  }
}

void QmitkQBallReconstructionView::NumericalQBallReconstruction(mitk::DataNode::Pointer node, int normalization)
{
  try
  {
    mitk::Image* vols = static_cast<mitk::Image*>(node->GetData());

    std::string nodename = node->GetName();

    typedef itk::DiffusionQballReconstructionImageFilter
        <DiffusionPixelType, DiffusionPixelType, TTensorPixelType, ODF_SAMPLING_SIZE>
        QballReconstructionImageFilterType;

    ITKDiffusionImageType::Pointer itkVectorImagePointer = ITKDiffusionImageType::New();
    mitk::CastToItkImage(vols, itkVectorImagePointer);

    QballReconstructionImageFilterType::Pointer filter = QballReconstructionImageFilterType::New();
    filter->SetBValue(mitk::DiffusionPropertyHelper::GetReferenceBValue(vols));
    filter->SetGradientImage(mitk::DiffusionPropertyHelper::GetGradientContainer(vols), itkVectorImagePointer);
    filter->SetThreshold( static_cast<short>(m_Controls->m_QBallReconstructionThreasholdEdit->value()) );

    std::string nodePostfix;
    switch(normalization)
    {
    case 0:
    {
      filter->SetNormalizationMethod(QballReconstructionImageFilterType::QBR_STANDARD);
      nodePostfix = "_Numerical_Qball";
      break;
    }
    case 1:
    {
      filter->SetNormalizationMethod(QballReconstructionImageFilterType::QBR_B_ZERO_B_VALUE);
      nodePostfix = "_Numerical_ZeroBvalueNormalization_Qball";
      break;
    }
    case 2:
    {
      filter->SetNormalizationMethod(QballReconstructionImageFilterType::QBR_B_ZERO);
      nodePostfix = "_NumericalQball_ZeroNormalization_Qball";
      break;
    }
    case 3:
    {
      filter->SetNormalizationMethod(QballReconstructionImageFilterType::QBR_NONE);
      nodePostfix = "_NumericalQball_NoNormalization_Qball";
      break;
    }
    default:
    {
      filter->SetNormalizationMethod(QballReconstructionImageFilterType::QBR_STANDARD);
      nodePostfix = "_NumericalQball_Qball";
    }
    }

    filter->Update();

    // ODFs TO DATATREE
    mitk::OdfImage::Pointer image = mitk::OdfImage::New();
    image->InitializeByItk( filter->GetOutput() );
    image->SetVolume( filter->GetOutput()->GetBufferPointer() );
    mitk::DataNode::Pointer new_node = mitk::DataNode::New();
    new_node->SetData( image );
    new_node->SetName(nodename+nodePostfix);
    mitk::ProgressBar::GetInstance()->Progress();

    GetDataStorage()->Add(new_node, node);

    this->GetRenderWindowPart()->RequestUpdate();
  }
  catch (itk::ExceptionObject &ex)
  {
    MITK_INFO << ex ;
    QMessageBox::information(nullptr, "Reconstruction not possible:", ex.GetDescription());
    return ;
  }
}

void QmitkQBallReconstructionView::AnalyticalQBallReconstruction( mitk::DataNode::Pointer node, int normalization)
{
  try
  {
    auto lambda = m_Controls->m_QBallReconstructionLambdaLineEdit->value();
    switch(m_Controls->m_QBallReconstructionMaxLLevelComboBox->currentIndex())
    {
    case 0:
    {
      TemplatedAnalyticalQBallReconstruction<2>(node, lambda, normalization);
      break;
    }
    case 1:
    {
      TemplatedAnalyticalQBallReconstruction<4>(node, lambda, normalization);
      break;
    }
    case 2:
    {
      TemplatedAnalyticalQBallReconstruction<6>(node, lambda, normalization);
      break;
    }
    case 3:
    {
      TemplatedAnalyticalQBallReconstruction<8>(node, lambda, normalization);
      break;
    }
    case 4:
    {
      TemplatedAnalyticalQBallReconstruction<10>(node, lambda, normalization);
      break;
    }
    case 5:
    {
      TemplatedAnalyticalQBallReconstruction<12>(node, lambda, normalization);
      break;
    }
    }

    this->GetRenderWindowPart()->RequestUpdate();
  }
  catch (itk::ExceptionObject &ex)
  {
    MITK_INFO << ex;
    QMessageBox::information(nullptr, "Reconstruction not possible:", ex.GetDescription());
    return;
  }
}

template<int L>
void QmitkQBallReconstructionView::TemplatedAnalyticalQBallReconstruction(mitk::DataNode* dataNodePointer, double lambda, int normalization)
{
  typedef itk::AnalyticalDiffusionQballReconstructionImageFilter
      <DiffusionPixelType,DiffusionPixelType,TTensorPixelType,L,ODF_SAMPLING_SIZE> FilterType;
  typename FilterType::Pointer filter = FilterType::New();

  mitk::Image* vols = dynamic_cast<mitk::Image*>(dataNodePointer->GetData());

  ITKDiffusionImageType::Pointer itkVectorImagePointer = ITKDiffusionImageType::New();
  mitk::CastToItkImage(vols, itkVectorImagePointer);

  filter->SetBValue(mitk::DiffusionPropertyHelper::GetReferenceBValue(vols));
  filter->SetGradientImage(mitk::DiffusionPropertyHelper::GetGradientContainer(vols), itkVectorImagePointer);
  filter->SetThreshold( static_cast<short>(m_Controls->m_QBallReconstructionThreasholdEdit->value()) );
  filter->SetLambda(lambda);

  std::string nodePostfix;
  switch(normalization)
  {
  case 0:
  {
    filter->SetNormalizationMethod(FilterType::QBAR_STANDARD);
    nodePostfix = "_Qball";
    break;
  }
  case 1:
  {
    filter->SetNormalizationMethod(FilterType::QBAR_B_ZERO_B_VALUE);
    nodePostfix = "_1_Qball";
    break;
  }
  case 2:
  {
    filter->SetNormalizationMethod(FilterType::QBAR_B_ZERO);
    nodePostfix = "_2_Qball";
    break;
  }
  case 3:
  {
    filter->SetNormalizationMethod(FilterType::QBAR_NONE);
    nodePostfix = "_3_Qball";
    break;
  }
  case 4:
  {
    filter->SetNormalizationMethod(FilterType::QBAR_ADC_ONLY);
    nodePostfix = "_AdcProfile";
    break;
  }
  case 5:
  {
    filter->SetNormalizationMethod(FilterType::QBAR_RAW_SIGNAL);
    nodePostfix = "_RawSignal";
    break;
  }
  case 6:
  {
    filter->SetNormalizationMethod(FilterType::QBAR_SOLID_ANGLE);
    nodePostfix = "_CSA_Qball";
    break;
  }
  case 7:
  {
    filter->SetNormalizationMethod(FilterType::QBAR_NONNEG_SOLID_ANGLE);
    nodePostfix = "_NonNegCSA_Qball";
    break;
  }
  default:
  {
    filter->SetNormalizationMethod(FilterType::QBAR_STANDARD);
  }
  }

  filter->Update();

  mitk::Image::Pointer coeffsImage = dynamic_cast<mitk::Image*>(mitk::ShImage::New().GetPointer());
  coeffsImage->InitializeByItk( filter->GetCoefficientImage().GetPointer() );
  coeffsImage->SetVolume( filter->GetCoefficientImage()->GetBufferPointer() );

  mitk::DataNode::Pointer coeffsNode = mitk::DataNode::New();
  coeffsNode->SetData( coeffsImage );
  coeffsNode->SetProperty( "name", mitk::StringProperty::New(dataNodePointer->GetName() + nodePostfix + "_SH") );
  GetDataStorage()->Add(coeffsNode, dataNodePointer);

  // ODFs TO DATATREE
  if(m_Controls->m_OutputOdfImage->isChecked())
  {
    mitk::OdfImage::Pointer image = mitk::OdfImage::New();
    image->InitializeByItk( filter->GetOutput() );
    image->SetVolume( filter->GetOutput()->GetBufferPointer() );
    mitk::DataNode::Pointer node=mitk::DataNode::New();
    node->SetData( image );
    node->SetName(dataNodePointer->GetName() + nodePostfix + "_Sampeld");

    GetDataStorage()->Add(node, coeffsNode);
  }
}

void QmitkQBallReconstructionView::MultiQBallReconstruction(mitk::DataNode::Pointer node)
{
  try
  {
    auto lambda  = m_Controls->m_QBallReconstructionLambdaLineEdit->value();
    switch(m_Controls->m_QBallReconstructionMaxLLevelComboBox->currentIndex())
    {
    case 0:
    {
      TemplatedMultiQBallReconstruction<2>(lambda, node);
      break;
    }
    case 1:
    {
      TemplatedMultiQBallReconstruction<4>(lambda, node);
      break;
    }
    case 2:
    {
      TemplatedMultiQBallReconstruction<6>(lambda, node);
      break;
    }
    case 3:
    {
      TemplatedMultiQBallReconstruction<8>(lambda, node);
      break;
    }
    case 4:
    {
      TemplatedMultiQBallReconstruction<10>(lambda, node);
      break;
    }
    case 5:
    {
      TemplatedMultiQBallReconstruction<12>(lambda, node);
      break;
    }
    }
  }
  catch (itk::ExceptionObject &ex)
  {
    MITK_INFO << ex ;
    QMessageBox::information(nullptr, "Reconstruction not possible:", ex.GetDescription());
    return ;
  }
}

template<int L>
void QmitkQBallReconstructionView::TemplatedMultiQBallReconstruction(double lambda, mitk::DataNode* dataNodePointer)
{
  typedef itk::DiffusionMultiShellQballReconstructionImageFilter
      <DiffusionPixelType,DiffusionPixelType,TTensorPixelType,L,ODF_SAMPLING_SIZE> FilterType;
  typename FilterType::Pointer filter = FilterType::New();

  std::string nodename;
  dataNodePointer->GetStringProperty("name",nodename);

  mitk::Image* dwi = dynamic_cast<mitk::Image*>(dataNodePointer->GetData());
  BValueMapType currSelectionMap = m_ShellSelectorMap[dataNodePointer]->GetBValueSelctionMap();

  if(currSelectionMap.size() != 4)// || currSelectionMap.find(0) != currSelectionMap.end())
  {
    QMessageBox::information(nullptr, "Reconstruction not possible:" ,QString("Only three equidistant shells are supported. (ImageName: " + QString(nodename.c_str()) + ")"));
    return;
  }

  BValueMapType::reverse_iterator it1 = currSelectionMap.rbegin();
  BValueMapType::reverse_iterator it2 = currSelectionMap.rbegin();
  ++it2;

  // Get average distance
  int avdistance = 0;
  for(; it2 != currSelectionMap.rend(); ++it1,++it2)
    avdistance += static_cast<int>(it1->first - it2->first);
  avdistance /= currSelectionMap.size()-1;

  // Check if all shells are using the same averae distance
  it1 = currSelectionMap.rbegin();
  it2 = currSelectionMap.rbegin();
  ++it2;
  for(; it2 != currSelectionMap.rend(); ++it1,++it2)
  {
    if(avdistance != static_cast<int>(it1->first - it2->first))
    {
      QMessageBox::information(nullptr, "Reconstruction not possible:" ,QString("Selected Shells are not in a equidistant configuration. (ImageName: " + QString(nodename.c_str()) + ")"));
      return;
    }
  }
  ITKDiffusionImageType::Pointer itkVectorImagePointer = ITKDiffusionImageType::New();
  mitk::CastToItkImage(dwi, itkVectorImagePointer);

  filter->SetBValueMap(m_ShellSelectorMap[dataNodePointer]->GetBValueSelctionMap());
  filter->SetGradientImage(mitk::DiffusionPropertyHelper::GetGradientContainer(dwi), itkVectorImagePointer, mitk::DiffusionPropertyHelper::GetReferenceBValue(dwi));
  filter->SetThreshold( static_cast<short>(m_Controls->m_QBallReconstructionThreasholdEdit->value()) );
  filter->SetLambda(lambda);
  filter->Update();


  mitk::Image::Pointer coeffsImage = dynamic_cast<mitk::Image*>(mitk::ShImage::New().GetPointer());
  coeffsImage->InitializeByItk( filter->GetCoefficientImage().GetPointer() );
  coeffsImage->SetVolume( filter->GetCoefficientImage()->GetBufferPointer() );

  mitk::DataNode::Pointer coeffsNode=mitk::DataNode::New();
  coeffsNode->SetData( coeffsImage );
  coeffsNode->SetProperty( "name", mitk::StringProperty::New(dataNodePointer->GetName() + "_SH_MultiShell_Qball_SH") );
  GetDataStorage()->Add(coeffsNode, dataNodePointer);

  if(m_Controls->m_OutputOdfImage->isChecked())
  {
    // ODFs TO DATATREE
    mitk::OdfImage::Pointer image = mitk::OdfImage::New();
    image->InitializeByItk( filter->GetOutput() );
    image->SetVolume( filter->GetOutput()->GetBufferPointer() );
    mitk::DataNode::Pointer node=mitk::DataNode::New();
    node->SetData( image );
    node->SetName(dataNodePointer->GetName() + "_SH_MultiShell_Qball_Sampled");

    GetDataStorage()->Add(node, coeffsNode);
  }
}

void QmitkQBallReconstructionView::GenerateShellSelectionUI(mitk::DataNode::Pointer node)
{
  std::map<const mitk::DataNode * , QbrShellSelection * > tempMap;

  if(m_ShellSelectorMap.find(  node.GetPointer() ) != m_ShellSelectorMap.end())
  {
    tempMap[node.GetPointer()] = m_ShellSelectorMap[node.GetPointer()];
    m_ShellSelectorMap.erase(node.GetPointer());
  }
  else
  {
    tempMap[node.GetPointer()] = new QbrShellSelection(this, node );
    tempMap[node.GetPointer()]->SetVisible(true);
  }

  for(std::map<const mitk::DataNode * , QbrShellSelection * >::iterator it = m_ShellSelectorMap.begin(); it != m_ShellSelectorMap.end();it ++)
  {
    delete it->second;
  }
  m_ShellSelectorMap.clear();
  m_ShellSelectorMap = tempMap;
}

void QmitkQBallReconstructionView::PreviewThreshold(short threshold)
{
  if (m_Controls->m_ImageBox->GetSelectedNode().IsNotNull())
  {
    mitk::Image* vols = static_cast<mitk::Image*>(m_Controls->m_ImageBox->GetSelectedNode()->GetData());

    // Extract b0 image
    ITKDiffusionImageType::Pointer itkVectorImagePointer = ITKDiffusionImageType::New();
    mitk::CastToItkImage(vols, itkVectorImagePointer);

    typedef itk::B0ImageExtractionImageFilter<short, short> FilterType;
    FilterType::Pointer filterB0 = FilterType::New();
    filterB0->SetInput( itkVectorImagePointer );
    filterB0->SetDirections(mitk::DiffusionPropertyHelper::GetGradientContainer(vols));
    filterB0->Update();

    mitk::Image::Pointer mitkImage = mitk::Image::New();

    typedef itk::Image<short, 3> ImageType;
    typedef itk::Image<short, 3> SegmentationType;
    typedef itk::BinaryThresholdImageFilter<ImageType, SegmentationType> ThresholdFilterType;
    // apply threshold
    ThresholdFilterType::Pointer filterThreshold = ThresholdFilterType::New();
    filterThreshold->SetInput(filterB0->GetOutput());
    filterThreshold->SetLowerThreshold(threshold);
    filterThreshold->SetInsideValue(0);
    filterThreshold->SetOutsideValue(1); // mark cut off values red
    filterThreshold->Update();

    mitkImage->InitializeByItk( filterThreshold->GetOutput() );
    mitkImage->SetVolume( filterThreshold->GetOutput()->GetBufferPointer() );
    mitk::DataNode::Pointer node;
    if (this->GetDataStorage()->GetNamedDerivedNode("ThresholdOverlay", m_Controls->m_ImageBox->GetSelectedNode()))
    {
      node = this->GetDataStorage()->GetNamedDerivedNode("ThresholdOverlay", m_Controls->m_ImageBox->GetSelectedNode());
    }
    else
    {
      // create a new node, to show thresholded values
      node = mitk::DataNode::New();
      GetDataStorage()->Add( node, m_Controls->m_ImageBox->GetSelectedNode() );
      node->SetProperty( "name", mitk::StringProperty::New("ThresholdOverlay"));
      node->SetBoolProperty("helper object", true);
    }
    node->SetData( mitkImage );
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}
