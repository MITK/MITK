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

// ####### Blueberry includes #######
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>

// ####### Qmitk includes #######
#include "QmitkBrainNetworkAnalysisView.h"
#include "QmitkStdMultiWidget.h"

// ####### Qt includes #######
#include <QMessageBox>

// ####### ITK includes #######
#include <itkRGBAPixel.h>

// ####### MITK includes #######

#include <mitkConnectomicsConstantsManager.h>
#include "mitkConnectomicsSyntheticNetworkGenerator.h"
#include "mitkConnectomicsSimulatedAnnealingManager.h"
#include "mitkConnectomicsSimulatedAnnealingPermutationModularity.h"
#include "mitkConnectomicsSimulatedAnnealingCostFunctionModularity.h"
#include <itkConnectomicsNetworkToConnectivityMatrixImageFilter.h>

// Includes for image casting between ITK and MITK
#include "mitkImageCast.h"
#include "mitkITKImageImport.h"
#include "mitkImageAccessByItk.h"

const std::string QmitkBrainNetworkAnalysisView::VIEW_ID = "org.mitk.views.brainnetworkanalysis";

QmitkBrainNetworkAnalysisView::QmitkBrainNetworkAnalysisView()
: QmitkFunctionality()
, m_Controls( 0 )
, m_MultiWidget( NULL )  
, m_ConnectomicsNetworkCreator( mitk::ConnectomicsNetworkCreator::New() ) 
, m_demomode( false )
, m_currentIndex( 0 )
{
}

QmitkBrainNetworkAnalysisView::~QmitkBrainNetworkAnalysisView()
{
}


void QmitkBrainNetworkAnalysisView::CreateQtPartControl( QWidget *parent )
{
  // build up qt view, unless already done
  if ( !m_Controls )
  {
    // create GUI widgets from the Qt Designer's .ui file
    m_Controls = new Ui::QmitkBrainNetworkAnalysisViewControls;
    m_Controls->setupUi( parent );

    QObject::connect( m_Controls->convertToRGBAImagePushButton, SIGNAL(clicked()), this, SLOT(OnConvertToRGBAImagePushButtonClicked()) );
    QObject::connect( m_Controls->networkifyPushButton, SIGNAL(clicked()), this, SLOT(OnNetworkifyPushButtonClicked()) );
    QObject::connect( m_Controls->syntheticNetworkCreationPushButton, SIGNAL(clicked()), this, SLOT(OnSyntheticNetworkCreationPushButtonClicked()) );
    QObject::connect( (QObject*)( m_Controls->syntheticNetworkComboBox ), SIGNAL(currentIndexChanged (int)),  this, SLOT(OnSyntheticNetworkComboBoxCurrentIndexChanged(int)) );
    QObject::connect( (QObject*)( m_Controls->modularizePushButton ), SIGNAL(clicked()),  this, SLOT(OnModularizePushButtonClicked()) );
    QObject::connect( (QObject*)( m_Controls->prunePushButton ), SIGNAL(clicked()),  this, SLOT(OnPrunePushButtonClicked()) );
    QObject::connect( (QObject*)( m_Controls->createConnectivityMatrixImagePushButton ), SIGNAL(clicked()),  this, SLOT(OnCreateConnectivityMatrixImagePushButtonClicked()) );
  }

  // GUI is different for developer and demo mode
  m_demomode = false;
  if( m_demomode )
  {
    this->m_Controls->convertToRGBAImagePushButton->hide();
    this->m_Controls->networkifyPushButton->show();
    this->m_Controls->networkifyPushButton->setText( "Create Network" );
    this->m_Controls->modularizePushButton->hide();
    this->m_Controls->pruneOptionsGroupBox->hide();

    this->m_Controls->syntheticNetworkOptionsGroupBox->show();
    //--------------------------- fill comboBox---------------------------
    this->m_Controls->syntheticNetworkComboBox->insertItem(0,"Regular lattice");
    this->m_Controls->syntheticNetworkComboBox->insertItem(1,"Heterogenic sphere");
    this->m_Controls->syntheticNetworkComboBox->insertItem(2,"Random network");
  }
  else
  {
    this->m_Controls->convertToRGBAImagePushButton->show();
    this->m_Controls->networkifyPushButton->show();
    this->m_Controls->networkifyPushButton->setText( "Networkify" );
    this->m_Controls->modularizePushButton->show();
    this->m_Controls->pruneOptionsGroupBox->show();

    this->m_Controls->syntheticNetworkOptionsGroupBox->show();
    //--------------------------- fill comboBox---------------------------
    this->m_Controls->syntheticNetworkComboBox->insertItem(0,"Regular lattice");
    this->m_Controls->syntheticNetworkComboBox->insertItem(1,"Heterogenic sphere");
    this->m_Controls->syntheticNetworkComboBox->insertItem(2,"Random network");
    this->m_Controls->syntheticNetworkComboBox->insertItem(3,"Scale free network");
    this->m_Controls->syntheticNetworkComboBox->insertItem(4,"Small world network");
  }

  this->WipeDisplay();
}


