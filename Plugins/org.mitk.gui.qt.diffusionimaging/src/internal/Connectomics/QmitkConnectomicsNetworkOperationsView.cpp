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
#include "QmitkConnectomicsNetworkOperationsView.h"
#include "QmitkStdMultiWidget.h"

// ####### Qt includes #######
#include <QMessageBox>

// ####### ITK includes #######
#include <itkRGBAPixel.h>

// ####### MITK includes #######

#include <mitkConnectomicsConstantsManager.h>
#include "mitkConnectomicsSimulatedAnnealingManager.h"
#include "mitkConnectomicsSimulatedAnnealingPermutationModularity.h"
#include "mitkConnectomicsSimulatedAnnealingCostFunctionModularity.h"
#include <itkConnectomicsNetworkToConnectivityMatrixImageFilter.h>

// Includes for image casting between ITK and MITK
#include "mitkImageCast.h"
#include "mitkITKImageImport.h"
#include "mitkImageAccessByItk.h"

const std::string QmitkConnectomicsNetworkOperationsView::VIEW_ID = "org.mitk.views.connectomicsnetworkoperations";

QmitkConnectomicsNetworkOperationsView::QmitkConnectomicsNetworkOperationsView()
: QmitkFunctionality()
, m_Controls( 0 )
, m_MultiWidget( NULL )
, m_demomode( false )
, m_currentIndex( 0 )
{
}

QmitkConnectomicsNetworkOperationsView::~QmitkConnectomicsNetworkOperationsView()
{
}


void QmitkConnectomicsNetworkOperationsView::CreateQtPartControl( QWidget *parent )
{
  // build up qt view, unless already done
  if ( !m_Controls )
  {
    // create GUI widgets from the Qt Designer's .ui file
    m_Controls = new Ui::QmitkConnectomicsNetworkOperationsViewControls;
    m_Controls->setupUi( parent );

    QObject::connect( m_Controls->convertToRGBAImagePushButton, SIGNAL(clicked()), this, SLOT(OnConvertToRGBAImagePushButtonClicked()) );
    QObject::connect( (QObject*)( m_Controls->modularizePushButton ), SIGNAL(clicked()),  this, SLOT(OnModularizePushButtonClicked()) );
    QObject::connect( (QObject*)( m_Controls->prunePushButton ), SIGNAL(clicked()),  this, SLOT(OnPrunePushButtonClicked()) );
    QObject::connect( (QObject*)( m_Controls->createConnectivityMatrixImagePushButton ), SIGNAL(clicked()),  this, SLOT(OnCreateConnectivityMatrixImagePushButtonClicked()) );
  }

  // GUI is different for developer and demo mode
  m_demomode = true;
  if( m_demomode )
  {
    this->m_Controls->convertToRGBAImagePushButton->show();
    this->m_Controls->modularizePushButton->hide();
    this->m_Controls->pruneOptionsGroupBox->hide();
  }
  else
  {
    this->m_Controls->convertToRGBAImagePushButton->show();
    this->m_Controls->modularizePushButton->show();
    this->m_Controls->pruneOptionsGroupBox->show();
  }

  this->WipeDisplay();
}


void QmitkConnectomicsNetworkOperationsView::StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget)
{
  m_MultiWidget = &stdMultiWidget;
}


void QmitkConnectomicsNetworkOperationsView::StdMultiWidgetNotAvailable()
{
  m_MultiWidget = NULL;
}

void QmitkConnectomicsNetworkOperationsView::WipeDisplay()
{
  m_Controls->lblWarning->setVisible( true );
  m_Controls->inputImageOneNameLabel->setText( mitk::ConnectomicsConstantsManager::CONNECTOMICS_GUI_DASH );
  m_Controls->inputImageOneNameLabel->setVisible( false );
  m_Controls->inputImageOneLabel->setVisible( false );
}

