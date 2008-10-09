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


/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/
#include <qstring.h>
#include <mitkBaseRenderer.h>
#include <mitkRenderingManager.h>

#include "mitkPointOperation.h"


const int QmitkPointListWidget::UNLIMITED = 999;

void QmitkPointListWidget::init()
{
  m_DataChangedCommand = itk::SimpleMemberCommand<QmitkPointListWidget>::New();
  m_DataChangedCommand->SetCallbackFunction(this, &QmitkPointListWidget::ItemsOfListUpdate);
  m_CurrentObserverID = 0;
  m_CurrentInteraction = NULL;
  m_CurrentPolygonInteraction = NULL;
  m_Color[0] = (0.0f);
  m_Color[1] = (1.0f);
  m_Color[2] = (1.0f);
  m_Label = "";
  m_Name = "PointSet";
  m_PointSetNode = NULL;
  m_NumberOfPoints = UNLIMITED;
  this->m_SetPoints->setEnabled(false);
  this->m_ClearPointSet->setEnabled(false);
  this->m_LoadPointSet->setEnabled(false);
  this->m_SavePointSet->setEnabled(false);
  this->ShowPointSetActionButtons(false);
}

void QmitkPointListWidget::PointSelect( int ItemIndex )
//when a point is selected in the widget, then an event equal to an user event gets sent to the global interaction so that the data (point) is selected
//TODO: change the event EIDLEFTMOUSEBTN to something more defined, cause when user interaction changes in xml-file, 
//then this don't work anymore (then change the event here too)
{  
  this->m_SetPoints->setOn(true);
  assert(m_PointSet.IsNotNull());
  mitk::PointSet::PointType ppt;

  // convert item index given from the list box into
  // a point id referring to the Nth point in the point list.
  // This is necessary, because the mitk::PointSet uses an itk::MapContainer
  // as points container and thus indexes must not necessarily be strictly 
  // increasing. This is the case if points have been deleted from the list.
  // Unfortunately, there is no such fnction in the mitk::PointSet.
  mitk::PointSet::PointIdentifier pointId = 0;
  //unsigned int pointIndex; // Momentary not used.
  mitk::PointSet::PointsContainer::Iterator it = m_PointSet->GetPointSet()->GetPoints()->Begin();  
  for ( unsigned int currentIndex = 0 ; (signed)currentIndex <= ItemIndex ; ++currentIndex, ++it )
  {
    if ( it == m_PointSet->GetPointSet()->GetPoints()->End())
    {
      itkGenericOutputMacro("couldn't determine point id from index");
      return;  
    }    
    if ( (signed)currentIndex == ItemIndex )
      pointId = it->Index();
  }   

  if ( m_PointSet->GetPointIfExists( pointId, &ppt))
  {
    mitk::Point2D p2d;
    mitk::Point3D p3d;
    mitk::FillVector3D(p3d, (ppt)[0],(ppt)[1],(ppt)[2]);
    mitk::BaseRenderer::Pointer anyRenderer = mitk::BaseRenderer::GetByName("mitkWidget1");

    mitk::PositionEvent event(anyRenderer, 0, 0, 0, mitk::Key_unknown, p2d, p3d);
    mitk::StateEvent *stateEvent = new mitk::StateEvent(mitk::EIDLEFTMOUSEBTN , &event);    

    mitk::GlobalInteraction::GetInstance()->HandleEvent( stateEvent );
    stateEvent->Set(mitk::EIDLEFTMOUSERELEASE , &event);
    mitk::GlobalInteraction::GetInstance()->HandleEvent( stateEvent );

    delete stateEvent;
  }
}

