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
mitk::BoundingBox::Pointer mitk::DataTree::ComputeBoundingBox(mitk::DataTreeIterator * it, const char* boolPropertyKey, mitk::BaseRenderer* renderer, const char* boolPropertyKey2)
{
  mitk::DataTreeIterator* _it=it->clone();
  mitk::BoundingBox::Pointer m_BoundingBox=mitk::BoundingBox::New();
  mitk::BoundingBox::PointsContainer::Pointer pointscontainer=mitk::BoundingBox::PointsContainer::New();

  mitk::BoundingBox::PointIdentifier pointid=0;

  while (_it->hasNext())
  {
    _it->next();
    mitk::DataTreeNode::Pointer node = _it->get();
    assert(node.IsNotNull());
    if (node->GetData() != NULL && node->GetData()->GetUpdatedGeometry() != NULL && node->IsOn(boolPropertyKey, renderer) && node->IsOn(boolPropertyKey2, renderer)) 
    {
      const Geometry3D* geometry = node->GetData()->GetUpdatedGeometry();
      unsigned char i;
      for(i=0; i<8; ++i)
        pointscontainer->InsertElement( pointid++, geometry->GetCornerPoint(i));
    }
  }

  mitk::BoundingBox::Pointer result = mitk::BoundingBox::New();
  result->SetPoints(pointscontainer);
  result->ComputeBoundingBox();

  delete _it;
  return result;
}

mitk::TimeBounds mitk::DataTree::ComputeTimeBoundsInMS(mitk::DataTreeIterator * it, const char* boolPropertyKey, mitk::BaseRenderer* renderer, const char* boolPropertyKey2)
{
  TimeBounds timebounds;

  mitk::DataTreeIterator* _it=it->clone();

  mitk::ScalarType stmin, stmax, cur;

  stmin=-ScalarTypeNumericTraits::max();
  stmax= ScalarTypeNumericTraits::max();

  timebounds[0]=stmax; timebounds[1]=stmin;

  while (_it->hasNext())
  {
    _it->next();
    mitk::DataTreeNode::Pointer node = _it->get();
    if (node->GetData() != NULL && node->GetData()->GetUpdatedGeometry() != NULL && node->IsOn(boolPropertyKey, renderer) && node->IsOn(boolPropertyKey2, renderer))
    {
      mitk::Geometry3D::ConstPointer geometry = node->GetData()->GetGeometry();

      cur=geometry->GetTimeBoundsInMS()[0];
      //is it after than -infinity, but before everything else that we found until now?
      if((cur>stmin) && (cur<timebounds[0]))
        timebounds[0] = cur;

      cur=geometry->GetTimeBoundsInMS()[1];
      //is it before than infinity, but after everything else that we found until now?
      if((cur<stmax) && (cur>timebounds[1]))
        timebounds[1] = cur;
    }
  }

  if(!(timebounds[0]<stmax))
  {
    timebounds[0] = stmin;
    timebounds[1] = stmax;
  }

  delete _it;

  return timebounds;
}
