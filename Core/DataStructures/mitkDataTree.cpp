#include "mitkDataTree.h"

//##ModelId=3E38F46A0190
mitk::DataTree::DataTree() : 
DataTreeBase( )
{
  addTreeChangeListener( this );
  setRoot(mitk::DataTreeNode::New());
}


//##ModelId=3E38F46A01AE
mitk::DataTree::~DataTree()
{
}

/**
*
*/
//##ModelId=3E3FE0430148
TreeIterator<mitk::DataTreeNode::Pointer>* mitk::DataTree::GetNext( const char* propertyKey, const mitk::BaseProperty* property,  TreeIterator<mitk::DataTreeNode::Pointer>* startPosition ){
  
  if ( startPosition == NULL )
    startPosition = preorderIterator();
  
  TreeIterator<mitk::DataTreeNode::Pointer>* pos = startPosition->clone();
  mitk::DataTreeNode::Pointer dtn;
  
  
  while ( pos->hasNext() ) {
    
    dtn = pos->next();
    mitk::PropertyList::Pointer propertyList = dtn->GetPropertyList();
    mitk::BaseProperty::Pointer tmp = propertyList->GetProperty( propertyKey );
    if ( (*property) == *(tmp) )
      return pos;			
  }
  
  delete pos;
  return NULL;
}

/**
*
*/
//##ModelId=3EA6ADB7029F
void mitk::DataTree::treeChanged( TreeIterator<DataTreeNode::Pointer>& changedTreePosition ) {
  
  Modified();
}

//##ModelId=3ED91D050085
mitk::BoundingBox::Pointer mitk::DataTree::ComputeBoundingBox(mitk::DataTreeIterator * it)
{
  mitk::DataTreeIterator* _it=it->clone();
  mitk::BoundingBox::Pointer m_BoundingBox=mitk::BoundingBox::New();
  mitk::BoundingBox::PointsContainer::Pointer pointscontainer=mitk::BoundingBox::PointsContainer::New();
  
  mitk::ScalarType nullpoint[]={0,0,0};
  mitk::BoundingBox::PointType p(nullpoint);
  
  mitk::BoundingBox::PointIdentifier pointid=0;
  
  while (_it->hasNext())
  {
    _it->next();
    mitk::DataTreeNode::Pointer node = _it->get();
    if (node->GetData() != NULL && node->GetData()->GetUpdatedGeometry()!=NULL) 
    {
      mitk::BoundingBox::ConstPointer nextBoundingBox = node->GetData()->GetGeometry()->GetBoundingBox();
      const mitk::BoundingBox::PointsContainer * nextPoints = nextBoundingBox->GetPoints();
      if(nextPoints!=NULL)
      {
        mitk::BoundingBox::PointsContainer::ConstIterator pointsIt = nextPoints->Begin();
      
        while (pointsIt != nextPoints->End() )
        {
          pointscontainer->InsertElement( pointid++, pointsIt->Value());
          ++pointsIt;
        }
      }
    }
  }
  
  mitk::BoundingBox::Pointer result = mitk::BoundingBox::New();
  result->SetPoints(pointscontainer);
  result->ComputeBoundingBox();
  
  delete _it;
  return result;
}

mitk::BoundingBox::Pointer mitk::DataTree::ComputeVisibleBoundingBox(mitk::DataTreeIterator * it, mitk::BaseRenderer* renderer)
{
  mitk::DataTreeIterator* _it=it->clone();
  mitk::BoundingBox::Pointer m_BoundingBox=mitk::BoundingBox::New();
  mitk::BoundingBox::PointsContainer::Pointer pointscontainer=mitk::BoundingBox::PointsContainer::New();
  
  mitk::ScalarType nullpoint[]={0,0,0};
  mitk::BoundingBox::PointType p(nullpoint);
  
  mitk::BoundingBox::PointIdentifier pointid=0;
  
  while (_it->hasNext())
  {
    _it->next();
    mitk::DataTreeNode::Pointer node = _it->get();
    if (node->GetData() != NULL && node->GetData()->GetUpdatedGeometry()!=NULL && node->IsVisible(renderer)) 
    {
      mitk::BoundingBox::ConstPointer nextBoundingBox = node->GetData()->GetGeometry()->GetBoundingBox();
      const mitk::BoundingBox::PointsContainer * nextPoints = nextBoundingBox->GetPoints();
      if(nextPoints!=NULL)
      {
        mitk::BoundingBox::PointsContainer::ConstIterator pointsIt = nextPoints->Begin();
      
        while (pointsIt != nextPoints->End() )
        {
          pointscontainer->InsertElement( pointid++, pointsIt->Value());
          ++pointsIt;
        }
      }
    }
  }
  
  mitk::BoundingBox::Pointer result = mitk::BoundingBox::New();
  result->SetPoints(pointscontainer);
  result->ComputeBoundingBox();
  
  delete _it;
  return result;
}
