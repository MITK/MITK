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

#include "QmitkQBallReconstructionView.h"
#include "mitkDiffusionImagingConfigure.h"

// qt includes
#include <QMessageBox>

// itk includes
#include "itkTimeProbe.h"

// mitk includes
#include "mitkProgressBar.h"
#include "mitkStatusBar.h"

#include "mitkNodePredicateDataType.h"
#include "QmitkDataStorageComboBox.h"
#include "QmitkStdMultiWidget.h"

#include "itkDiffusionQballReconstructionImageFilter.h"
#include "itkAnalyticalDiffusionQballReconstructionImageFilter.h"
#include "itkVectorContainer.h"

#include "mitkQBallImage.h"
#include "mitkProperties.h"
#include "mitkVtkResliceInterpolationProperty.h"
#include "mitkLookupTable.h"
#include "mitkLookupTableProperty.h"
#include "mitkTransferFunction.h"
#include "mitkTransferFunctionProperty.h"
#include "mitkDataNodeObject.h"
#include "mitkOdfNormalizationMethodProperty.h"
#include "mitkOdfScaleByProperty.h"

#include "berryIStructuredSelection.h"
#include "berryIWorkbenchWindow.h"
#include "berryISelectionService.h"

const std::string QmitkQBallReconstructionView::VIEW_ID = 
  "org.mitk.views.qballreconstruction";

#define DI_INFO MITK_INFO("DiffusionImaging")


typedef float TTensorPixelType;

const int QmitkQBallReconstructionView::nrconvkernels = 252;

using namespace berry;

struct QbrSelListener : ISelectionListener
{

  berryObjectMacro(QbrSelListener);

  QbrSelListener(QmitkQBallReconstructionView* view)
  {
    m_View = view;
  }

  void DoSelectionChanged(ISelection::ConstPointer selection)
  {
    // save current selection in member variable
    m_View->m_CurrentSelection = selection.Cast<const IStructuredSelection>();

    // do something with the selected items
    if(m_View->m_CurrentSelection)
    {
      bool foundDwiVolume = false;

      // iterate selection
      for (IStructuredSelection::iterator i = m_View->m_CurrentSelection->Begin(); 
        i != m_View->m_CurrentSelection->End(); ++i)
      {

        // extract datatree node
        if (mitk::DataNodeObject::Pointer nodeObj = i->Cast<mitk::DataNodeObject>())
        {
          mitk::DataNode::Pointer node = nodeObj->GetDataNode();
          
          // only look at interesting types
          if(QString("DiffusionImage").compare(node->GetData()->GetNameOfClass())==0)
          {
            foundDwiVolume = true;
          }
        }
      }

      m_View->m_Controls->m_ButtonStandard->setEnabled(foundDwiVolume);
      m_View->m_Controls->m_ButtonAverageGradients->setEnabled(foundDwiVolume);
      m_View->m_Controls->m_ButtonExtractB0->setEnabled(foundDwiVolume);      
    }
  }

  void SelectionChanged(IWorkbenchPart::Pointer part, ISelection::ConstPointer selection)
  {
    // check, if selection comes from datamanager
    if (part)
    {
      QString partname(part->GetPartName().c_str());
      if(partname.compare("Datamanager")==0)
      {

        // apply selection
        DoSelectionChanged(selection);

      }
    }
  }

  QmitkQBallReconstructionView* m_View;
};

QmitkQBallReconstructionView::QmitkQBallReconstructionView()
: QmitkFunctionality(),
  m_Controls(NULL),
  m_MultiWidget(NULL)
{
}

//void QmitkQBallReconstructionView::OpactiyChanged(int value)
//{
//  if (m_CurrentSelection)
//  {
//    if (mitk::DataNodeObject::Pointer nodeObj = m_CurrentSelection->Begin()->Cast<mitk::DataNodeObject>())
//    {
//      mitk::DataNode::Pointer node = nodeObj->GetDataNode();
//      if(QString("DiffusionImage").compare(node->GetData()->GetNameOfClass())==0)
//      {
//        node->SetIntProperty("DisplayChannel", value);
//        mitk::RenderingManager::GetInstance()->RequestUpdateAll();
//      }
//    }
//  }
//}
//
//void QmitkQBallReconstructionView::OpactiyActionChanged()
//{
//    if (m_CurrentSelection)
//  {
//    if (mitk::DataNodeObject::Pointer nodeObj = m_CurrentSelection->Begin()->Cast<mitk::DataNodeObject>())
//    {
//      mitk::DataNode::Pointer node = nodeObj->GetDataNode();
//      if(QString("DiffusionImage").compare(node->GetData()->GetNameOfClass())==0)
//      {
//        int displayChannel = 0.0;
//        if(node->GetIntProperty("DisplayChannel", displayChannel))
//        {
//          m_OpacitySlider->setValue(displayChannel);
//        }
//      }
//    }
//  }
//    
//  MITK_INFO << "changed";
//}