void QmitkBrainNetworkAnalysisView::StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget)
{
  m_MultiWidget = &stdMultiWidget;
}


void QmitkBrainNetworkAnalysisView::StdMultiWidgetNotAvailable()
{
  m_MultiWidget = NULL;
}

void QmitkBrainNetworkAnalysisView::WipeDisplay()
{
  m_Controls->lblWarning->setVisible( true );
  m_Controls->inputImageOneNameLabel->setText( mitk::ConnectomicsConstantsManager::CONNECTOMICS_GUI_DASH );
  m_Controls->inputImageOneNameLabel->setVisible( false );
  m_Controls->inputImageOneLabel->setVisible( false );
  m_Controls->inputImageTwoNameLabel->setText( mitk::ConnectomicsConstantsManager::CONNECTOMICS_GUI_DASH );
  m_Controls->inputImageTwoNameLabel->setVisible( false );
  m_Controls->inputImageTwoLabel->setVisible( false );
  m_Controls->numberOfVerticesLabel->setText( mitk::ConnectomicsConstantsManager::CONNECTOMICS_GUI_DASH );
  m_Controls->numberOfEdgesLabel->setText( mitk::ConnectomicsConstantsManager::CONNECTOMICS_GUI_DASH );
  m_Controls->numberOfSelfLoopsLabel->setText( mitk::ConnectomicsConstantsManager::CONNECTOMICS_GUI_DASH );
  m_Controls->averageDegreeLabel->setText( mitk::ConnectomicsConstantsManager::CONNECTOMICS_GUI_DASH );
  m_Controls->connectionDensityLabel->setText( mitk::ConnectomicsConstantsManager::CONNECTOMICS_GUI_DASH );
  m_Controls->efficiencyLabel->setText( mitk::ConnectomicsConstantsManager::CONNECTOMICS_GUI_DASH );
  m_Controls->globalClusteringLabel->setText( mitk::ConnectomicsConstantsManager::CONNECTOMICS_GUI_DASH );
  m_Controls->betweennessNetworkHistogramCanvas->SetHistogram(   NULL );
  m_Controls->degreeNetworkHistogramCanvas->SetHistogram(       NULL );
  m_Controls->shortestPathNetworkHistogramCanvas->SetHistogram( NULL );
  m_Controls->betweennessNetworkHistogramCanvas->update();
  m_Controls->degreeNetworkHistogramCanvas->update();
  m_Controls->shortestPathNetworkHistogramCanvas->update();
  m_Controls->betweennessNetworkHistogramCanvas->Clear();
  m_Controls->degreeNetworkHistogramCanvas->Clear();
  m_Controls->shortestPathNetworkHistogramCanvas->Clear();
  m_Controls->betweennessNetworkHistogramCanvas->Replot();
  m_Controls->degreeNetworkHistogramCanvas->Replot();
  m_Controls->shortestPathNetworkHistogramCanvas->Replot();
}

