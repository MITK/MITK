/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/
//#include <itkCommand.h>
//#include <mitkPointSetInteractor.h>

mitk::PointSetInteractor::Pointer QmitkPointListWidget::m_CurrentInteraction;
void QmitkPointListWidget::init()
{
  std::cout << "1" << std::endl;
  	m_DataChangedCommand = itk::SimpleMemberCommand<QmitkPointListWidget>::New();
#ifdef WIN32
    m_DataChangedCommand->SetCallbackFunction(this, QmitkPointListWidget::ItemsOfListUpdate);
#else
    m_DataChangedCommand->SetCallbackFunction(this, &QmitkPointListWidget::ItemsOfListUpdate);
#endif    
}



void QmitkPointListWidget::PointSelect( int ItemIndex )
{  
    assert(m_PointSet.IsNotNull());    
    //assert(m_PointSet->IndexExists(ItemIndex));
    mitk::PointSet::PointType ppt;
    mitk::PointSet::PointsContainer::Iterator it, end;  
    unsigned int i=0;
    for (it = m_PointSet->GetPointSet()->GetPoints()->Begin(); i<ItemIndex; it++) i++; ///the iterator will be set to the position needed 
    m_PointSet->GetPointSet()->GetPoints()->GetElementIfIndexExists(it->Index(), &ppt);
    mitk::Point2D p2d;
    mitk::Point3D p3d((ppt)[0],(ppt)[1],(ppt)[2]);
    mitk::PositionEvent event(NULL, 0, 0, 0, mitk::Key_unknown, p2d, p3d);
    mitk::StateEvent *stateEvent = new mitk::StateEvent(mitk::EIDLEFTMOUSEBTN , &event);    
    mitk::GlobalInteraction* globalInteraction = dynamic_cast<mitk::GlobalInteraction*>(mitk::EventMapper::GetGlobalStateMachine());
    if(globalInteraction!=NULL)
    {
	    globalInteraction->HandleEvent(stateEvent, mitk::OperationEvent::GetCurrObjectEventId(), mitk::OperationEvent::GetCurrGroupEventId());
	    stateEvent->Set(mitk::EIDLEFTMOUSERELEASE , &event);
	    globalInteraction->HandleEvent(stateEvent, mitk::OperationEvent::GetCurrObjectEventId(), mitk::OperationEvent::GetCurrGroupEventId());
    }
    delete stateEvent;
    
}


void QmitkPointListWidget::ItemsOfListUpdate()
{
  if (m_DatatreeNode.IsNull())return;
  InteractivePointList->clear();
	m_PointSet = (mitk::PointSet*)(m_DatatreeNode->GetData());	
  int size =m_PointSet->GetSize();
  if (size!=0)
  {
	  const mitk::PointSet::DataType::Pointer Pointlist= m_PointSet->GetPointSet();
////
    mitk::PointSet::PointsContainer::Iterator it, end;
    end = m_PointSet->GetPointSet()->GetPoints()->End();   
    unsigned int i=0;
    mitk::PointSet::PointType ppt;
    for (it = m_PointSet->GetPointSet()->GetPoints()->Begin(); it!=end; it++,i++) 
    {
      m_PointSet->GetPointSet()->GetPoints()->GetElementIfIndexExists(it->Index(), &ppt);    
      std::stringstream  aStrStream;
      aStrStream<<prefix<<i+1<<"  ("<< (ppt)[0]<<",  "<<(ppt)[1]<<",  "<<(ppt)[2]<<")";
      const std::string s = aStrStream.str();
      const char * Item =s.c_str();				
      this->InteractivePointList->insertItem(Item);              

    }
    
/////
    //int i=0;
    //
    //int subindex =0;
    //do
    //{      
    //  if (m_PointSet->IndexExists(subindex))
    //  {         
    //    std::stringstream  aStrStream;
    //    aStrStream<<prefix<<i+1<<"  ("<< Pointlist->GetPoints()->GetElement(subindex)[0]<<",  "<<Pointlist->GetPoints()->GetElement(subindex)[1]<<",  "<<Pointlist->GetPoints()->GetElement(subindex)[2]<<")";
    //    const std::string s = aStrStream.str();
    //    const char * Item =s.c_str();				
    //    this->InteractivePointList->insertItem(Item);        
    //    i++;        
    //  }
    //  subindex++;
    //}while (i<size);
///////
	  //for (i =0 ;i<size; i++)
	  //{
   //   std::stringstream aStrStream;
   //   for (int subindex =0;i)
   //   if (IndexExists(subindex))
   //   {        
   //     aStrStream<<prefix<<i+1<<"  ("<< Pointlist->GetPoints()->GetElement(i)[0]<<",  "<<Pointlist->GetPoints()->GetElement(i)[1]<<",  "<<Pointlist->GetPoints()->GetElement(i)[2]<<")";
   //     const std::string s = aStrStream.str();
   //     const char * Item =s.c_str();				
   //     this->InteractivePointList->insertItem(Item);        
   //   }
   //   
	  //}        
  }
}