QmitkQBallReconstructionView::~QmitkQBallReconstructionView()
{
  this->GetSite()->GetWorkbenchWindow()->GetSelectionService()->RemovePostSelectionListener(/*"org.mitk.views.datamanager",*/ m_SelListener);
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
    items << "2" << "4" << "6" << "8";
    m_Controls->m_QBallReconstructionMaxLLevelComboBox->addItems(items);
    m_Controls->m_QBallReconstructionMaxLLevelComboBox->setCurrentIndex(1);
    m_Controls->m_Description->setText("Spherical harmonics recon. (Desoteaux2007)");

    m_Controls->m_QBallReconstructionNumberThreadsSpinbox->setValue(8);

#ifndef DIFFUSION_IMAGING_EXTENDED
    m_Controls->m_QBallReconstructionMethodComboBox->removeItem(3);
#endif

    AdvancedCheckboxClicked();
    PreprocCheckboxClicked();
    // define data type for combobox
    //m_Controls->m_ImageSelector->SetDataStorage( this->GetDefaultDataStorage() );
    //m_Controls->m_ImageSelector->SetPredicate( mitk::NodePredicateDataType::New("DiffusionImage") );
  }

  m_SelListener = berry::ISelectionListener::Pointer(new QbrSelListener(this));
  this->GetSite()->GetWorkbenchWindow()->GetSelectionService()->AddPostSelectionListener(/*"org.mitk.views.datamanager",*/ m_SelListener);
  berry::ISelection::ConstPointer sel(
    this->GetSite()->GetWorkbenchWindow()->GetSelectionService()->GetSelection("org.mitk.views.datamanager"));
  m_CurrentSelection = sel.Cast<const IStructuredSelection>();
  m_SelListener.Cast<QbrSelListener>()->DoSelectionChanged(sel);
}

void QmitkQBallReconstructionView::StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget)
{
  m_MultiWidget = &stdMultiWidget;
}

void QmitkQBallReconstructionView::StdMultiWidgetNotAvailable()
{
  m_MultiWidget = NULL;
}

void QmitkQBallReconstructionView::CreateConnections()
{
  if ( m_Controls )
  {
    connect( (QObject*)(m_Controls->m_PreprocessingToggle), SIGNAL(clicked()), this, SLOT(PreprocCheckboxClicked()) );
    connect( (QObject*)(m_Controls->m_ButtonAverageGradients), SIGNAL(clicked()), this, SLOT(AverageGradients()) );
    connect( (QObject*)(m_Controls->m_ButtonStandard), SIGNAL(clicked()), this, SLOT(ReconstructStandard()) );
    connect( (QObject*)(m_Controls->m_QBallReconstructionLambdaMultiCheckbox), SIGNAL(clicked()), this, SLOT(MultiLambdasClicked()) );
    connect( (QObject*)(m_Controls->m_AdvancedCheckbox), SIGNAL(clicked()), this, SLOT(AdvancedCheckboxClicked()) );
    connect( (QObject*)(m_Controls->m_QBallReconstructionMethodComboBox), SIGNAL(currentIndexChanged(int)), this, SLOT(MethodChoosen(int)) );
    connect( (QObject*)(m_Controls->m_ButtonExtractB0), SIGNAL(clicked()), this, SLOT(ExtractB0()) );

  }
}