void QmitkBrainNetworkAnalysisView::OnSelectionChanged( std::vector<mitk::DataNode*> nodes )
{
  this->WipeDisplay();

  // Valid options are either
  // 1 image (parcellation)
  //
  // 1 image (parcellation)
  // 1 fiber bundle
  //
  // 1 network
  if( nodes.size() > 2 )
  {
    return;
  }

  bool alreadyFiberBundleSelected( false ), alreadyImageSelected( false ), currentFormatUnknown( true );
  // iterate all selected objects, adjust warning visibility
  for( std::vector<mitk::DataNode*>::iterator it = nodes.begin();
       it != nodes.end();
       ++it )
  {
    mitk::DataNode::Pointer node = *it;
    currentFormatUnknown = true;
  
    if( node.IsNotNull() && dynamic_cast<mitk::Image*>(node->GetData()) )
    {
      currentFormatUnknown = false;
      if( alreadyImageSelected )
      {
        this->WipeDisplay();
        return;
      }
      alreadyImageSelected = true;
      m_Controls->lblWarning->setVisible( false );
      m_Controls->inputImageOneNameLabel->setText(node->GetName().c_str());
      m_Controls->inputImageOneNameLabel->setVisible( true );
      m_Controls->inputImageOneLabel->setVisible( true );
    }

    if( node.IsNotNull() && dynamic_cast<mitk::FiberBundleX*>(node->GetData()) )
    {
      currentFormatUnknown = false;
      // a fiber bundle has to be in conjunction with a parcellation
      if( nodes.size() != 2 || alreadyFiberBundleSelected )
      {
        this->WipeDisplay();
        return;
      }
      alreadyFiberBundleSelected = true;
      m_Controls->lblWarning->setVisible( false );
      m_Controls->inputImageTwoNameLabel->setText(node->GetName().c_str());
      m_Controls->inputImageTwoNameLabel->setVisible( true );
      m_Controls->inputImageTwoLabel->setVisible( true );
    }

    { // network section
      mitk::ConnectomicsNetwork* network = dynamic_cast<mitk::ConnectomicsNetwork*>( node->GetData() );
      if( node.IsNotNull() && network )
      {
        currentFormatUnknown = false;
        if( nodes.size() != 1 )
        {
          // only valid option is a single network
          this->WipeDisplay();
          return;
        }
        m_Controls->lblWarning->setVisible( false );
        m_Controls->inputImageOneNameLabel->setText(node->GetName().c_str());
        m_Controls->inputImageOneNameLabel->setVisible( true );
        m_Controls->inputImageOneLabel->setVisible( true );

        int noVertices = network->GetNumberOfVertices();
        int noEdges = network->GetNumberOfEdges();
        int noSelfLoops = network->GetNumberOfSelfLoops();
        double averageDegree = network->GetAverageDegree();
        double connectionDensity = network->GetConnectionDensity();
        double globalClustering = network->GetGlobalClusteringCoefficient();

        m_Controls->numberOfVerticesLabel->setText( QString::number( noVertices ) );
        m_Controls->numberOfEdgesLabel->setText( QString::number( noEdges ) );
        m_Controls->numberOfSelfLoopsLabel->setText( QString::number( noSelfLoops ) );
        m_Controls->averageDegreeLabel->setText( QString::number( averageDegree ) );
        m_Controls->connectionDensityLabel->setText( QString::number( connectionDensity ) );
        m_Controls->globalClusteringLabel->setText( QString::number( globalClustering ) );

        mitk::ConnectomicsNetwork::Pointer connectomicsNetwork( network );
        mitk::ConnectomicsHistogramsContainer *histogramContainer = histogramCache[ connectomicsNetwork ];
        if(histogramContainer)
        {
          m_Controls->betweennessNetworkHistogramCanvas->SetHistogram(  histogramContainer->GetBetweennessHistogram() );
          m_Controls->degreeNetworkHistogramCanvas->SetHistogram(       histogramContainer->GetDegreeHistogram() );
          m_Controls->shortestPathNetworkHistogramCanvas->SetHistogram( histogramContainer->GetShortestPathHistogram() );
          m_Controls->betweennessNetworkHistogramCanvas->DrawProfiles();
          m_Controls->degreeNetworkHistogramCanvas->DrawProfiles();
          m_Controls->shortestPathNetworkHistogramCanvas->DrawProfiles();

          double efficiency = histogramContainer->GetShortestPathHistogram()->GetEfficiency();

          m_Controls->efficiencyLabel->setText( QString::number( efficiency ) );
        }
      }
    } // end network section

    if ( currentFormatUnknown )
    {
      this->WipeDisplay();
      return;
    }
  } // end for loop
}