void QmitkPointListWidget::ItemsOfListUpdate()
{
  emit PointListChanged();
  if (m_PointSetNode.IsNull())
  {
    m_NumberOfPointsLabel->setText(QString::number(0));
    InteractivePointList->clear();
    return;
  }

  //mitk::StringProperty::Pointer label = mitk::StringProperty::New(m_Label);
  mitk::ColorProperty::Pointer color = mitk::ColorProperty::New(m_Color[0], m_Color[1], m_Color[2]);
  //m_PointSetNode->SetProperty("label",label);
  m_PointSetNode->SetProperty("color",color);

  int lastSelectedPoint = InteractivePointList->currentItem();

  InteractivePointList->clear();
  m_PointSet = (mitk::PointSet*)(m_PointSetNode->GetData());
  int size =m_PointSet->GetSize();
  m_NumberOfPointsLabel->setText(QString::number(size));
  if (size!=0)
  {
    const mitk::PointSet::DataType::Pointer Pointlist= m_PointSet->GetPointSet();
    mitk::PointSet::PointsContainer::Iterator it, end;
    end = m_PointSet->GetPointSet()->GetPoints()->End();
    unsigned int i=0;
    mitk::PointSet::PointType ppt;
    for (it = m_PointSet->GetPointSet()->GetPoints()->Begin(); it!=end; it++,i++)
    {
      if ( m_PointSet->GetPointIfExists( it->Index(), &ppt))
      {
        std::stringstream  aStrStream;
        aStrStream<<prefix<<i+1<<"  ("<< (ppt)[0]<<",  "<<(ppt)[1]<<",  "<<(ppt)[2]<<")"<<", Index "<< it->Index();
        const std::string s = aStrStream.str();
        const char * Item =s.c_str();
        this->InteractivePointList->insertItem(Item);
      }
    }
    //
    // if there is a selected point in the point set, it will
    // also be highlighted in the pointlist widget.
    //
    int selectedPointIndex = m_PointSet->SearchSelectedPoint();
    if (selectedPointIndex == -1) 
      selectedPointIndex = lastSelectedPoint;
    if (selectedPointIndex > -1 && m_PointSet->GetPointSet()->GetPoints()->IndexExists(selectedPointIndex))
    {
      //
      // convert index into a number ranging from [0..N-1]
      // this is necessary, since if the user deletes points, the point
      // indices are not monotonically increasing.
      //
      int selectedItem = 0;
      for (it = m_PointSet->GetPointSet()->GetPoints()->Begin(); it->Index()!= (unsigned)selectedPointIndex; it++,selectedItem++)
      {}

      // select the currently active point in the point
      // list box      
      this->InteractivePointList->setSelected( selectedItem, true );

      // center the currently active point in the vertical
      // scroll bar
      this->InteractivePointList->centerCurrentItem();
    }
    else
    {
      this->InteractivePointList->clearSelection();
    }
  }
}

void QmitkPointListWidget::RemoveInteraction()
{
  mitk::PointSet::Pointer pointset;

  if (m_CurrentInteraction.IsNotNull())
  {
    //remove last Interactor
    mitk::GlobalInteraction::GetInstance()
      ->RemoveInteractor( m_CurrentInteraction );
    pointset = dynamic_cast<mitk::PointSet*>(m_PointSetNode->GetData());
    assert(pointset.IsNotNull());
    pointset->RemoveObserver(m_CurrentObserverID);
    m_CurrentInteraction = NULL;
  }
}

void QmitkPointListWidget::AddInteraction()
{
  this->RemoveInteraction();
  if (m_PointSetNode.IsNotNull())
  {
    mitk::PointSetInteractor::Pointer sop;
    mitk::PointSet::Pointer pointset;

    mitk::ColorProperty::Pointer color = mitk::ColorProperty::New(m_Color[0], m_Color[1], m_Color[2]);

    sop = dynamic_cast<mitk::PointSetInteractor*>( m_PointSetNode->GetInteractor());
    if (sop.IsNull())
    {

      //new layer property
      mitk::IntProperty::Pointer layer = mitk::IntProperty::New(1);
      mitk::StringProperty::Pointer name = mitk::StringProperty::New(m_Name);


      //if necessary create a DataElement that holds the points
      if(m_PointSet.IsNull())
        m_PointSet = mitk::PointSet::New();

      //declaring a new Interactor
      if (m_NumberOfPoints!=UNLIMITED)//limited number of points
        sop = mitk::PointSetInteractor::New("pointsetinteractor", m_PointSetNode, m_NumberOfPoints);
      else   //unlimited number of points
        sop = mitk::PointSetInteractor::New("pointsetinteractor", m_PointSetNode);

      //m_PointSetNode: and set the data, layer and Interactor
      m_PointSetNode->SetData(m_PointSet);
      m_PointSetNode->SetProperty("layer",layer);
      m_PointSetNode->SetProperty("color",color);
      
      m_PointSetNode->SetInteractor(sop);
      m_PointSetNode->SetProperty("name", name);
    }
    m_PointSetNode->SetProperty("color",color);
    if (m_CurrentInteraction.IsNotNull())
    {
      //remove last Interactor
      mitk::GlobalInteraction::GetInstance()
        ->RemoveInteractor(m_CurrentInteraction);

      assert(m_PointSet.IsNotNull());
      m_PointSet->RemoveObserver(m_CurrentObserverID);
    }

    assert(m_PointSet.IsNotNull());
    m_CurrentObserverID = m_PointSet->AddObserver(itk::EndEvent(), m_DataChangedCommand);

    //new Interactor
    m_CurrentInteraction = sop;
    prefix= m_Label;
    //tell the global Interactor, that there is another one to ask if it can handle the event
    mitk::GlobalInteraction::GetInstance()->AddInteractor(m_CurrentInteraction);
  }
  ItemsOfListUpdate();
}