void QmitkQBallReconstructionView::ExtractB0()
{
  if (m_CurrentSelection)
  {
    mitk::DataStorage::SetOfObjects::Pointer set =
      mitk::DataStorage::SetOfObjects::New();

    int at = 0;
    for (IStructuredSelection::iterator i = m_CurrentSelection->Begin(); 
      i != m_CurrentSelection->End(); 
      ++i)
    {

      if (mitk::DataNodeObject::Pointer nodeObj = i->Cast<mitk::DataNodeObject>())
      {
        mitk::DataNode::Pointer node = nodeObj->GetDataNode();
        if(QString("DiffusionImage").compare(node->GetData()->GetNameOfClass())==0)
        {
          set->InsertElement(at++, node);
        }
      }
    }

    DoExtractB0(set);

  }
}

void QmitkQBallReconstructionView::Activated()
{
  QmitkFunctionality::Activated();

  berry::ISelection::ConstPointer sel(
    this->GetSite()->GetWorkbenchWindow()->GetSelectionService()->GetSelection("org.mitk.views.datamanager"));
  m_CurrentSelection = sel.Cast<const IStructuredSelection>();
  m_SelListener.Cast<QbrSelListener>()->DoSelectionChanged(sel);
}

void QmitkQBallReconstructionView::Deactivated()
{
  QmitkFunctionality::Deactivated();
}

void QmitkQBallReconstructionView::PreprocCheckboxClicked()
{
  m_Controls->groupBox_4->setVisible(m_Controls->
    m_PreprocessingToggle->isChecked());  
}

void QmitkQBallReconstructionView::ReconstructStandard()
{

  int index = m_Controls->m_QBallReconstructionMethodComboBox->currentIndex();

#ifndef DIFFUSION_IMAGING_EXTENDED
    if(index>=3)
    {
      index = index + 1;
    }
#endif

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
      // Constrained Solid Angle
      Reconstruct(1,7);
      break;
    }
  case 4:
    {
      // ADC
      Reconstruct(1,4);
      break;
    }
  case 5:
    {
      // Raw Signal
      Reconstruct(1,5);
      break;
    }
  }
}

void QmitkQBallReconstructionView::MethodChoosen(int method)
{
  switch(method)
  {
  case 0:
    m_Controls->m_Description->setText("Numerical recon. (Tuch2004)");
    break;
  case 1:
    m_Controls->m_Description->setText("Spherical harmonics recon. (Desoteaux2007)");
    break;
  case 2:
    m_Controls->m_Description->setText("SH recon. with solid angle consideration (Aganj2009)");
    break;
  case 3:
    m_Controls->m_Description->setText("SH solid angle with non-neg. constraint (Goh2009)");
    break;
  case 4:
    m_Controls->m_Description->setText("SH recon. of the plain ADC-profiles");
    break;
  case 5:
    m_Controls->m_Description->setText("SH recon. of the raw diffusion signal");
    break;
  }
}


void QmitkQBallReconstructionView::MultiLambdasClicked()
{
  bool check = m_Controls->
    m_QBallReconstructionLambdaMultiCheckbox->isChecked();
  
  m_Controls->textLabel1_2->setEnabled(check);
  m_Controls->m_QBallReconstructionLambdaStepLineEdit->setEnabled(check);
  m_Controls->textLabel1_3->setEnabled(check);
  m_Controls->m_QBallReconstructionLambdaMaxLineEdit->setEnabled(check);

}

void QmitkQBallReconstructionView::AdvancedCheckboxClicked()
{
  bool check = m_Controls->
    m_AdvancedCheckbox->isChecked();
  
  m_Controls->m_QBallReconstructionMaxLLevelTextLabel_2->setVisible(check);
  m_Controls->m_QBallReconstructionMaxLLevelComboBox->setVisible(check);
  m_Controls->m_QBallReconstructionLambdaTextLabel_2->setVisible(check);
  m_Controls->m_QBallReconstructionLambdaLineEdit->setVisible(check);
  m_Controls->m_QBallReconstructionLambdaMultiCheckbox->setVisible(check);

  m_Controls->m_QBallReconstructionThresholdLabel_2->setVisible(check);
  m_Controls->m_QBallReconstructionThreasholdEdit->setVisible(check);
  m_Controls->m_OutputB0Image->setVisible(check);
  m_Controls->m_QBallReconstructionNumberThreadsLabel_2->setVisible(check);
  m_Controls->m_QBallReconstructionNumberThreadsSpinbox->setVisible(check);
  m_Controls->label_2->setVisible(check);

  m_Controls->textLabel1_2->setVisible(check);
  m_Controls->m_QBallReconstructionLambdaStepLineEdit->setVisible(check);
  m_Controls->textLabel1_3->setVisible(check);
  m_Controls->m_QBallReconstructionLambdaMaxLineEdit->setVisible(check);

  m_Controls->frame_2->setVisible(check);
}