void QmitkBrainNetworkAnalysisView::OnSyntheticNetworkComboBoxCurrentIndexChanged(int currentIndex)
{
  m_currentIndex = currentIndex;

    switch (m_currentIndex) {
  case 0:
    this->m_Controls->parameterOneLabel->setText( "Nodes per side" );
    this->m_Controls->parameterTwoLabel->setText( "Internode distance" );
    this->m_Controls->parameterOneSpinBox->setEnabled( true );
    this->m_Controls->parameterOneSpinBox->setValue( 5 );
    this->m_Controls->parameterTwoDoubleSpinBox->setEnabled( true );
    this->m_Controls->parameterTwoDoubleSpinBox->setMaximum( 999.999 );
    this->m_Controls->parameterTwoDoubleSpinBox->setValue( 10.0 );
    break;
  case 1:
    this->m_Controls->parameterOneLabel->setText( "Number of nodes" );
    this->m_Controls->parameterTwoLabel->setText( "Radius" );
    this->m_Controls->parameterOneSpinBox->setEnabled( true );
    this->m_Controls->parameterOneSpinBox->setValue( 1000 );
    this->m_Controls->parameterTwoDoubleSpinBox->setEnabled( true );
    this->m_Controls->parameterTwoDoubleSpinBox->setMaximum( 999.999 );
    this->m_Controls->parameterTwoDoubleSpinBox->setValue( 50.0 );
    break;
  case 2:
    this->m_Controls->parameterOneLabel->setText( "Number of nodes" );
    this->m_Controls->parameterTwoLabel->setText( "Edge percentage" );
    this->m_Controls->parameterOneSpinBox->setEnabled( true );
    this->m_Controls->parameterOneSpinBox->setValue( 100 );
    this->m_Controls->parameterTwoDoubleSpinBox->setEnabled( true );
    this->m_Controls->parameterTwoDoubleSpinBox->setMaximum( 1.0 );
    this->m_Controls->parameterTwoDoubleSpinBox->setValue( 0.5 );
    break;
  case 3:
    //GenerateSyntheticScaleFreeNetwork( network, 1000 );
    break;
  case 4:
    //GenerateSyntheticSmallWorldNetwork( network, 1000 );
    break;
  default:
    this->m_Controls->parameterOneLabel->setText( "Parameter 1" );
    this->m_Controls->parameterTwoLabel->setText( "Paramater 2" );
    this->m_Controls->parameterOneSpinBox->setEnabled( false );
    this->m_Controls->parameterOneSpinBox->setValue( 0 );
    this->m_Controls->parameterTwoDoubleSpinBox->setEnabled( false );
    this->m_Controls->parameterTwoDoubleSpinBox->setValue( 0.0 );
    }
}