void QmitkPointListWidget::SwitchInteraction( mitk::PointSetInteractor::Pointer *sop, mitk::DataTreeNode::Pointer * node, int numberOfPoints, mitk::Point3D c, std::string l )
{
  mitk::PointSet::Pointer pointset;
  if ((*sop).IsNull())
  {
    //new layer property
    mitk::IntProperty::Pointer layer = mitk::IntProperty::New(1);
    mitk::ColorProperty::Pointer color = mitk::ColorProperty::New(c[0],c[1],c[2]);

    mitk::BoolProperty::Pointer contour = mitk::BoolProperty::New(false);
    mitk::BoolProperty::Pointer close = mitk::BoolProperty::New(true);


    //if necessary create a TreeNode, to connect the data to...
    if((*node).IsNull())
      m_PointSetNode = mitk::DataTreeNode::New();
    else
      m_PointSetNode = *node;

    //if necessary create a DataElement that holds the points
    if(dynamic_cast<mitk::PointSet*>(m_PointSetNode->GetData()) == NULL)
      pointset = mitk::PointSet::New();
    else
      pointset = dynamic_cast<mitk::PointSet*>(m_PointSetNode->GetData());

    //declaring a new Interactor
    if (numberOfPoints!=UNLIMITED)//limited number of points
      *sop = mitk::PointSetInteractor::New("pointsetinteractor", m_PointSetNode, numberOfPoints);
    else   //unlimited number of points
      *sop = mitk::PointSetInteractor::New("pointsetinteractor", m_PointSetNode);

    //m_PointSetNode: and set the data, layer and Interactor
    m_PointSetNode->SetData(pointset);
    m_PointSetNode->SetProperty("layer",layer);
    m_PointSetNode->SetProperty("color",color);
    m_PointSetNode->SetProperty("contour",contour);
    m_PointSetNode->SetProperty("close",close);
    m_PointSetNode->SetInteractor(*sop);

    *node = m_PointSetNode;


  }

  if (m_CurrentInteraction.IsNotNull())
  {
    //remove last Interactor
    mitk::GlobalInteraction::GetInstance()
      ->RemoveInteractor(m_CurrentInteraction);

    pointset = dynamic_cast<mitk::PointSet*>(m_PointSetNode->GetData());
    assert(pointset.IsNotNull());
    pointset->RemoveObserver(m_CurrentObserverID);
  }

  //connect data to refresh method
  pointset = dynamic_cast<mitk::PointSet*>((*node)->GetData());
  assert(pointset.IsNotNull());
  m_CurrentObserverID = pointset->AddObserver(itk::EndEvent(), m_DataChangedCommand);

  //new Interactor
  m_CurrentInteraction = *sop;
  prefix= l;
  m_PointSetNode= *node;
  //tell the global Interactor, that there is another one to ask if it can handle the event
  mitk::GlobalInteraction::GetInstance()->AddInteractor(m_CurrentInteraction);
  ItemsOfListUpdate();
}

