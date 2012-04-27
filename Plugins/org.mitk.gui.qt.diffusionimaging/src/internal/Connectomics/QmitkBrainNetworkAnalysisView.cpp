/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$ 
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

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
    QObject::connect( (QObject*)( m_Controls->syntheticNetworkComboBox ), SIGNAL(currentIndexChanged (int)),	this, SLOT(OnSyntheticNetworkComboBoxCurrentIndexChanged(int)) );
    QObject::connect( (QObject*)( m_Controls->modularizePushButton ), SIGNAL(clicked()),	this, SLOT(OnModularizePushButtonClicked()) );
  }

  // GUI is different for developer and demo mode
  m_demomode = false;
  if( m_demomode )
  {
    this->m_Controls->convertToRGBAImagePushButton->hide();
    this->m_Controls->networkifyPushButton->show();
    this->m_Controls->networkifyPushButton->setText( "Create Network" );
    this->m_Controls->modularizePushButton->show();

    this->m_Controls->syntheticNetworkOptionsGroupBox->hide();
  }
  else
  {
    this->m_Controls->convertToRGBAImagePushButton->show();
    this->m_Controls->networkifyPushButton->show();
    this->m_Controls->networkifyPushButton->setText( "Networkify" );
    this->m_Controls->modularizePushButton->show();

    this->m_Controls->syntheticNetworkOptionsGroupBox->show();
    //--------------------------- fill comboBox---------------------------
    this->m_Controls->syntheticNetworkComboBox->insertItem(0,"Regular lattice");
    this->m_Controls->syntheticNetworkComboBox->insertItem(1,"Heterogenic sphere");
    this->m_Controls->syntheticNetworkComboBox->insertItem(2,"Random network");
    this->m_Controls->syntheticNetworkComboBox->insertItem(3,"Scale free network");
    this->m_Controls->syntheticNetworkComboBox->insertItem(4,"Small world network");
  }
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
}

void QmitkBrainNetworkAnalysisView::OnSelectionChanged( std::vector<mitk::DataNode*> nodes )
{
  this->WipeDisplay();

  // iterate all selected objects, adjust warning visibility
  for( std::vector<mitk::DataNode*>::iterator it = nodes.begin();
       it != nodes.end();
       ++it )
  {
    mitk::DataNode::Pointer node = *it;
  
    if( node.IsNotNull() && dynamic_cast<mitk::Image*>(node->GetData()) )
    {
      m_Controls->lblWarning->setVisible( false );
      return;
    }

    {
      mitk::ConnectomicsNetwork* network = dynamic_cast<mitk::ConnectomicsNetwork*>( node->GetData() );
      if( node.IsNotNull() && network )
      {
        m_Controls->lblWarning->setVisible( false );

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

        return;
      }
    }
  }

  m_Controls->lblWarning->setVisible( true );
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
  mitk::ConnectomicsSyntheticNetworkGenerator::Pointer generator = mitk::ConnectomicsSyntheticNetworkGenerator::New();

  mitk::DataNode::Pointer networkNode = mitk::DataNode::New();
  int parameterOne = this->m_Controls->parameterOneSpinBox->value();
  double parameterTwo = this->m_Controls->parameterTwoDoubleSpinBox->value();
  ////add network to datastorage
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
  mitk::DataNode* node = nodes.front();
  mitk::DataNode* fiberNode = nodes.at(1);

  if (!node)
  {
    // Nothing selected. Inform the user and return
    QMessageBox::information( NULL, mitk::ConnectomicsConstantsManager::CONNECTOMICS_GUI_CONNECTOMICS_CREATION, mitk::ConnectomicsConstantsManager::CONNECTOMICS_GUI_SELECTION_WARNING);
    return;
  }

  // here we have a valid mitk::DataNode

  // a node itself is not very useful, we need its data item (the image)
  mitk::BaseData* data = node->GetData();
  mitk::BaseData* fiberData = fiberNode->GetData();
  if (data && fiberData)
  {
    // test if this data item is an image or not (could also be a surface or something totally different)
    mitk::Image* image = dynamic_cast<mitk::Image*>( data );
    mitk::FiberBundleX* fiberBundle = dynamic_cast<mitk::FiberBundleX*>( fiberData );
    if (image && fiberBundle)
    {
      m_ConnectomicsNetworkCreator->SetSegmentation( image ); 
      m_ConnectomicsNetworkCreator->SetFiberBundle( fiberBundle );
      m_ConnectomicsNetworkCreator->CreateNetworkFromFibersAndSegmentation();
      mitk::DataNode::Pointer networkNode = mitk::DataNode::New();

      ////add network to datastorage
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
