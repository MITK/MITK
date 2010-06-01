/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: 17495 $ 
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>

// MITK
#include "mitkImageAccessByItk.h"
#include "mitkITKImageImport.h"
#include "mitkProperties.h"
#include "mitkColorProperty.h"

// Qmitk
#include "QmitkRegionGrowingView.h"
#include "QmitkPointListWidget.h"
#include "QmitkStdMultiWidget.h"

// Qt
#include <QMessageBox>

// ITK
#include <itkConnectedThresholdImageFilter.h>

const std::string QmitkRegionGrowingView::VIEW_ID = "org.mitk.views.regiongrowing";

QmitkRegionGrowingView::QmitkRegionGrowingView()
: QmitkFunctionality()
, m_Controls( 0 )
, m_MultiWidget( NULL )
{
}

QmitkRegionGrowingView::~QmitkRegionGrowingView()
{
}

void QmitkRegionGrowingView::Deactivated()
{
  m_Controls->lstPoints->DeactivateInteractor(true);
}

void QmitkRegionGrowingView::CreateQtPartControl( QWidget *parent )
{
  // build up qt view, unless already done
  if ( !m_Controls )
  {
    // create GUI widgets from the Qt Designer's .ui file
    m_Controls = new Ui::QmitkRegionGrowingViewControls;
    m_Controls->setupUi( parent );
 
    connect( m_Controls->btnPerformImageProcessing, SIGNAL(clicked()), this, SLOT(DoImageProcessing()) );
   
    // let the point set widget know about the multi widget (crosshair updates)
    m_Controls->lstPoints->SetMultiWidget( m_MultiWidget );
    
    // create a new DataNode containing a PointSet with some interaction
    m_PointSet = mitk::PointSet::New();

    mitk::DataNode::Pointer pointSetNode = mitk::DataNode::New();
    pointSetNode->SetData( m_PointSet );
    pointSetNode->SetName("seed points for region growing");
    pointSetNode->SetProperty("helper object", mitk::BoolProperty::New(true) );
    pointSetNode->SetProperty("layer", mitk::IntProperty::New(1024) );

    // add the pointset to the data tree (for rendering and access by other modules)
    GetDefaultDataStorage()->Add( pointSetNode );

    // tell the GUI widget about out point set
    m_Controls->lstPoints->SetPointSetNode( pointSetNode );
  }
}


void QmitkRegionGrowingView::StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget)
{
  m_MultiWidget = &stdMultiWidget;
  m_Controls->lstPoints->SetMultiWidget( m_MultiWidget );
}


void QmitkRegionGrowingView::StdMultiWidgetNotAvailable()
{
  m_MultiWidget = NULL;
  m_Controls->lstPoints->SetMultiWidget( NULL );
}


void QmitkRegionGrowingView::OnSelectionChanged( std::vector<mitk::DataNode*> nodes )
{ 
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
  }

  m_Controls->lblWarning->setVisible( true );
}


void QmitkRegionGrowingView::DoImageProcessing()
{
  std::vector<mitk::DataNode*> nodes = this->GetDataManagerSelection();
  if (nodes.empty()) return;

  mitk::DataNode* node = nodes.front();

  if (!node)
  {
    // Nothing selected. Inform the user and return
    QMessageBox::information( NULL, "Region growing functionality", "Please load and select an image before region growing.");
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
      message << "Performing image processing for image ";
      if (node->GetName(name))
      {
        // a property called "name" was found for this DataNode
        message << "'" << name << "'";
      }
      message << ".";
      MITK_INFO << message.str();

      // So we have an image. Let's see if the user has set some seed points already
      if ( m_PointSet->GetSize() == 0 )
      {
        // no points there. Not good for region growing
        QMessageBox::information( NULL, "Region growing functionality", 
                                        "Please set some seed points inside the image first.\n"
                                        "(hold Shift key and click left mouse button inside the image.)"
                                );
        return;
      }

      // actually perform region growing. Here we have both an image and some seed points
      AccessByItk_2( image, ItkImageProcessing, image->GetGeometry(), node ); // some magic to call the correctly templated function

    }
  }
}


template < typename TPixel, unsigned int VImageDimension >
void QmitkRegionGrowingView::ItkImageProcessing( itk::Image< TPixel, VImageDimension >* itkImage, mitk::Geometry3D* imageGeometry, mitk::DataNode* parent )
{
  typedef itk::Image< TPixel, VImageDimension > InputImageType;
  typedef typename InputImageType::IndexType    IndexType;
  
  // instantiate an ITK region growing filter, set its parameters
  typedef itk::ConnectedThresholdImageFilter<InputImageType, InputImageType> RegionGrowingFilterType;
  typename RegionGrowingFilterType::Pointer regionGrower = RegionGrowingFilterType::New();
  regionGrower->SetInput( itkImage ); // don't forget this

  // determine a thresholding interval
  IndexType seedIndex;
  TPixel min( std::numeric_limits<TPixel>::max() );
  TPixel max( std::numeric_limits<TPixel>::min() );
  mitk::PointSet::PointsContainer* points = m_PointSet->GetPointSet()->GetPoints();
  for ( mitk::PointSet::PointsConstIterator pointsIterator = points->Begin(); 
        pointsIterator != points->End();
        ++pointsIterator ) 
  {
    // first test if this point is inside the image at all
    if ( !imageGeometry->IsInside( pointsIterator.Value()) ) 
    {
      continue;
    }

    // convert world coordinates to image indices
    imageGeometry->WorldToIndex( pointsIterator.Value(), seedIndex);

    // get the pixel value at this point
    TPixel currentPixelValue = itkImage->GetPixel( seedIndex );

    // adjust minimum and maximum values
    if (currentPixelValue > max)
      max = currentPixelValue;

    if (currentPixelValue < min)
      min = currentPixelValue;

    regionGrower->AddSeed( seedIndex );
  }

  std::cout << "Values between " << min << " and " << max << std::endl;

  min -= 30;
  max += 30;

  // set thresholds and execute filter
  regionGrower->SetLower( min );
  regionGrower->SetUpper( max );

  regionGrower->Update();

  mitk::Image::Pointer resultImage = mitk::ImportItkImage( regionGrower->GetOutput() );
  mitk::DataNode::Pointer newNode = mitk::DataNode::New();
  newNode->SetData( resultImage );

  // set some properties
  newNode->SetProperty("binary", mitk::BoolProperty::New(true));
  newNode->SetProperty("name", mitk::StringProperty::New("dumb segmentation"));
  newNode->SetProperty("color", mitk::ColorProperty::New(1.0,0.0,0.0));
  newNode->SetProperty("volumerendering", mitk::BoolProperty::New(false));
  newNode->SetProperty("layer", mitk::IntProperty::New(1));
  newNode->SetProperty("opacity", mitk::FloatProperty::New(0.5));

  // add result to data tree
  this->GetDefaultDataStorage()->Add( newNode, parent );
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

