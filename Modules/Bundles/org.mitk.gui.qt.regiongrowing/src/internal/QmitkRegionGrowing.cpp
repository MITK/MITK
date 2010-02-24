/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: 15359 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

// class header
#include "QmitkRegionGrowing.h"

// MITK images <--> ITK images interfaces
#include "mitkImageAccessByItk.h"
#include "mitkITKImageImport.h"

// data node 
#include <mitkDataTreeNodeObject.h>

// properties for MITK data nodes
#include <mitkProperties.h>
#include <mitkColorProperty.h>

// a widget for editing a point list (here the points are used as a seed)
#include <QmitkPointListWidget.h>

// the standard display widget
#include <QmitkStdMultiWidget.h>

// we need to be a client of the berry selection service to get notified about selected data nodes
#include <berryISelectionService.h>

// we need to access the selection service from the current berry workbench
#include <berryIWorkbenchWindow.h>

// qt display box
#include <QMessageBox>

// itk filter for region growing
#include <itkConnectedThresholdImageFilter.h>


QmitkRegionGrowing::QmitkRegionGrowing()
: QmitkFunctionality(), 
  m_Controls(0),
  m_MultiWidget(NULL)
{
}

QmitkRegionGrowing::~QmitkRegionGrowing()
{
  // if a selection listener was registered, unregister him
  if(m_SelectionListener.IsNotNull())
  {
    berry::ISelectionService* s = GetSite()->GetWorkbenchWindow()->GetSelectionService();
    if(s)
      s->RemoveSelectionListener(m_SelectionListener);
  }
}

// build up qt view
void QmitkRegionGrowing::CreateQtPartControl(QWidget *parent)
{
  // if not already done
  if (!m_Controls)
  {
    // create GUI widgets from the automatic qt designer code generation
    m_Controls = new Ui::QmitkRegionGrowingControls;
    m_Controls->setupUi(parent);
 
    connect( (QObject*)(m_Controls->m_ButtonRegionGrow), SIGNAL(clicked()), this, SLOT(DoRegionGrowing()) );
   
    // if so, let the point set widget know about the multi widget (crosshair updates)
    m_Controls->m_PointListWidget->SetMultiWidget( m_MultiWidget );
    
    SetStatusText("Please select an image!", true);
  }
  
  // registering the selection listener
  m_SelectionListener = berry::ISelectionListener::Pointer(new berry::SelectionChangedAdapter<QmitkRegionGrowing>
    (this, &QmitkRegionGrowing::SelectionChanged));
  berry::ISelectionService* s = GetSite()->GetWorkbenchWindow()->GetSelectionService();
  if(s)
    s->AddSelectionListener(m_SelectionListener);
}

void QmitkRegionGrowing::StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget)
{
  m_MultiWidget = &stdMultiWidget;
  m_Controls->m_PointListWidget->SetMultiWidget( m_MultiWidget );
}

void QmitkRegionGrowing::StdMultiWidgetNotAvailable()
{
  m_MultiWidget = NULL;
}

void QmitkRegionGrowing::Activated()
{
  QmitkFunctionality::Activated();

  if (m_PointSetNode.IsNull()) // only once create a new DataTreeNode containing a PointSet with some interaction
  {
    // new node and data item
    m_PointSet = mitk::PointSet::New();

    m_PointSetNode = mitk::DataTreeNode::New();
    m_PointSetNode->SetData( m_PointSet );
    m_PointSetNode->SetName("seed points for region growing");
    m_PointSetNode->SetProperty("helper object", mitk::BoolProperty::New(true));
    m_PointSetNode->SetProperty("layer", mitk::IntProperty::New(1024));
  
    // add the pointset to the data tree (for rendering and access by other modules)
    GetDefaultDataStorage()->Add( m_PointSetNode );

    // tell the GUI widget about out point set
    m_Controls->m_PointListWidget->SetPointSetNode( m_PointSetNode );
  }
}

void QmitkRegionGrowing::Deactivated()
{
}