void QmitkBrainNetworkAnalysisView::OnSyntheticNetworkCreationPushButtonClicked()
{
  // warn if trying to create a very big network
  // big network is a network with > 5000 nodes (estimate)
  // this might fill up the memory to the point it freezes
  int numberOfNodes( 0 );
  switch (m_currentIndex) {
  case 0:
    numberOfNodes = this->m_Controls->parameterOneSpinBox->value() 
      * this->m_Controls->parameterOneSpinBox->value() 
      * this->m_Controls->parameterOneSpinBox->value();
    break;
  case 1:
    numberOfNodes = this->m_Controls->parameterOneSpinBox->value();
    break;
  case 2:
    numberOfNodes = this->m_Controls->parameterOneSpinBox->value();
    break;
  case 3:
    // not implemented yet
    break;
  case 4:
    // not implemented yet
    break;
  default:
    break;

  }

  if( numberOfNodes > 5000 )
  {
    QMessageBox msgBox;
    msgBox.setText("Trying to generate very large network.");
    msgBox.setIcon( QMessageBox::Warning );
    msgBox.setInformativeText("You are trying to generate a network with more than 5000 nodes, this is very resource intensive and might lead to program instability. Proceed with network generation?");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);
    int ret = msgBox.exec();

    switch (ret) {
  case QMessageBox::Yes:
    // continue
    break;
  case QMessageBox::No:
    // stop
    return;
    break;

  default:
    // should never be reached
    break;
    }
  }

  // proceed
  mitk::ConnectomicsSyntheticNetworkGenerator::Pointer generator = mitk::ConnectomicsSyntheticNetworkGenerator::New();

  mitk::DataNode::Pointer networkNode = mitk::DataNode::New();
  int parameterOne = this->m_Controls->parameterOneSpinBox->value();
  double parameterTwo = this->m_Controls->parameterTwoDoubleSpinBox->value();
  //add network to datastorage
  networkNode->SetData( generator->CreateSyntheticNetwork( m_currentIndex, parameterOne, parameterTwo ) );
  networkNode->SetName( mitk::ConnectomicsConstantsManager::CONNECTOMICS_PROPERTY_DEFAULT_CNF_NAME );
  if( generator->WasGenerationSuccessfull() )
  {
    this->GetDefaultDataStorage()->Add( networkNode );
  }
  else
  {
    MITK_WARN << "Problem occured during synthetic network generation.";
  }

  return;  
}

void QmitkBrainNetworkAnalysisView::OnConvertToRGBAImagePushButtonClicked()
{
  std::vector<mitk::DataNode*> nodes = this->GetDataManagerSelection();
  if (nodes.empty()) return;

  mitk::DataNode* node = nodes.front();

  if (!node)
  {
    // Nothing selected. Inform the user and return
    QMessageBox::information( NULL, mitk::ConnectomicsConstantsManager::CONNECTOMICS_GUI_CONNECTOMICS_CREATION, mitk::ConnectomicsConstantsManager::CONNECTOMICS_GUI_SELECTION_WARNING);
    return;
  }

  // here we have a valid mitk::DataNode

  // a node itself is not very useful, we need its data item (the image)
  mitk::BaseData* data = node->GetData();
  if (data)
  {
    // test if this data item is an image or not (could also be a surface or something totally different)
    mitk::Image* image = dynamic_cast<mitk::Image*>( data );
    if (image)
    {
      std::stringstream message;
      std::string name;
      message << mitk::ConnectomicsConstantsManager::CONNECTOMICS_GUI_PERFORMING_IMAGE_PROCESSING_FOR_IMAGE;
      if (node->GetName(name))
      {
        // a property called "name" was found for this DataNode
        message << "'" << name << "'";
      }
      message << ".";
      MITK_INFO << message.str();

      // Convert to RGBA
      AccessByItk( image, TurnIntoRGBA );
      this->GetDefaultDataStorage()->GetNamedNode( mitk::ConnectomicsConstantsManager::CONNECTOMICS_PROPERTY_DEFAULT_RGBA_NAME )->GetData()->SetGeometry( node->GetData()->GetGeometry() );

      mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    }
  }
}