void QmitkPointListWidget::SwitchInteraction( mitk::PointSetInteractor::Pointer *sop, mitk::DataTreeNode::Pointer * node, int numberOfPoints, mitk::Point3D c, std::string l )
{
    mitk::GlobalInteraction* globalInteraction = dynamic_cast<mitk::GlobalInteraction*>(mitk::EventMapper::GetGlobalStateMachine());
    if(globalInteraction!=NULL)
    {
	    if ((*sop).IsNull())
	    {
	        //new layer property
	        mitk::IntProperty::Pointer layer = new mitk::IntProperty(1);
	        mitk::ColorProperty::Pointer color = new mitk::ColorProperty(c.x,c.y,c.z);
    	    
	        mitk::BoolProperty::Pointer contour = new mitk::BoolProperty(false);
	        mitk::BoolProperty::Pointer close = new mitk::BoolProperty(true);
    	    
    	    
	        mitk::StringProperty::Pointer label = new mitk::StringProperty(l);
    	    
    	    
	        //create a DataElement that holds the points
	        mitk::PointSet::Pointer pointset = mitk::PointSet::New();
    	    
	        //connect data to refresh method
	        pointset->AddObserver(itk::EndEvent(), m_DataChangedCommand);
    	    
	        //then crate a TreeNode, to connect the data to...
	        mitk::DataTreeNode::Pointer dataTreeNode = mitk::DataTreeNode::New();
	        //declaring a new Interactor
	        if (numberOfPoints!=UNLIMITED)//limited number of points			
		    *sop = new mitk::PointSetInteractor("pointsetinteractor", dataTreeNode, numberOfPoints);
	        else   //unlimited number of points      
		    *sop = new mitk::PointSetInteractor("pointsetinteractor", dataTreeNode);
    	    
    	    
	        //datatreenode: and give set the data, layer and Interactor
	        dataTreeNode->SetData(pointset);
	        dataTreeNode->SetProperty("layer",layer);
	        dataTreeNode->SetProperty("color",color);
	        dataTreeNode->SetProperty("contour",contour);
	        dataTreeNode->SetProperty("close",close);
	        dataTreeNode->SetProperty("label",label);
	        dataTreeNode->SetInteractor(*sop);
	        *node = dataTreeNode;
          
	        ////then add it to the existing DataTree
	        //m_DataTreeIterator->add(dataTreeNode);
	    }
    	
	    if (m_CurrentInteraction.IsNotNull())
	    {
	        //remove last Interactor
	        globalInteraction->RemoveInteractor(m_CurrentInteraction);
	    }
    	
	    //new Interactor
	    m_CurrentInteraction = *sop;
      prefix= l;
      m_DatatreeNode= *node;
	    //tell the global Interactor, that there is another one to ask if it can handle the event
	    globalInteraction->AddInteractor(m_CurrentInteraction);
      ItemsOfListUpdate();
    }
}
