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


// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>

// Qmitk
#include "QmitkRegionGrowingView.h"

//! [cpp-includes]
// Qmitk
#include "QmitkPointListWidget.h"
#include "QmitkRenderWindow.h"

// MITK
#include "mitkImageAccessByItk.h"
#include "mitkITKImageImport.h"
#include "mitkProperties.h"
#include "mitkColorProperty.h"
#include "mitkImageCast.h"

// ITK
#include <itkConnectedThresholdImageFilter.h>
//! [cpp-includes]

// Qt
#include <QMessageBox>


const std::string QmitkRegionGrowingView::VIEW_ID = "org.mitk.views.example.regiongrowing";

QmitkRegionGrowingView::QmitkRegionGrowingView()
  : m_PointListWidget(NULL)
{
}

void QmitkRegionGrowingView::SetFocus()
{
  m_Controls.buttonPerformImageProcessing->setFocus();
}

void QmitkRegionGrowingView::CreateQtPartControl( QWidget *parent )
{
  // create GUI widgets from the Qt Designer's .ui file
  m_Controls.setupUi( parent );
  connect( m_Controls.buttonPerformImageProcessing, SIGNAL(clicked()), this, SLOT(DoImageProcessing()) );

  //! [cpp-createqtpartcontrol]
  // create a QmitkPointListWidget and add it to the widget created from .ui file
  m_PointListWidget = new QmitkPointListWidget();
  m_Controls.verticalLayout->addWidget(m_PointListWidget, 1);

  // retrieve a possibly existing IRenderWindowPart
  if (mitk::IRenderWindowPart* renderWindowPart = GetRenderWindowPart())
  {
    // let the point set widget know about the render window part (crosshair updates)
    RenderWindowPartActivated(renderWindowPart);
  }

  // create a new DataNode containing a PointSet with some interaction
  m_PointSet = mitk::PointSet::New();
  mitk::DataNode::Pointer pointSetNode = mitk::DataNode::New();
  pointSetNode->SetData( m_PointSet );
  pointSetNode->SetName("seed points for region growing");
  pointSetNode->SetProperty("helper object", mitk::BoolProperty::New(true) );
  pointSetNode->SetProperty("layer", mitk::IntProperty::New(1024) );

  // add the pointset to the data storage (for rendering and access by other modules)
  GetDataStorage()->Add( pointSetNode );

  // tell the GUI widget about the point set
  m_PointListWidget->SetPointSetNode( pointSetNode );
  //! [cpp-createqtpartcontrol]
}

void QmitkRegionGrowingView::OnSelectionChanged( berry::IWorkbenchPart::Pointer /*source*/,
                                             const QList<mitk::DataNode::Pointer>& nodes )
{
  // iterate all selected objects, adjust warning visibility
  foreach( mitk::DataNode::Pointer node, nodes )
  {
    if( node.IsNotNull() && dynamic_cast<mitk::Image*>(node->GetData()) )
    {
      m_Controls.labelWarning->setVisible( false );
      m_Controls.buttonPerformImageProcessing->setEnabled( true );
      return;
    }
  }

  m_Controls.labelWarning->setVisible( true );
  m_Controls.buttonPerformImageProcessing->setEnabled( false );
}

void QmitkRegionGrowingView::RenderWindowPartActivated(mitk::IRenderWindowPart* renderWindowPart)
{
  // let the point set widget know about the slice navigation controllers
  // in the active render window part (crosshair updates)
  foreach(QmitkRenderWindow* renderWindow, renderWindowPart->GetQmitkRenderWindows().values())
  {
    m_PointListWidget->AddSliceNavigationController(renderWindow->GetSliceNavigationController());
  }
}

void QmitkRegionGrowingView::RenderWindowPartDeactivated(mitk::IRenderWindowPart* renderWindowPart)
{
  foreach(QmitkRenderWindow* renderWindow, renderWindowPart->GetQmitkRenderWindows().values())
  {
    m_PointListWidget->RemoveSliceNavigationController(renderWindow->GetSliceNavigationController());
  }
}

void QmitkRegionGrowingView::DoImageProcessing()
{
  QList<mitk::DataNode::Pointer> nodes = this->GetDataManagerSelection();
  if (nodes.empty()) return;

  mitk::DataNode* node = nodes.front();

  if (!node)
  {
    // Nothing selected. Inform the user and return
    QMessageBox::information( NULL, "Template", "Please load and select an image before starting image processing.");
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
      //! [cpp-doimageprocessing]
      // So we have an image. Let's see if the user has set some seed points already
      if (m_PointSet->GetSize() == 0)
      {
        // no points there. Not good for region growing
        QMessageBox::information( NULL,
                                  "Region growing functionality",
                                  "Please set some seed points inside the image first.\n"
                                  "(hold Shift key and click left mouse button inside the image.)"
                                  );
        return;
      }

      // actually perform region growing. Here we have both an image and some seed points
      AccessByItk_1( image, ItkImageProcessing, image->GetGeometry() ) // some magic to call the correctly templated function
      //! [cpp-doimageprocessing]
    }
  }
}

//! [cpp-itkimageaccess]
template < typename TPixel, unsigned int VImageDimension >
void QmitkRegionGrowingView::ItkImageProcessing( itk::Image< TPixel, VImageDimension >* itkImage, mitk::BaseGeometry* imageGeometry )
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

  MITK_INFO << "Values between " << min << " and " << max;

  min -= 30;
  max += 30;

  // set thresholds and execute filter
  regionGrower->SetLower( min );
  regionGrower->SetUpper( max );

  regionGrower->Update();

  mitk::Image::Pointer resultImage;
  mitk::CastToMitkImage( regionGrower->GetOutput(), resultImage );
  mitk::DataNode::Pointer newNode = mitk::DataNode::New();
  newNode->SetData( resultImage );

  // set some properties
  newNode->SetProperty("binary", mitk::BoolProperty::New(true));
  newNode->SetProperty("name", mitk::StringProperty::New("dumb segmentation"));
  newNode->SetProperty("color", mitk::ColorProperty::New(1.0,0.0,0.0));
  newNode->SetProperty("volumerendering", mitk::BoolProperty::New(true));
  newNode->SetProperty("layer", mitk::IntProperty::New(1));
  newNode->SetProperty("opacity", mitk::FloatProperty::New(0.5));

  // add result to data tree
  this->GetDataStorage()->Add( newNode );
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}
//! [cpp-itkimageaccess]