void QmitkQBallReconstructionView::AverageGradients()
{

  if (m_CurrentSelection)
  {
    mitk::DataStorage::SetOfObjects::Pointer set =
      mitk::DataStorage::SetOfObjects::New();

    int at = 0;
    for (IStructuredSelection::iterator i = m_CurrentSelection->Begin(); 
      i != m_CurrentSelection->End(); 
      ++i)
    {

      if (mitk::DataNodeObject::Pointer nodeObj = i->Cast<mitk::DataNodeObject>())
      {
        mitk::DataNode::Pointer node = nodeObj->GetDataNode();
        if(QString("DiffusionImage").compare(node->GetData()->GetNameOfClass())==0)
        {
          set->InsertElement(at++, node);
        }
      }
    }

    DoAverageGradients(set);

  }

}

void QmitkQBallReconstructionView::DoExtractB0
  (mitk::DataStorage::SetOfObjects::Pointer inImages)
{
  typedef mitk::DiffusionImage<DiffusionPixelType>              DiffusionImageType;
  typedef DiffusionImageType::GradientDirectionContainerType    GradientContainerType;

  int nrFiles = inImages->size();
  if (!nrFiles) return;

  mitk::DataStorage::SetOfObjects::const_iterator itemiter( inImages->begin() ); 
  mitk::DataStorage::SetOfObjects::const_iterator itemiterend( inImages->end() ); 

  std::vector<mitk::DataNode::Pointer> nodes;
  while ( itemiter != itemiterend ) // for all items
  {

    DiffusionImageType* vols = 
      static_cast<DiffusionImageType*>(
      (*itemiter)->GetData());

    std::string nodename;
    (*itemiter)->GetStringProperty("name", nodename);

    // Extract B0
    GradientContainerType::Pointer dirs = vols->GetDirections();
    
    GradientContainerType::Iterator begin = dirs->Begin();
    GradientContainerType::Iterator end = dirs->End();


    // Find the index of the b0 image
    std::vector<int> indices;
    int index = 0;
    while(begin!=end)
    {
      DiffusionImageType::GradientDirectionType grad = begin->Value();
      
      if(grad[0] == 0 && grad[1] == 0 && grad[2] == 0)
      {
        indices.push_back(index);        
      }
      ++index;
      ++begin;
    }


    // Extract image using found index
    typedef itk::Image<DiffusionImageType::PixelType,3> SingleImageType;
    SingleImageType::Pointer b0Image = SingleImageType::New();
    

    DiffusionImageType::ImageType::Pointer vectorImage = vols->GetVectorImage();

    SingleImageType::RegionType region = vectorImage->GetLargestPossibleRegion();   
    
    b0Image->SetRegions(region);
    b0Image->Allocate();

    b0Image->SetSpacing(vectorImage->GetSpacing());
    b0Image->SetOrigin(vectorImage->GetOrigin());
    b0Image->SetDirection(vectorImage->GetDirection());

    itk::ImageRegionIterator<SingleImageType> it(b0Image, b0Image->GetLargestPossibleRegion() );
    itk::ImageRegionIterator<DiffusionImageType::ImageType> vectorIt(vectorImage, vectorImage->GetLargestPossibleRegion() );

    //Sum all images that have zero diffusion weighting (indices stored in vector index)
    for(std::vector<int>::iterator indexIt = indices.begin(); 
        indexIt != indices.end();
        indexIt++)
    {
      it.GoToBegin();
      vectorIt.GoToBegin();
      
      while(!it.IsAtEnd() && !vectorIt.IsAtEnd())
      {    
        DiffusionImageType::ImageType::PixelType vec = vectorIt.Get();
        it.Set(it.Get() + vec[*indexIt]/indices.size());        
        ++it;
        ++vectorIt;
      }      
    }    

    mitk::Image::Pointer mitkImage = mitk::Image::New();
    mitkImage->InitializeByItk( b0Image.GetPointer() );
    mitkImage->SetVolume( b0Image->GetBufferPointer() );
    mitk::DataNode::Pointer node=mitk::DataNode::New();
    node->SetData( mitkImage );
    node->SetProperty( "name", mitk::StringProperty::New(nodename + "_B0"));
    
    GetDefaultDataStorage()->Add(node);

    ++itemiter;
  }

 
}