void QmitkPointListWidget::SwitchInteraction( mitk::PolygonInteractor::Pointer *sop, mitk::DataTreeNode::Pointer * node, int itkNotUsed(numberOfPoints), mitk::Point3D c, std::string l )
{
  if ((*sop).IsNull())
  {
    //new layer property
    std::cout << "creating polygon interactor ..." << std::endl;

    mitk::IntProperty::Pointer layer = mitk::IntProperty::New(1);
    mitk::ColorProperty::Pointer color = mitk::ColorProperty::New(c[0],c[1],c[2]);
    //mitk::StringProperty::Pointer label = mitk::StringProperty::New(l);
    mitk::StringProperty::Pointer name = mitk::StringProperty::New(l);
    //create a DataElement that holds the points
    mitk::Mesh::Pointer meshpointset = mitk::Mesh::New();

    //connect data to refresh method
    meshpointset->AddObserver(itk::EndEvent(), m_DataChangedCommand);

    //then crate a TreeNode, to connect the data to...
    m_PointSetNode = mitk::DataTreeNode::New();
    *sop = mitk::PolygonInteractor::New("polygoninteractor", m_PointSetNode);


    //m_PointSetNode: and set the data, layer and Interactor
    m_PointSetNode->SetData(meshpointset);
    m_PointSetNode->SetProperty("layer",layer);
    m_PointSetNode->SetProperty("color",color);
    //m_PointSetNode->SetProperty("label",label);
    m_PointSetNode->SetInteractor(*sop);
    //m_PointSetNode->SetProperty("name", name); /// name is identical with label????
    *node = m_PointSetNode;

    ////then add it to the existing DataTree
    //m_DataTreeIterator->add(m_PointSetNode);
  }

  if (m_CurrentPolygonInteraction.IsNotNull())
  {
    //remove last Interactor
    mitk::GlobalInteraction::GetInstance()
      ->RemoveInteractor(m_CurrentPolygonInteraction);
  }

  //new Interactor
  m_CurrentPolygonInteraction = *sop;
  prefix= l;
  m_PointSetNode= *node;
  //tell the global Interactor, that there is another one to ask if it can handle the event
  mitk::GlobalInteraction::GetInstance()->AddInteractor(m_CurrentPolygonInteraction);
  ItemsOfListUpdate();
}


mitk::DataTreeNode* QmitkPointListWidget::GetDataTreeNode()
{
  return m_PointSetNode.GetPointer();
}


void QmitkPointListWidget::Reinitialize( bool disableInteraction )
{
  // disable previous pointset
  if ( m_PointSet.IsNotNull() )
    m_PointSet->RemoveObserver( m_CurrentObserverID );
  if ( disableInteraction )
    mitk::GlobalInteraction::GetInstance()->RemoveInteractor( m_CurrentInteraction );

  // reset member variables
  m_DataChangedCommand = itk::SimpleMemberCommand<QmitkPointListWidget>::New();
  m_DataChangedCommand->SetCallbackFunction(this, &QmitkPointListWidget::ItemsOfListUpdate);
  m_CurrentObserverID = 0;
  m_CurrentInteraction = NULL;
  m_CurrentPolygonInteraction = NULL;
  m_PointSetNode = NULL;
  m_PointSet = NULL;

  // reset gui
  InteractivePointList->clear();
  m_NumberOfPointsLabel->setText(QString::number(0));
}

void QmitkPointListWidget::keyPressEvent( QKeyEvent * e )
{
  //pass the event to the mitk global interaction 
  mitk::BaseRenderer::Pointer anyRenderer = mitk::BaseRenderer::GetByName("mitkWidget1");
  mitk::Event *tempEvent = new mitk::Event(anyRenderer, e->type(), mitk::BS_NoButton, mitk::BS_NoButton, e->key());
  mitk::EventMapper::MapEvent(tempEvent);
  delete tempEvent;

  if(e->key() == QKeyEvent::Key_F2)
  {
    mitk::PointOperation* doOp = new mitk::PointOperation(
      mitk::OpMOVEPOINTUP, m_PointSet->SearchSelectedPoint(), m_PointSet->GetPoint( m_PointSet->SearchSelectedPoint() ));

    //execute the Operation
    m_PointSet->ExecuteOperation(doOp);

  }
  if(e->key() == QKeyEvent::Key_F3)
  {
    mitk::PointOperation* doOp = new mitk::PointOperation(
      mitk::OpMOVEPOINTDOWN, m_PointSet->SearchSelectedPoint(), m_PointSet->GetPoint( m_PointSet->SearchSelectedPoint() ));


    //execute the Operation
    m_PointSet->ExecuteOperation(doOp);

  }

}

void QmitkPointListWidget::OnSetPointsToggled(bool setPoints)
{
  if (setPoints == false)
    this->RemoveInteraction();
  else
  {
    if (m_PointSetNode.IsNotNull())
      this->AddInteraction();
  }
}

void QmitkPointListWidget::OnClearPointSetClicked()
{
  switch( QMessageBox::question(this, "Clear current pointset...", "Do you really want do clear the current point set?",
    QMessageBox::Yes | QMessageBox::Default, QMessageBox::No, QMessageBox::Cancel | QMessageBox::Escape)) 
  {
  case QMessageBox::Yes: 
    {
      mitk::PointSet::DataType::PointsContainer::Pointer pointsContainer = m_PointSet->GetPointSet()->GetPoints();
      pointsContainer->Initialize(); // a call to initialize results in clearing the points container
      this->ItemsOfListUpdate();
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    }
  default: 
    return;
  }
}