template < typename TPixel, unsigned int VImageDimension >
void QmitkBrainNetworkAnalysisView::TurnIntoRGBA( itk::Image<TPixel, VImageDimension>* inputImage)
{
  typedef itk::RGBAPixel< unsigned char >               RGBAPixelType;
  typedef itk::Image< TPixel, VImageDimension >         TemplateImageType;
  typedef itk::Image< RGBAPixelType, VImageDimension >  RGBAImageType;

  itk::ImageRegionIterator<TemplateImageType> it_inputImage(inputImage, inputImage->GetLargestPossibleRegion());

  TPixel minimumValue, maximumValue;

  it_inputImage.GoToBegin();
  maximumValue = minimumValue = it_inputImage.Value();

  for(it_inputImage.GoToBegin(); !it_inputImage.IsAtEnd(); ++it_inputImage)
  {
    if ( it_inputImage.Value() < minimumValue )
    {
      minimumValue = it_inputImage.Value();
    }
    else
    {
      if ( it_inputImage.Value() > maximumValue )
      {
        maximumValue = it_inputImage.Value();
      }
    }
  }

  int range = int ( maximumValue - minimumValue ); //needs to be castable to int
  int offset = int ( minimumValue );

  if ( range < 0 ) //error
  {
    return;
  }

  std::vector< unsigned int > histogram;
  histogram.resize( range + 1, 0 );

  for(it_inputImage.GoToBegin(); !it_inputImage.IsAtEnd(); ++it_inputImage)
  {
    histogram[ int ( it_inputImage.Value() ) - offset ] += 1;
  }

  int gapCounter = 0; //this variable will be used to count the empty labels

  //stores how much has to be subtracted from the image to remove gaps
  std::vector< TPixel > subtractionStorage; 
  subtractionStorage.resize( range + 1, 0 );

  for( int index = 0; index <= range; index++ )
  {
    if( histogram[ index ] == 0 )
    {
      gapCounter++; //if the label is empty, increase gapCounter
    }
    else
    {
      subtractionStorage[ index ] = TPixel ( gapCounter );
    }
  }

  //remove gaps from label image
  for(it_inputImage.GoToBegin(); !it_inputImage.IsAtEnd(); ++it_inputImage)
  {
    it_inputImage.Value() = it_inputImage.Value() - subtractionStorage[ int ( it_inputImage.Value() ) ];
  }

  // create colour vector
  std::vector< RGBAPixelType > lookupTable;

  {
    RGBAPixelType backgroundColour;
    for( int elementIndex = 0; elementIndex < 4; ++elementIndex )
    {
      backgroundColour.SetElement( elementIndex, 0 );
    }

    lookupTable.push_back( backgroundColour );

    for(int colourNumber = 0; colourNumber < range ; ++colourNumber)
    {
      RGBAPixelType colour;
      for( int elementIndex = 0; elementIndex < 3; ++elementIndex )
      {
        colour.SetElement( elementIndex, rand() % 256 );
      }
      colour.SetAlpha( 255 );
      lookupTable.push_back( colour );
    }
  }

  // create RGBA image
  typename RGBAImageType::Pointer rgbaImage = RGBAImageType::New();

  rgbaImage->SetRegions(inputImage->GetLargestPossibleRegion().GetSize());
  rgbaImage->SetSpacing(inputImage->GetSpacing());
  rgbaImage->SetOrigin(inputImage->GetOrigin());
  rgbaImage->Allocate();

  //fill with appropriate colours
  itk::ImageRegionIterator<RGBAImageType> it_rgbaImage(rgbaImage, rgbaImage->GetLargestPossibleRegion());

  for(it_inputImage.GoToBegin(), it_rgbaImage.GoToBegin(); !it_inputImage.IsAtEnd(); ++it_inputImage, ++it_rgbaImage)
  {
    it_rgbaImage.Value() = lookupTable[ int ( it_inputImage.Value() ) ];
  }

  mitk::Image::Pointer mitkRGBAImage = mitk::ImportItkImage( rgbaImage ); 

  mitk::DataNode::Pointer rgbaImageNode = mitk::DataNode::New();
  rgbaImageNode->SetData(mitkRGBAImage);
  rgbaImageNode->SetProperty(mitk::ConnectomicsConstantsManager::CONNECTOMICS_PROPERTY_NAME, mitk::StringProperty::New(mitk::ConnectomicsConstantsManager::CONNECTOMICS_PROPERTY_DEFAULT_RGBA_NAME));
  rgbaImageNode->SetBoolProperty( mitk::ConnectomicsConstantsManager::CONNECTOMICS_PROPERTY_VOLUMERENDERING, true);
  this->GetDefaultDataStorage()->Add( rgbaImageNode );
}