void QmitkQBallReconstructionView::DoAverageGradients
  (mitk::DataStorage::SetOfObjects::Pointer inImages) 
{
    int nrFiles = inImages->size();
    if (!nrFiles) return;

    mitk::DataStorage::SetOfObjects::const_iterator itemiter( inImages->begin() ); 
    mitk::DataStorage::SetOfObjects::const_iterator itemiterend( inImages->end() ); 

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

void QmitkQBallReconstructionView::Reconstruct(int method, int normalization)
{
  if (m_CurrentSelection)
  {
    mitk::DataStorage::SetOfObjects::Pointer set =
      mitk::DataStorage::SetOfObjects::New();

    int at = 0;
    for (IStructuredSelection::iterator i = m_CurrentSelection->Begin(); 
      i != m_CurrentSelection->End(); 
      ++i)
    {

      if (mitk::DataNodeObject::Pointer nodeObj = i->Cast<mitk::DataNodeObject>())
      {
        mitk::DataNode::Pointer node = nodeObj->GetDataNode();
        if(QString("DiffusionImage").compare(node->GetData()->GetNameOfClass())==0)
        {
          set->InsertElement(at++, node);
        }
      }
    }

    if(method == 0)
    {
      NumericalQBallReconstruction(set, normalization);
    }

    if(method == 1)
    {
      AnalyticalQBallReconstruction(set, normalization);
    }

  }
}

void QmitkQBallReconstructionView::NumericalQBallReconstruction
  (mitk::DataStorage::SetOfObjects::Pointer inImages, int normalization) 
{
  try
  {
    itk::TimeProbe clock;

    int nrFiles = inImages->size();
    if (!nrFiles) return;

    QString status;
    mitk::ProgressBar::GetInstance()->AddStepsToDo(nrFiles);

    mitk::DataStorage::SetOfObjects::const_iterator itemiter( inImages->begin() ); 
    mitk::DataStorage::SetOfObjects::const_iterator itemiterend( inImages->end() ); 

    std::vector<mitk::DataNode::Pointer> nodes;
    while ( itemiter != itemiterend ) // for all items
    {

      mitk::DiffusionImage<DiffusionPixelType>* vols = 
        static_cast<mitk::DiffusionImage<DiffusionPixelType>*>(
        (*itemiter)->GetData());
      
      std::string nodename;
      (*itemiter)->GetStringProperty("name", nodename);
      ++itemiter;

      // QBALL RECONSTRUCTION
      clock.Start();
      std::cout << "QBall reconstruction ";
      mitk::StatusBar::GetInstance()->DisplayText(status.sprintf(
        "QBall reconstruction for %s", nodename.c_str()).toAscii());

      typedef itk::DiffusionQballReconstructionImageFilter
        <DiffusionPixelType, DiffusionPixelType, TTensorPixelType, QBALL_ODFSIZE>
        QballReconstructionImageFilterType;
      
      QballReconstructionImageFilterType::Pointer filter = 
        QballReconstructionImageFilterType::New();
      filter->SetGradientImage( vols->GetDirections(), vols->GetVectorImage() );
      filter->SetNumberOfThreads( m_Controls->m_QBallReconstructionNumberThreadsSpinbox->value() );
      filter->SetBValue(vols->GetB_Value());  
      filter->SetThreshold( m_Controls->m_QBallReconstructionThreasholdEdit->text().toFloat() );

      switch(normalization)
      {
      case 0:
        {
          filter->SetNormalizationMethod(QballReconstructionImageFilterType::QBR_STANDARD);
          break;
        }
      case 1:
        {
          filter->SetNormalizationMethod(QballReconstructionImageFilterType::QBR_B_ZERO_B_VALUE);
          break;
        }
      case 2:
        {
          filter->SetNormalizationMethod(QballReconstructionImageFilterType::QBR_B_ZERO);
          break;
        }
      case 3:
        {
          filter->SetNormalizationMethod(QballReconstructionImageFilterType::QBR_NONE);
          break;
        }
      default:
        {
          filter->SetNormalizationMethod(QballReconstructionImageFilterType::QBR_STANDARD);
        }
      }

      filter->Update();
      clock.Stop();
      std::cout << "took " << clock.GetMeanTime() << "s." << std::endl;

      // ODFs TO DATATREE
      mitk::QBallImage::Pointer image = mitk::QBallImage::New();
      image->InitializeByItk( filter->GetOutput() );
      //image->SetImportVolume( filter->GetOutput()->GetBufferPointer(), 0, 0, mitk::Image::ImportMemoryManagementType::ManageMemory );
      image->SetVolume( filter->GetOutput()->GetBufferPointer() );
      mitk::DataNode::Pointer node=mitk::DataNode::New();
      node->SetData( image );
      QString newname;
      newname = newname.append(nodename.c_str());
      newname = newname.append("_QN%1").arg(normalization);
      SetDefaultNodeProperties(node, newname.toStdString());
      nodes.push_back(node);
      
      // B-Zero TO DATATREE
      if(m_Controls->m_OutputB0Image->isChecked())
      {
        mitk::Image::Pointer image4 = mitk::Image::New();
        image4->InitializeByItk( filter->GetBZeroImage().GetPointer() );
        image4->SetVolume( filter->GetBZeroImage()->GetBufferPointer() );
        mitk::DataNode::Pointer node4=mitk::DataNode::New();
        node4->SetData( image4 );
        node4->SetProperty( "name", mitk::StringProperty::New(
          QString(nodename.c_str()).append("_b0").toStdString()) );
        nodes.push_back(node4);
      }
      mitk::ProgressBar::GetInstance()->Progress();

    }

    std::vector<mitk::DataNode::Pointer>::iterator nodeIt;
    for(nodeIt = nodes.begin(); nodeIt != nodes.end(); ++nodeIt)
      GetDefaultDataStorage()->Add(*nodeIt);

    mitk::StatusBar::GetInstance()->DisplayText(status.sprintf("Finished Processing %d Files", nrFiles).toAscii());
    m_MultiWidget->RequestUpdate();

  }
  catch (itk::ExceptionObject &ex)
  {
    std::cout << ex << std::endl;
    return ;
  }
}

void QmitkQBallReconstructionView::AnalyticalQBallReconstruction(
  mitk::DataStorage::SetOfObjects::Pointer inImages, 
  int normalization) 
{
  try
  {
    itk::TimeProbe clock;

    int nrFiles = inImages->size();
    if (!nrFiles) return;

    std::vector<float> lambdas;
    float minLambda  = m_Controls->m_QBallReconstructionLambdaLineEdit->text().toFloat();
    if(m_Controls->m_QBallReconstructionLambdaMultiCheckbox->isChecked())
    {
      float stepLambda = m_Controls->m_QBallReconstructionLambdaStepLineEdit->text().toFloat();
      float maxLambda  = m_Controls->m_QBallReconstructionLambdaMaxLineEdit->text().toFloat();
      for(float l=minLambda; l<maxLambda; l+=stepLambda)
      {
        lambdas.push_back(l);
      }
    }
    int nLambdas = lambdas.size();
    if(nLambdas == 0)
    {
      lambdas.push_back(minLambda);
      nLambdas = 1;
    }

    QString status;
    mitk::ProgressBar::GetInstance()->AddStepsToDo(nrFiles*nLambdas);

    mitk::DataStorage::SetOfObjects::const_iterator itemiter( inImages->begin() ); 
    mitk::DataStorage::SetOfObjects::const_iterator itemiterend( inImages->end() ); 

    std::vector<mitk::DataNode::Pointer>* nodes
      = new std::vector<mitk::DataNode::Pointer>();
    while ( itemiter != itemiterend ) // for all items
    {

      mitk::DiffusionImage<DiffusionPixelType>* vols = 
        static_cast<mitk::DiffusionImage<DiffusionPixelType>*>(
        (*itemiter)->GetData());

      std::string nodename;
      (*itemiter)->GetStringProperty("name",nodename);
      itemiter++;

      // QBALL RECONSTRUCTION
      clock.Start();
      std::cout << "QBall reconstruction ";
      mitk::StatusBar::GetInstance()->DisplayText(status.sprintf(
        "QBall reconstruction for %s", nodename.c_str()).toAscii());

      for(int i=0; i<nLambdas; i++)
      {

        float currentLambda = lambdas[i];

        switch(m_Controls->m_QBallReconstructionMaxLLevelComboBox->currentIndex())
        {
        case 0:
          {
            TemplatedAnalyticalQBallReconstruction<2>(vols, currentLambda, nodename, nodes, normalization);
            break;
          }
        case 1:
          {
            TemplatedAnalyticalQBallReconstruction<4>(vols, currentLambda, nodename, nodes, normalization);
            break;
          }
        case 2:
          {
            TemplatedAnalyticalQBallReconstruction<6>(vols, currentLambda, nodename, nodes, normalization);
            break;
          }
        case 3:
          {
            TemplatedAnalyticalQBallReconstruction<8>(vols, currentLambda, nodename, nodes, normalization);
            break;
          }
        }

        clock.Stop();
        std::cout << "took " << clock.GetMeanTime() << "s." << std::endl;
        mitk::ProgressBar::GetInstance()->Progress();

      }
    }

    std::vector<mitk::DataNode::Pointer>::iterator nodeIt;
    for(nodeIt = nodes->begin(); nodeIt != nodes->end(); ++nodeIt)
      GetDefaultDataStorage()->Add(*nodeIt);

    m_MultiWidget->RequestUpdate();

    mitk::StatusBar::GetInstance()->DisplayText(status.sprintf("Finished Processing %d Files", nrFiles).toAscii());

  }
  catch (itk::ExceptionObject &ex)
  {
    std::cout << ex << std::endl;
    return ;
  }
}

template<int L>
void QmitkQBallReconstructionView::TemplatedAnalyticalQBallReconstruction(
  mitk::DiffusionImage<DiffusionPixelType>* vols, float lambda,
  std::string nodename, std::vector<mitk::DataNode::Pointer>* nodes, 
  int normalization) 
{
  typedef itk::AnalyticalDiffusionQballReconstructionImageFilter
    <DiffusionPixelType,DiffusionPixelType,TTensorPixelType,L,QBALL_ODFSIZE> FilterType;
  typename FilterType::Pointer filter = FilterType::New();
  filter->SetGradientImage( vols->GetDirections(), vols->GetVectorImage() );
  filter->SetNumberOfThreads( m_Controls->m_QBallReconstructionNumberThreadsSpinbox->value() );
  filter->SetBValue(vols->GetB_Value());  
  filter->SetThreshold( m_Controls->m_QBallReconstructionThreasholdEdit->text().toFloat() );
  filter->SetLambda(lambda);

  switch(normalization)
  {
  case 0:
    {
      filter->SetNormalizationMethod(FilterType::QBAR_STANDARD);
      break;
    }
  case 1:
    {
      filter->SetNormalizationMethod(FilterType::QBAR_B_ZERO_B_VALUE);
      break;
    }            
  case 2:
    {
      filter->SetNormalizationMethod(FilterType::QBAR_B_ZERO);
      break;
    }
  case 3:
    {
      filter->SetNormalizationMethod(FilterType::QBAR_NONE);
      break;
    }
  case 4:
    {
      filter->SetNormalizationMethod(FilterType::QBAR_ADC_ONLY);
      break;
    }
  case 5:
    {
      filter->SetNormalizationMethod(FilterType::QBAR_RAW_SIGNAL);
      break;
    }
  case 6:
    {
      filter->SetNormalizationMethod(FilterType::QBAR_SOLID_ANGLE);
      break;
    }
  case 7:
    {
      filter->SetNormalizationMethod(FilterType::QBAR_NONNEG_SOLID_ANGLE);
      break;
    }
  default:
    {
      filter->SetNormalizationMethod(FilterType::QBAR_STANDARD);
    }
  }

  filter->Update();

  // ODFs TO DATATREE
  mitk::QBallImage::Pointer image = mitk::QBallImage::New();
  image->InitializeByItk( filter->GetOutput() );
  image->SetVolume( filter->GetOutput()->GetBufferPointer() );
  mitk::DataNode::Pointer node=mitk::DataNode::New();
  node->SetData( image );
  QString newname;
  newname = newname.append(nodename.c_str());
  newname = newname.append("_QA%1").arg(normalization);
  SetDefaultNodeProperties(node, newname.toStdString());
  nodes->push_back(node);
  
  // B-Zero TO DATATREE
  if(m_Controls->m_OutputB0Image->isChecked())
  {
    mitk::Image::Pointer image4 = mitk::Image::New();
    image4->InitializeByItk( filter->GetBZeroImage().GetPointer() );
    image4->SetVolume( filter->GetBZeroImage()->GetBufferPointer() );
    mitk::DataNode::Pointer node4=mitk::DataNode::New();
    node4->SetData( image4 );
    node4->SetProperty( "name", mitk::StringProperty::New(
      QString(nodename.c_str()).append("_b0").toStdString()) );
    nodes->push_back(node4);
  }  
}



void QmitkQBallReconstructionView::SetDefaultNodeProperties(mitk::DataNode::Pointer node, std::string name)
{
  node->SetProperty( "ShowMaxNumber", mitk::IntProperty::New( 500 ) );
  node->SetProperty( "Scaling", mitk::FloatProperty::New( 1.0 ) );
  node->SetProperty( "Normalization", mitk::OdfNormalizationMethodProperty::New());
  node->SetProperty( "ScaleBy", mitk::OdfScaleByProperty::New());
  node->SetProperty( "IndexParam1", mitk::FloatProperty::New(2));
  node->SetProperty( "IndexParam2", mitk::FloatProperty::New(1));
  node->SetProperty( "visible", mitk::BoolProperty::New( true ) );
  node->SetProperty( "VisibleOdfs", mitk::BoolProperty::New( false ) );
  node->SetProperty ("layer", mitk::IntProperty::New(100));
  node->SetProperty( "DoRefresh", mitk::BoolProperty::New( true ) );
  //node->SetProperty( "opacity", mitk::FloatProperty::New(1.0f) );

  node->SetProperty( "name", mitk::StringProperty::New(name) );

}


  //node->SetProperty( "volumerendering", mitk::BoolProperty::New( false ) );
  //node->SetProperty( "use color", mitk::BoolProperty::New( true ) );
  //node->SetProperty( "texture interpolation", mitk::BoolProperty::New( true ) );
  //node->SetProperty( "reslice interpolation", mitk::VtkResliceInterpolationProperty::New() );
  //node->SetProperty( "layer", mitk::IntProperty::New(0));
  //node->SetProperty( "in plane resample extent by geometry", mitk::BoolProperty::New( false ) );
  //node->SetOpacity(1.0f);
  //node->SetColor(1.0,1.0,1.0);  
  //node->SetVisibility(true);
  //node->SetProperty( "IsQBallVolume", mitk::BoolProperty::New( true ) );

  //mitk::LevelWindowProperty::Pointer levWinProp = mitk::LevelWindowProperty::New();
  //mitk::LevelWindow levelwindow;
  ////  levelwindow.SetAuto( image );
  //levWinProp->SetLevelWindow( levelwindow );
  //node->GetPropertyList()->SetProperty( "levelwindow", levWinProp );

  //// add a default rainbow lookup table for color mapping
  //if(!node->GetProperty("LookupTable"))
  //{
  //  mitk::LookupTable::Pointer mitkLut = mitk::LookupTable::New();
  //  vtkLookupTable* vtkLut = mitkLut->GetVtkLookupTable();
  //  vtkLut->SetHueRange(0.6667, 0.0);
  //  vtkLut->SetTableRange(0.0, 20.0);
  //  vtkLut->Build();
  //  mitk::LookupTableProperty::Pointer mitkLutProp = mitk::LookupTableProperty::New();
  //  mitkLutProp->SetLookupTable(mitkLut);
  //  node->SetProperty( "LookupTable", mitkLutProp );
  //}
  //if(!node->GetProperty("binary"))
  //  node->SetProperty( "binary", mitk::BoolProperty::New( false ) );

  //// add a default transfer function
  //mitk::TransferFunction::Pointer tf = mitk::TransferFunction::New();
  //node->SetProperty ( "TransferFunction", mitk::TransferFunctionProperty::New ( tf.GetPointer() ) );

  //// set foldername as string property
  //mitk::StringProperty::Pointer nameProp = mitk::StringProperty::New( name );
  //node->SetProperty( "name", nameProp );