void QmitkConnectomicsNetworkOperationsView::OnSelectionChanged( std::vector<mitk::DataNode*> nodes )
{
  this->WipeDisplay();

  // Valid options are either
  if( nodes.size() > 2 )
  {
    return;
  }

  bool currentFormatUnknown(true), alreadyImageSelected(false);

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

    if( node.IsNotNull() )
    { // network section
      mitk::ConnectomicsNetwork* network = dynamic_cast<mitk::ConnectomicsNetwork*>( node->GetData() );
      if( network )
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

      }
    } // end network section

    if ( currentFormatUnknown )
    {
      this->WipeDisplay();
      return;
    }
  } // end for loop
}

void QmitkConnectomicsNetworkOperationsView::OnConvertToRGBAImagePushButtonClicked()
{
  std::vector<mitk::DataNode*> nodes = this->GetDataManagerSelection();
  if (nodes.empty()) return;

  mitk::DataNode* node = nodes.front();

  if (!node)
  {
    // Nothing selected. Inform the user and return
    QMessageBox::information( NULL, mitk::ConnectomicsConstantsManager::CONNECTOMICS_GUI_CONNECTOMICS, mitk::ConnectomicsConstantsManager::CONNECTOMICS_GUI_ONLY_PARCELLATION_SELECTION_WARNING);
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
    else
    {
      QMessageBox::information( NULL, mitk::ConnectomicsConstantsManager::CONNECTOMICS_GUI_CONNECTOMICS, mitk::ConnectomicsConstantsManager::CONNECTOMICS_GUI_ONLY_PARCELLATION_SELECTION_WARNING);
      return;
    }
  }
  else
  {
    QMessageBox::information( NULL, mitk::ConnectomicsConstantsManager::CONNECTOMICS_GUI_CONNECTOMICS, mitk::ConnectomicsConstantsManager::CONNECTOMICS_GUI_ONLY_PARCELLATION_SELECTION_WARNING);
    return;
  }
}

template < typename TPixel, unsigned int VImageDimension >
void QmitkConnectomicsNetworkOperationsView::TurnIntoRGBA( itk::Image<TPixel, VImageDimension>* inputImage)
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
    it_inputImage.Value() = it_inputImage.Value() - subtractionStorage[int ( it_inputImage.Value() ) - offset ];
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
    it_rgbaImage.Value() = lookupTable[ int ( it_inputImage.Value() ) - offset ];
  }

  mitk::Image::Pointer mitkRGBAImage = mitk::ImportItkImage( rgbaImage );

  mitk::DataNode::Pointer rgbaImageNode = mitk::DataNode::New();
  rgbaImageNode->SetData(mitkRGBAImage);
  rgbaImageNode->SetProperty(mitk::ConnectomicsConstantsManager::CONNECTOMICS_PROPERTY_NAME, mitk::StringProperty::New(mitk::ConnectomicsConstantsManager::CONNECTOMICS_PROPERTY_DEFAULT_RGBA_NAME));
  rgbaImageNode->SetBoolProperty( mitk::ConnectomicsConstantsManager::CONNECTOMICS_PROPERTY_VOLUMERENDERING, true);
  this->GetDefaultDataStorage()->Add( rgbaImageNode );
}

void QmitkConnectomicsNetworkOperationsView::OnModularizePushButtonClicked()
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

void QmitkConnectomicsNetworkOperationsView::OnPrunePushButtonClicked()
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


void QmitkConnectomicsNetworkOperationsView::OnCreateConnectivityMatrixImagePushButtonClicked()
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
        filter->SetBinaryConnectivity(m_Controls->binaryCheckBox->isChecked());
        filter->SetRescaleConnectivity(m_Controls->rescaleCheckBox->isChecked());
        filter->Update();

        mitk::Image::Pointer connectivityMatrixImage = mitk::ImportItkImage( filter->GetOutput());
        mitk::DataNode::Pointer connectivityMatrixImageNode = mitk::DataNode::New();
        connectivityMatrixImageNode->SetData ( connectivityMatrixImage );
        connectivityMatrixImageNode->SetName( "Connectivity matrix" );
        this->GetDefaultDataStorage()->Add(connectivityMatrixImageNode, node );
      }
    }
  }
}