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
#include "itkDiffusionMultiShellQballReconstructionImageFilter.h"
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

#include <boost/version.hpp>

const std::string QmitkQBallReconstructionView::VIEW_ID = "org.mitk.views.qballreconstruction";


typedef float TTensorPixelType;

const int QmitkQBallReconstructionView::nrconvkernels = 252;


struct QbrShellSelection
{
  QmitkQBallReconstructionView* m_View;
  mitk::DataNode * m_Node;
  std::string m_NodeName;

  std::vector<QCheckBox *> m_CheckBoxes;
  QLabel * m_Label;

  mitk::DiffusionImage<DiffusionPixelType> * m_Image;
  typedef mitk::DiffusionImage<DiffusionPixelType>::BValueMap  BValueMap;

  QbrShellSelection(QmitkQBallReconstructionView* view, mitk::DataNode * node)
    : m_View(view),
      m_Node(node),
      m_NodeName(node->GetName())
  {
    m_Image = dynamic_cast<mitk::DiffusionImage<DiffusionPixelType> * > (node->GetData());
    if(!m_Image){MITK_INFO << "QmitkQBallReconstructionView::QbrShellSelection : fail to initialize DiffusionImage "; return;}

    GenerateCheckboxes();

  }

  void GenerateCheckboxes()
  {
    BValueMap origMap = m_Image->GetB_ValueMap();
    BValueMap::iterator itStart = origMap.begin();
    itStart++;
    BValueMap::iterator itEnd = origMap.end();

    m_Label = new QLabel(m_NodeName.c_str());
    m_Label->setVisible(true);
    m_View->m_Controls->m_QBallSelectionBox->layout()->addWidget(m_Label);

    for(BValueMap::iterator it = itStart ; it!= itEnd; it++)
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

  BValueMap GetBValueSelctionMap()
  {
    BValueMap inputMap = m_Image->GetB_ValueMap();
    BValueMap outputMap;

    double val = 0;

    if(inputMap.find(0) == inputMap.end()){
      MITK_INFO << "QbrShellSelection: return empty BValueMap from GUI Selection";
      return outputMap;
    }else{
      outputMap[val] = inputMap[val];
      MITK_INFO << val;
    }

    foreach(QCheckBox * box, m_CheckBoxes)
    {
      if(box->isChecked()){
        val = box->text().toDouble();
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

      m_View->m_Controls->m_DiffusionImageLabel->setText("<font color='red'>mandatory</font>");
      m_View->m_Controls->m_InputData->setTitle("Please Select Input Data");

      QString selected_images = "";

      mitk::DataStorage::SetOfObjects::Pointer set =
          mitk::DataStorage::SetOfObjects::New();

      int at = 0;

      // iterate selection
      for (IStructuredSelection::iterator i = m_View->m_CurrentSelection->Begin();
           i != m_View->m_CurrentSelection->End(); ++i)
      {

        // extract datatree node
        if (mitk::DataNodeObject::Pointer nodeObj = i->Cast<mitk::DataNodeObject>())
        {
          mitk::DataNode::Pointer node = nodeObj->GetDataNode();
          mitk::DiffusionImage<DiffusionPixelType>* diffusionImage;
          // only look at interesting types
          if(diffusionImage = dynamic_cast<mitk::DiffusionImage<DiffusionPixelType> * >(node->GetData()))
          {
            foundDwiVolume = true;
            selected_images += QString(node->GetName().c_str());
            if(i + 1 != m_View->m_CurrentSelection->End())
              selected_images += "\n";
            set->InsertElement(at++, node);
          }
        }
      }
      m_View->GenerateShellSelectionUI(set);
      m_View->m_Controls->m_DiffusionImageLabel->setText(selected_images);
      m_View->m_Controls->m_ButtonStandard->setEnabled(foundDwiVolume);
      if (foundDwiVolume)
        m_View->m_Controls->m_InputData->setTitle("Input Data");
      else
        m_View->m_Controls->m_DiffusionImageLabel->setText("<font color='red'>mandatory</font>");
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


// --------------- QmitkQBallReconstructionView----------------- //


QmitkQBallReconstructionView::QmitkQBallReconstructionView()
  : QmitkFunctionality(),
    m_Controls(NULL),
    m_MultiWidget(NULL)
{
}

QmitkQBallReconstructionView::QmitkQBallReconstructionView(const QmitkQBallReconstructionView& other)
{
  Q_UNUSED(other);
  throw std::runtime_error("Copy constructor not implemented");
}

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

    m_Controls->m_DiffusionImageLabel->setText("<font color='red'>mandatory</font>");

    QStringList items;
    items << "2" << "4" << "6" << "8" << "10" << "12";
    m_Controls->m_QBallReconstructionMaxLLevelComboBox->addItems(items);
    m_Controls->m_QBallReconstructionMaxLLevelComboBox->setCurrentIndex(1);
    MethodChoosen(m_Controls->m_QBallReconstructionMethodComboBox->currentIndex());


#ifndef DIFFUSION_IMAGING_EXTENDED
    m_Controls->m_QBallReconstructionMethodComboBox->removeItem(3);
#endif

    AdvancedCheckboxClicked();
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
    connect( (QObject*)(m_Controls->m_ButtonStandard), SIGNAL(clicked()), this, SLOT(ReconstructStandard()) );
    connect( (QObject*)(m_Controls->m_AdvancedCheckbox), SIGNAL(clicked()), this, SLOT(AdvancedCheckboxClicked()) );
    connect( (QObject*)(m_Controls->m_QBallReconstructionMethodComboBox), SIGNAL(currentIndexChanged(int)), this, SLOT(MethodChoosen(int)) );

  }
}

void QmitkQBallReconstructionView::OnSelectionChanged( std::vector<mitk::DataNode*> nodes )
{

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
  case 6:
  {
    // Q-Ball reconstruction
    Reconstruct(2,0);
    break;
  }
  }
}

void QmitkQBallReconstructionView::MethodChoosen(int method)
{

#ifndef DIFFUSION_IMAGING_EXTENDED
  if(method>=3)
  {
    method = method + 1;
  }
#endif

  m_Controls->m_QBallSelectionBox->setHidden(true);
  m_Controls->m_OutputCoeffsImage->setHidden(true);

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
    m_Controls->m_OutputCoeffsImage->setHidden(false);
    break;
  case 2:
    m_Controls->m_Description->setText("SH recon. with solid angle consideration (Aganj 2009)");
    m_Controls->m_OutputCoeffsImage->setHidden(false);
    break;
  case 3:
    m_Controls->m_Description->setText("SH solid angle with non-neg. constraint (Goh 2009)");
    break;
  case 4:
    m_Controls->m_Description->setText("SH recon. of the plain ADC-profiles");
    break;
  case 5:
    m_Controls->m_Description->setText("SH recon. of the raw diffusion signal");
    break;
  case 6:
    m_Controls->m_Description->setText("SH recon. of the multi shell diffusion signal (Aganj 2010)");
    m_Controls->m_QBallSelectionBox->setHidden(false);
    m_Controls->m_OutputCoeffsImage->setHidden(false);
    break;
  }
}



void QmitkQBallReconstructionView::AdvancedCheckboxClicked()
{
  bool check = m_Controls->m_AdvancedCheckbox->isChecked();

  m_Controls->m_QBallReconstructionMaxLLevelTextLabel_2->setVisible(check);
  m_Controls->m_QBallReconstructionMaxLLevelComboBox->setVisible(check);
  m_Controls->m_QBallReconstructionLambdaTextLabel_2->setVisible(check);
  m_Controls->m_QBallReconstructionLambdaLineEdit->setVisible(check);

  m_Controls->m_QBallReconstructionThresholdLabel_2->setVisible(check);
  m_Controls->m_QBallReconstructionThreasholdEdit->setVisible(check);
  m_Controls->label_2->setVisible(check);
  m_Controls->frame_2->setVisible(check);
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
    else
    {
#if BOOST_VERSION / 100000 > 0
#if BOOST_VERSION / 100 % 1000 > 34
      if(method == 1)
      {
        AnalyticalQBallReconstruction(set, normalization);
      }
      if(method == 2)
      {
        MultiQBallReconstruction(set);
      }
#else
      std::cout << "ERROR: Boost 1.35 minimum required" << std::endl;
      QMessageBox::warning(NULL,"ERROR","Boost 1.35 minimum required");
#endif
#else
      std::cout << "ERROR: Boost 1.35 minimum required" << std::endl;
      QMessageBox::warning(NULL,"ERROR","Boost 1.35 minimum required");
#endif
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
      MITK_INFO << "QBall reconstruction ";
      mitk::StatusBar::GetInstance()->DisplayText(status.sprintf(
                                                    "QBall reconstruction for %s", nodename.c_str()).toAscii());

      typedef itk::DiffusionQballReconstructionImageFilter
          <DiffusionPixelType, DiffusionPixelType, TTensorPixelType, QBALL_ODFSIZE>
          QballReconstructionImageFilterType;

      QballReconstructionImageFilterType::Pointer filter =
          QballReconstructionImageFilterType::New();
      filter->SetGradientImage( vols->GetDirections(), vols->GetVectorImage() );
      filter->SetBValue(vols->GetB_Value());
      filter->SetThreshold( m_Controls->m_QBallReconstructionThreasholdEdit->value() );

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
      MITK_DEBUG << "took " << clock.GetMeanTime() << "s." ;

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
    MITK_INFO << ex ;
    QMessageBox::information(0, "Reconstruction not possible:", ex.GetDescription());
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
    float minLambda  = m_Controls->m_QBallReconstructionLambdaLineEdit->value();
    lambdas.push_back(minLambda);
    int nLambdas = lambdas.size();


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
      MITK_INFO << "QBall reconstruction ";
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
        case 4:
        {
          TemplatedAnalyticalQBallReconstruction<10>(vols, currentLambda, nodename, nodes, normalization);
          break;
        }
        case 5:
        {
          TemplatedAnalyticalQBallReconstruction<12>(vols, currentLambda, nodename, nodes, normalization);
          break;
        }
        }

        clock.Stop();
        MITK_DEBUG << "took " << clock.GetMeanTime() << "s." ;
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
    MITK_INFO << ex;
    QMessageBox::information(0, "Reconstruction not possible:", ex.GetDescription());
    return;
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
  filter->SetBValue(vols->GetB_Value());
  filter->SetThreshold( m_Controls->m_QBallReconstructionThreasholdEdit->value() );
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

  if(m_Controls->m_OutputCoeffsImage->isChecked())
  {
    mitk::Image::Pointer coeffsImage = mitk::Image::New();
    coeffsImage->InitializeByItk( filter->GetCoefficientImage().GetPointer() );
    coeffsImage->SetVolume( filter->GetCoefficientImage()->GetBufferPointer() );
    mitk::DataNode::Pointer coeffsNode=mitk::DataNode::New();
    coeffsNode->SetData( coeffsImage );
    coeffsNode->SetProperty( "name", mitk::StringProperty::New(
                               QString(nodename.c_str()).append("_coeffs").toStdString()) );
    nodes->push_back(coeffsNode);
  }

}

void QmitkQBallReconstructionView::MultiQBallReconstruction(
    mitk::DataStorage::SetOfObjects::Pointer inImages)
{
  try
  {
    itk::TimeProbe clock;

    int nrFiles = inImages->size();
    if (!nrFiles) return;

    std::vector<float> lambdas;
    float minLambda  = m_Controls->m_QBallReconstructionLambdaLineEdit->value();
    lambdas.push_back(minLambda);
    int nLambdas = lambdas.size();


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


      const mitk::DataNode * nodePointer = (*itemiter).GetPointer();
      std::string nodename;
      (*itemiter)->GetStringProperty("name",nodename);

      itemiter++;

      // QBALL RECONSTRUCTION
      clock.Start();
      MITK_INFO << "QBall reconstruction ";
      mitk::StatusBar::GetInstance()->DisplayText(status.sprintf(
                                                    "QBall reconstruction for %s", nodename.c_str()).toAscii());

      for(int i=0; i<nLambdas; i++)
      {

        float currentLambda = lambdas[i];

        switch(m_Controls->m_QBallReconstructionMaxLLevelComboBox->currentIndex())
        {
        case 0:
        {
          TemplatedMultiQBallReconstruction<2>(vols, currentLambda, nodePointer, nodes);
          break;
        }
        case 1:
        {
          TemplatedMultiQBallReconstruction<4>(vols, currentLambda, nodePointer, nodes);
          break;
        }
        case 2:
        {
          TemplatedMultiQBallReconstruction<6>(vols, currentLambda, nodePointer, nodes);
          break;
        }
        case 3:
        {
          TemplatedMultiQBallReconstruction<8>(vols, currentLambda, nodePointer, nodes);
          break;
        }
        case 4:
        {
          TemplatedMultiQBallReconstruction<10>(vols, currentLambda, nodePointer, nodes);
          break;
        }
        case 5:
        {
          TemplatedMultiQBallReconstruction<12>(vols, currentLambda, nodePointer, nodes);
          break;
        }
        }

        clock.Stop();
        MITK_DEBUG << "took " << clock.GetMeanTime() << "s." ;
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
    MITK_INFO << ex ;
    QMessageBox::information(0, "Reconstruction not possible:", ex.GetDescription());
    return ;
  }
}

template<int L>
void QmitkQBallReconstructionView::TemplatedMultiQBallReconstruction(
    mitk::DiffusionImage<DiffusionPixelType>* vols, float lambda,
    const mitk::DataNode * dataNodePointer, std::vector<mitk::DataNode::Pointer>* nodes)
{
  typedef itk::DiffusionMultiShellQballReconstructionImageFilter
      <DiffusionPixelType,DiffusionPixelType,TTensorPixelType,L,QBALL_ODFSIZE> FilterType;
  typename FilterType::Pointer filter = FilterType::New();

  std::string nodename;
  dataNodePointer->GetStringProperty("name",nodename);

  filter->SetBValueMap(m_ShellSelectorMap[dataNodePointer]->GetBValueSelctionMap());
  filter->SetGradientImage( vols->GetDirections(), vols->GetVectorImage(), vols->GetB_Value() );

  //filter->SetBValue(vols->GetB_Value());
  filter->SetThreshold( m_Controls->m_QBallReconstructionThreasholdEdit->value() );
  filter->SetLambda(lambda);


  filter->Update();

  // ODFs TO DATATREE
  mitk::QBallImage::Pointer image = mitk::QBallImage::New();
  image->InitializeByItk( filter->GetOutput() );
  image->SetVolume( filter->GetOutput()->GetBufferPointer() );
  mitk::DataNode::Pointer node=mitk::DataNode::New();
  node->SetData( image );
  QString newname;
  newname = newname.append(nodename.c_str());
  newname = newname.append("_QAMultiShell");
  SetDefaultNodeProperties(node, newname.toStdString());
  nodes->push_back(node);

  if(m_Controls->m_OutputCoeffsImage->isChecked())
  {
    mitk::Image::Pointer coeffsImage = mitk::Image::New();
    coeffsImage->InitializeByItk( filter->GetCoefficientImage().GetPointer() );
    coeffsImage->SetVolume( filter->GetCoefficientImage()->GetBufferPointer() );
    mitk::DataNode::Pointer coeffsNode=mitk::DataNode::New();
    coeffsNode->SetData( coeffsImage );
    coeffsNode->SetProperty( "name", mitk::StringProperty::New(
                               QString(nodename.c_str()).append("_coeffs").toStdString()) );
    nodes->push_back(coeffsNode);
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
//node->GetPropertyList()->SetPropertx( "levelwindow", levWinProp );

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


void QmitkQBallReconstructionView::GenerateShellSelectionUI(mitk::DataStorage::SetOfObjects::Pointer set)
{

  std::map<const mitk::DataNode * , QbrShellSelection * > tempMap;

  const mitk::DataStorage::SetOfObjects::iterator setEnd( set->end() );
  mitk::DataStorage::SetOfObjects::iterator NodeIt( set->begin() );
  while(NodeIt != setEnd)
  {


    if(m_ShellSelectorMap.find(  (*NodeIt).GetPointer() ) != m_ShellSelectorMap.end())
    {
      tempMap[(*NodeIt).GetPointer()] = m_ShellSelectorMap[(*NodeIt).GetPointer()];
      m_ShellSelectorMap.erase((*NodeIt).GetPointer());
    }else
    {
      tempMap[(*NodeIt).GetPointer()] = new QbrShellSelection(this, (*NodeIt) );
      tempMap[(*NodeIt).GetPointer()]->SetVisible(true);
    }

    NodeIt++;
  }

  for(std::map<const mitk::DataNode * , QbrShellSelection * >::iterator it = m_ShellSelectorMap.begin(); it != m_ShellSelectorMap.end();it ++)
  {
    delete it->second;
  }
  m_ShellSelectorMap.clear();
  m_ShellSelectorMap = tempMap;
}
