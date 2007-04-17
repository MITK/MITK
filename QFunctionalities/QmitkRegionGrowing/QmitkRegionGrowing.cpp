/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date: 2007-04-17 11:54:52 +0200 (Tue, 17 Apr 2007) $
Version:   $Revision: 10161 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#include "QmitkRegionGrowing.h"
#include "QmitkRegionGrowingControls.h"
#include <qaction.h>
#include <qmessagebox.h>
#include "icon.xpm"
#include "QmitkTreeNodeSelector.h"
#include "QmitkStdMultiWidget.h"

#include <limits>

#include "mitkImageAccessByItk.h"
#include "mitkPointSetInteractor.h"
#include "mitkGlobalInteraction.h"

QmitkRegionGrowing::QmitkRegionGrowing(QObject *parent, const char *name, QmitkStdMultiWidget *mitkStdMultiWidget, mitk::DataTreeIteratorBase* it)
    : QmitkFunctionality(parent, name, it), m_MultiWidget(mitkStdMultiWidget), m_Controls(NULL)
{
  SetAvailability(true);
}

QmitkRegionGrowing::~QmitkRegionGrowing()
{}

QWidget * QmitkRegionGrowing::CreateMainWidget(QWidget *parent)
{
  if ( m_MultiWidget == NULL )
  {
    m_MultiWidget = new QmitkStdMultiWidget( parent );
  }
  return m_MultiWidget;
}

QWidget * QmitkRegionGrowing::CreateControlWidget(QWidget *parent)
{
  if (m_Controls == NULL)
  {
    m_Controls = new QmitkRegionGrowingControls(parent);
  }
  return m_Controls;
}

void QmitkRegionGrowing::CreateConnections()
{
  if ( m_Controls )
  {
    connect( (QObject*)(m_Controls->btnRegionGrow), SIGNAL(clicked()), 
                                              this, SLOT(DoRegionGrowing()) );
  }
}

QAction * QmitkRegionGrowing::CreateAction(QActionGroup *parent)
{
  QAction* action;
  action = new QAction( tr( "ToolTip" ), QPixmap((const char**)icon_xpm), tr( "MenueEintrag" ), 0, parent, "QmitkRegionGrowing" );
  return action;
}

void QmitkRegionGrowing::TreeChanged()
{
  m_Controls->m_TreeNodeSelector->SetDataTreeNodeIterator(this->GetDataTreeIterator());
}

void QmitkRegionGrowing::Activated()
{
  QmitkFunctionality::Activated();

  if (m_PointSetNode.IsNull()) // only once create a new DataTreeNode containing a PointSet with some interaction
  {
    // new node and data item
    m_PointSetNode = mitk::DataTreeNode::New();
    m_PointSet = mitk::PointSet::New();
    m_PointSetNode->SetData( m_PointSet );

    // new behaviour/interaction for the pointset node
    mitk::PointSetInteractor::Pointer interactor = mitk::PointSetInteractor::New("pointsetinteractor", m_PointSetNode);
    mitk::GlobalInteraction::GetInstance()->AddInteractor( interactor );

    // add the pointset to the data tree (for rendering)
    GetDataTreeIterator()->Add( m_PointSetNode );
  }
}

void QmitkRegionGrowing::DoRegionGrowing()
{
  const mitk::DataTreeIteratorClone* iterator = m_Controls->m_TreeNodeSelector->GetSelectedIterator(); // should never fail, the selection widget cares for that
  if (!iterator)
  {
    // Nothing selected. Inform the user and return
    QMessageBox::information( NULL, "Region growing functionality", "Please load and select an image before region growing.");
    return;
  }
    
  mitk::DataTreeNode* node = (*iterator)->Get();
  if ( node )
  {
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
        std::cout << "Performing region growing for image";
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
          QMessageBox::information( NULL, "Region growing functionality", "Please set some seed points inside the image first.\n(hold Shift key and click left mouse button inside the image.)");
          return;
        }

        // actually perform region growing. Here we have both an image and some seed points
        AccessByItk_1( image, ItkImageProcessing, image->GetGeometry() ); // some magic to call the correctly templated function

      }
    }
  }

}

template < typename TPixel, unsigned int VImageDimension >
void QmitkRegionGrowing::ItkImageProcessing( itk::Image< TPixel, VImageDimension >* itkImage, mitk::Geometry3D* imageGeometry )
{
  typedef itk::Image< TPixel, VImageDimension > InputImageType;
  typedef typename InputImageType::IndexType    IndexType;
  IndexType seedIndex;
  
  TPixel min( std::numeric_limits<TPixel>::max() );
  TPixel max( std::numeric_limits<TPixel>::min() );
  for ( mitk::PointSet::PointsConstIterator pointsIterator = m_PointSet->GetPointSet()->GetPoints()->Begin(); // really nice syntax to get an interator for all points
        pointsIterator != m_PointSet->GetPointSet()->GetPoints()->End();
        ++pointsIterator ) 
  {
    // first test if this point is inside the image at all
    if ( !imageGeometry->IsInside( pointsIterator.Value()) ) 
      continue;

    // convert world coordinates to image indices
    imageGeometry->WorldToIndex( pointsIterator.Value(), seedIndex);

    // get the pixel value at this point
    TPixel currentPixelValue = itkImage->GetPixel( seedIndex );

    // adjust minimum and maximum values
    if (currentPixelValue > max)
      max = currentPixelValue;

    if (currentPixelValue < min)
      min = currentPixelValue;
  }

  std::cout << "Values between " << min << " and " << max << std::endl;
}