void QmitkRegionGrowing::SelectionChanged( berry::IWorkbenchPart::Pointer, berry::ISelection::ConstPointer selection )
{ 
  
  mitk::DataTreeNodeSelection::ConstPointer _DataTreeNodeSelection 
    = selection.Cast<const mitk::DataTreeNodeSelection>();

  if(_DataTreeNodeSelection.IsNotNull())
  {
    std::vector<mitk::DataTreeNode*> selectedNodes;
    mitk::DataTreeNodeObject* _DataTreeNodeObject = 0;

    for(mitk::DataTreeNodeSelection::iterator it = _DataTreeNodeSelection->Begin();it != _DataTreeNodeSelection->End(); ++it)
    {
      _DataTreeNodeObject = dynamic_cast<mitk::DataTreeNodeObject*>((*it).GetPointer());
      if(_DataTreeNodeObject)
      {
        mitk::DataTreeNode::Pointer node = _DataTreeNodeObject->GetDataTreeNode();
      
        if( node.IsNotNull() && dynamic_cast<mitk::Image*>(node->GetData()) )
           selectedNodes.push_back( node );
      }
    }

    m_SelectedNode = 0;

    mitk::DataTreeNode::Pointer node;
    
    if(selectedNodes.size() > 0)
    {
      node=selectedNodes.front();
    }
      
      if( node.IsNotNull() )
      {
        QString infoText;
      
        if (node->GetName().empty())
          infoText ="Selected Image: [currently selected image has no name]";
        else
          infoText = "Selected Image: " + (QString)node->GetName().c_str();
        infoText = infoText + "\n Add a seed point with shift+ left mouse click.";
       SetStatusText(infoText, false);
      }
      else
      {
        SetStatusText("No image selected! Please select an image!", true);
      }      
      m_SelectedNode = node;
  }
}


void QmitkRegionGrowing::UpdateFromCurrentDataManagerSelection()
{
  //MITK_INFO << "Update selection from DataManager";
  berry::ISelection::ConstPointer selection( this->GetSite()->GetWorkbenchWindow()->GetSelectionService()->GetSelection("org.mitk.views.datamanager"));
  this->SelectionChanged(berry::SmartPointer<IWorkbenchPart>(NULL), selection);
}

void QmitkRegionGrowing::DoRegionGrowing()
{
  mitk::DataTreeNode* node = m_SelectedNode;
  if (!node)
  {
    // Nothing selected. Inform the user and return
    QMessageBox::information( NULL, "Region growing functionality", "Please load and select an image before region growing.");
    return;
  }

  // here we have a valid mitk::DataTreeNode

  // a node itself is not very useful, we need its data item (the image)
  mitk::BaseData* data = node->GetData();
  if (data)
  {
    // test if this data item is an image or not (could also be a surface or something totally different)
    mitk::Image* image = dynamic_cast<mitk::Image*>( data );
    if (image)
    {
      std::string name;
      std::cout << "Performing region growing for image ";
      if (node->GetName(name))
      {
        // a property called "name" was found for this DataTreeNode
        std::cout << "'" << name << "'";
      }
      std::cout << "." << std::endl;

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
      AccessByItk_1( image, ItkImageProcessing, image->GetGeometry() ); // some magic to call the correctly templated function

    }
  }
}

template < typename TPixel, unsigned int VImageDimension >
void QmitkRegionGrowing::ItkImageProcessing( itk::Image< TPixel, VImageDimension >* itkImage, mitk::Geometry3D* imageGeometry )
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
  mitk::DataTreeNode::Pointer newNode = mitk::DataTreeNode::New();
  newNode->SetData( resultImage );

  // set some properties
  newNode->SetProperty("binary", mitk::BoolProperty::New(true));
  newNode->SetProperty("name", mitk::StringProperty::New("dumb segmentation"));
  newNode->SetProperty("color", mitk::ColorProperty::New(1.0,0.0,0.0));
  newNode->SetProperty("volumerendering", mitk::BoolProperty::New(true));
  newNode->SetProperty("layer", mitk::IntProperty::New(1));
  newNode->SetProperty("opacity", mitk::FloatProperty::New(0.5));

  // add result to data tree
  this->GetDefaultDataStorage()->Add( newNode );
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkRegionGrowing::SetStatusText( QString text, bool isError)
{
  QPalette palette;

  if (isError)
  {
    QBrush brush(QColor(255, 0, 0, 255));
    brush.setStyle(Qt::SolidPattern);
    palette.setBrush(QPalette::Active, QPalette::WindowText, brush);
    palette.setBrush(QPalette::Active, QPalette::Text, brush);
    palette.setBrush(QPalette::Active, QPalette::ButtonText, brush);
  }
  else
  {
    QBrush brush(QColor(0, 0, 0, 255));
    brush.setStyle(Qt::SolidPattern);
    palette.setBrush(QPalette::Active, QPalette::WindowText, brush);
    palette.setBrush(QPalette::Active, QPalette::Text, brush);
    palette.setBrush(QPalette::Active, QPalette::ButtonText, brush);
  }

  m_Controls->m_SelectedImageLabel->setText(text);
  m_Controls->m_SelectedImageLabel->setPalette(palette);
    
}