void QmitkPointListWidget::OnLoadPointSetClicked()
{
  //Ask for a file name
  //read the file by the data tree node factory
  //add an interactor to the point set
  //add it to the data tree
  //register it with the poitn list widget

  //
  // get the name of the file to load
  //
  QString filename = QFileDialog::getOpenFileName( QString::null, "MITK Point-Sets (*.mps)", NULL );
  if ( filename.isEmpty() )
    return ;

  //
  // instantiate a reader and load the file
  //
  mitk::PointSetReader::Pointer reader = mitk::PointSetReader::New();
  reader->SetFileName( filename.latin1() );
  reader->Update();

  mitk::PointSet::Pointer pointSet = reader->GetOutput();
  if ( pointSet.IsNull() )
  {
    QMessageBox::warning( this, "Load point set", "Something went wront during loading file..." );
    return;
  }

  //
  // fill the data tree node with the appropriate information
  //
  if(m_PointSetNode.IsNotNull() && m_PointSet.IsNotNull())
  {
    this->RemoveInteraction();
    m_PointSet = pointSet;
    m_PointSetNode->SetData( m_PointSet );
    if (this->m_SetPoints->isOn())
    {
      this->AddInteraction();
    }
    else
    {
      this->ItemsOfListUpdate();
    }
  }  
}

void QmitkPointListWidget::OnSavePointSet()
{
  if (m_PointSet.IsNull())
  {
    QMessageBox::warning( this, "Save point set", "A valid point set has to be selected first..." );
    return;
  }

  //
  // let the user choose a file
  //
  std::string name = "";

  QString fileNameProposal = "PointSet";
  fileNameProposal.replace(' ','_');
  fileNameProposal.append(".mps");
  QString aFilename = QFileDialog::getSaveFileName( fileNameProposal.latin1() );
  if ( !aFilename )
    return ;

  //
  // instantiate the writer and add the point-sets to write
  //
  mitk::PointSetWriter::Pointer writer = mitk::PointSetWriter::New();
  writer->SetInput( m_PointSet );
  writer->SetFileName( aFilename.latin1() );
  writer->Update();
}

void QmitkPointListWidget::SetPointColor(mitk::Color color)
{
  // the color for the points to be shown in render windows, has to be set before adding a new point set node
  m_Color = color;
}

void QmitkPointListWidget::SetName(std::string name)
{
  // the name of the point set, has to be set before adding a new point set node
  m_Name = name;
}

void QmitkPointListWidget::SetPointSetNode(mitk::DataTreeNode::Pointer pointSetNode)
{
  // has to be called to change the point set shown in the list view, controls the interaction process
  this->RemoveInteraction();
  m_PointSetNode = pointSetNode;
  m_PointSet = NULL;
  if(m_PointSetNode.IsNotNull())
  {
    m_PointSet = (mitk::PointSet*)(m_PointSetNode->GetData());
    this->m_SetPoints->setEnabled(true);
    this->m_ClearPointSet->setEnabled(true);
    this->m_LoadPointSet->setEnabled(true);
    this->m_SavePointSet->setEnabled(true);
  }
  else
  {
    this->m_SetPoints->setEnabled(false);
    this->m_SetPoints->setOn(false);
    this->m_ClearPointSet->setEnabled(false);
    this->m_LoadPointSet->setEnabled(false);
    this->m_SavePointSet->setEnabled(false);
  }
  if (this->m_SetPoints->isOn())
  {
    this->AddInteraction();
  }
  else
  {
    this->ItemsOfListUpdate();
  }
}

void QmitkPointListWidget::SetNumberOfPoints(int numberOfPoints)
{
  // the maximum number of points for the point set, has to be set before adding a new point set node
  m_NumberOfPoints = numberOfPoints;
}

void QmitkPointListWidget::DeactivateInteractor(bool removeInteractor)
{
  if (removeInteractor)
  {
    // deactivates the interactor
    this->RemoveInteraction();
    this->m_SetPoints->setOn(false);
  }
}

void QmitkPointListWidget::ShowPointSetActionButtons(bool show)
{
  // show buttons for special point set functions, to make them available
  if (show)
  {
    this->m_SetPoints->show();
    this->m_ClearPointSet->show();
    this->m_LoadPointSet->show();
    this->m_SavePointSet->show();
  }
  else
  {
    this->m_SetPoints->hide();
    this->m_ClearPointSet->hide();
    this->m_LoadPointSet->hide();
    this->m_SavePointSet->hide();
  }
}