void QmitkBrainNetworkAnalysisView::OnNetworkifyPushButtonClicked()
{
  std::vector<mitk::DataNode*> nodes = this->GetDataManagerSelection();
  if ( nodes.empty() )
  {
    QMessageBox::information( NULL, mitk::ConnectomicsConstantsManager::CONNECTOMICS_GUI_CONNECTOMICS_CREATION, mitk::ConnectomicsConstantsManager::CONNECTOMICS_GUI_SELECTION_WARNING);
    return;
  }

  if (! ( nodes.size() == 2 ) )
  {
    QMessageBox::information( NULL, mitk::ConnectomicsConstantsManager::CONNECTOMICS_GUI_CONNECTOMICS_CREATION, mitk::ConnectomicsConstantsManager::CONNECTOMICS_GUI_SELECTION_WARNING);
    return;
  }
  mitk::DataNode* firstNode = nodes.front();
  mitk::DataNode* secondNode = nodes.at(1);

  if (!firstNode)
  {
    // Nothing selected. Inform the user and return
    QMessageBox::information( NULL, mitk::ConnectomicsConstantsManager::CONNECTOMICS_GUI_CONNECTOMICS_CREATION, mitk::ConnectomicsConstantsManager::CONNECTOMICS_GUI_SELECTION_WARNING);
    return;
  }

  // here we have a valid mitk::DataNode

  // a node itself is not very useful, we need its data item (the image)
  mitk::BaseData* firstData = firstNode->GetData();
  mitk::BaseData* secondData = secondNode->GetData();
  if (firstData && secondData)
  {
    // test if this data item is an image or not (could also be a surface or something totally different)
    mitk::Image* image = dynamic_cast<mitk::Image*>( firstData );
    mitk::FiberBundleX* fiberBundle = dynamic_cast<mitk::FiberBundleX*>( secondData );

    // check whether order was switched
    if (! (image && fiberBundle) )
    {
      image = dynamic_cast<mitk::Image*>( secondData );
      fiberBundle = dynamic_cast<mitk::FiberBundleX*>( firstData );
    }

    if (image && fiberBundle)
    {
      m_ConnectomicsNetworkCreator->SetSegmentation( image ); 
      m_ConnectomicsNetworkCreator->SetFiberBundle( fiberBundle );
      m_ConnectomicsNetworkCreator->CreateNetworkFromFibersAndSegmentation();
      mitk::DataNode::Pointer networkNode = mitk::DataNode::New();

      //add network to datastorage
      networkNode->SetData( m_ConnectomicsNetworkCreator->GetNetwork() );
      networkNode->SetName( mitk::ConnectomicsConstantsManager::CONNECTOMICS_PROPERTY_DEFAULT_CNF_NAME );
      this->GetDefaultDataStorage()->Add( networkNode );
    }
  }

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkBrainNetworkAnalysisView::OnModularizePushButtonClicked()
{
  std::vector<mitk::DataNode*> nodes = this->GetDataManagerSelection();
  if ( nodes.empty() )
  {
    QMessageBox::information( NULL, "Modularization calculation", "Please select exactly one network.");
    return;
  }
  
  for( std::vector<mitk::DataNode*>::iterator it = nodes.begin();
    it != nodes.end();
    ++it )
  {
    mitk::DataNode::Pointer node = *it;

    if( node.IsNotNull() && dynamic_cast<mitk::Image*>(node->GetData()) )
    {
      return;
    }

    {
      mitk::ConnectomicsNetwork* network = dynamic_cast<mitk::ConnectomicsNetwork*>( node->GetData() );
      if( node.IsNotNull() && network )
      {

        typedef mitk::ConnectomicsSimulatedAnnealingPermutationModularity::ToModuleMapType MappingType;

        int depthOfModuleRecursive( 2 );
        double startTemperature( 2.0 );
        double stepSize( 4.0 );

        mitk::ConnectomicsNetwork::Pointer connectomicsNetwork( network );
        mitk::ConnectomicsSimulatedAnnealingManager::Pointer manager = mitk::ConnectomicsSimulatedAnnealingManager::New();
        mitk::ConnectomicsSimulatedAnnealingPermutationModularity::Pointer permutation = mitk::ConnectomicsSimulatedAnnealingPermutationModularity::New();
        mitk::ConnectomicsSimulatedAnnealingCostFunctionModularity::Pointer costFunction = mitk::ConnectomicsSimulatedAnnealingCostFunctionModularity::New();

        permutation->SetCostFunction( costFunction.GetPointer() );
        permutation->SetNetwork( connectomicsNetwork );
        permutation->SetDepth( depthOfModuleRecursive );
        permutation->SetStepSize( stepSize );

        manager->SetPermutation( permutation.GetPointer() );

        manager->RunSimulatedAnnealing( startTemperature, stepSize );

        MappingType mapping = permutation->GetMapping();

        MappingType::iterator iter = mapping.begin();
        MappingType::iterator end =  mapping.end();

        int loop( 0 );
        while( iter != end )
        {
          MBI_DEBUG << "Vertex " << iter->first << " belongs to module " << iter->second ;
          MBI_INFO << "Vertex " << iter->first << " belongs to module " << iter->second ;
          iter++;
        }

        MBI_DEBUG << "Overall number of modules is " << permutation->getNumberOfModules( &mapping ) ;
        MBI_DEBUG << "Cost is " << costFunction->Evaluate( network, &mapping ) ;
        MBI_INFO << "Overall number of modules is " << permutation->getNumberOfModules( &mapping ) ;
        MBI_INFO << "Cost is " << costFunction->Evaluate( network, &mapping ) ;

        return;
      }
    }
  }

}

void QmitkBrainNetworkAnalysisView::OnPrunePushButtonClicked()
{
  std::vector<mitk::DataNode*> nodes = this->GetDataManagerSelection();
  if ( nodes.empty() )
  {
    QMessageBox::information( NULL, "Network pruning", "Please select one or more networks.");
    return;
  }

  for( std::vector<mitk::DataNode*>::iterator it = nodes.begin();
    it != nodes.end();
    ++it )
  {
    mitk::DataNode::Pointer node = *it;

    if( node.IsNotNull() && dynamic_cast<mitk::Image*>(node->GetData()) )
    {
      return;
    }

    {
      mitk::ConnectomicsNetwork* network = dynamic_cast<mitk::ConnectomicsNetwork*>( node->GetData() );
      if( node.IsNotNull() && network )
      {
        // Edge pruning will also do node pruning
        network->PruneEdgesBelowWeight( this->m_Controls->pruneEdgeWeightSpinBox->value() );
      }
    }
  }
}


void QmitkBrainNetworkAnalysisView::OnCreateConnectivityMatrixImagePushButtonClicked()
{
  std::vector<mitk::DataNode*> nodes = this->GetDataManagerSelection();
  if ( nodes.empty() )
  {
    QMessageBox::information( NULL, "Connectivity Matrix Image creation", "Please select one or more networks.");
    return;
  }

  for( std::vector<mitk::DataNode*>::iterator it = nodes.begin();
    it != nodes.end();
    ++it )
  {
    mitk::DataNode::Pointer node = *it;

    if( node.IsNotNull() && dynamic_cast<mitk::Image*>(node->GetData()) )
    {
      return;
    }

    {
      mitk::ConnectomicsNetwork* network = dynamic_cast<mitk::ConnectomicsNetwork*>( node->GetData() );
      if( node.IsNotNull() && network )
      {
        itk::ConnectomicsNetworkToConnectivityMatrixImageFilter::Pointer filter = itk::ConnectomicsNetworkToConnectivityMatrixImageFilter::New();
        filter->SetInputNetwork( network );
        filter->Update();

        mitk::Image::Pointer connectivityMatrixImage = mitk::ImportItkImage( filter->GetOutput());
        mitk::DataNode::Pointer connectivityMatrixImageNode = mitk::DataNode::New();
        connectivityMatrixImageNode->SetData ( connectivityMatrixImage );
        this->GetDefaultDataStorage()->Add(connectivityMatrixImageNode, node );
      }
    }
  }
}