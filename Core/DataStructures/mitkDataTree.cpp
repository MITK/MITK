#include "mitkDataTree.h"


//##ModelId=3E38F46A0190
mitk::DataTree::DataTree() : 
DataTreeBase( )
{
  SetRoot(mitk::DataTreeNode::New());
}


//##ModelId=3E38F46A01AE
mitk::DataTree::~DataTree()
{
}

/**
*
*/
//##ModelId=3E3FE0430148
mitk::DataTreeIteratorClone mitk::DataTree::GetNext( const char* propertyKey, const mitk::BaseProperty* property,  mitk::DataTreeIteratorBase* startPosition )
{
  DataTreeIteratorClone pos;

  if(startPosition != NULL)
    pos = *startPosition;
  else
    pos = DataTreePreOrderIterator(this);

  mitk::DataTreeNode::Pointer dtn;
  while ( !pos->IsAtEnd() )
  {
    dtn = pos->Get();
    mitk::PropertyList::Pointer propertyList = dtn->GetPropertyList();
    mitk::BaseProperty::Pointer tmp = propertyList->GetProperty( propertyKey );
    if ( (*property) == *(tmp) )
      return pos;
    ++pos;
  }
  return pos;
}

//##ModelId=3ED91D050085
mitk::BoundingBox::Pointer mitk::DataTree::ComputeBoundingBox(mitk::DataTreeIteratorBase* it, const char* boolPropertyKey, mitk::BaseRenderer* renderer, const char* boolPropertyKey2)
{
  mitk::DataTreeIteratorClone _it=it;
  mitk::BoundingBox::Pointer m_BoundingBox=mitk::BoundingBox::New();
  mitk::BoundingBox::PointsContainer::Pointer pointscontainer=mitk::BoundingBox::PointsContainer::New();

  mitk::BoundingBox::PointIdentifier pointid=0;

  while (!_it->IsAtEnd())
  {
    mitk::DataTreeNode::Pointer node = _it->Get();
    assert(node.IsNotNull());
    if (node->GetData() != NULL && node->GetData()->GetUpdatedGeometry() != NULL && node->IsOn(boolPropertyKey, renderer) && node->IsOn(boolPropertyKey2, renderer)) 
    {
      const Geometry3D* geometry = node->GetData()->GetUpdatedGeometry();
      unsigned char i;
      for(i=0; i<8; ++i)
        pointscontainer->InsertElement( pointid++, geometry->GetCornerPoint(i));
    }
    ++_it;
  }

  mitk::BoundingBox::Pointer result = mitk::BoundingBox::New();
  result->SetPoints(pointscontainer);
  result->ComputeBoundingBox();

  return result;
}

mitk::TimeBounds mitk::DataTree::ComputeTimeBoundsInMS(mitk::DataTreeIteratorBase* it, const char* boolPropertyKey, mitk::BaseRenderer* renderer, const char* boolPropertyKey2)
{
  TimeBounds timebounds;

  mitk::DataTreeIteratorClone _it=it;

  mitk::ScalarType stmin, stmax, cur;

  stmin=-ScalarTypeNumericTraits::max();
  stmax= ScalarTypeNumericTraits::max();

  timebounds[0]=stmax; timebounds[1]=stmin;

  while (!_it->IsAtEnd())
  {
    mitk::DataTreeNode::Pointer node = _it->Get();
    if (node->GetData() != NULL && node->GetData()->GetUpdatedGeometry() != NULL && node->IsOn(boolPropertyKey, renderer) && node->IsOn(boolPropertyKey2, renderer))
    {
      mitk::Geometry3D::ConstPointer geometry = node->GetData()->GetGeometry();

      cur=geometry->GetTimeBoundsInMS()[0];
      //is it after -infinity, but before everything else that we found until now?
      if((cur>stmin) && (cur<timebounds[0]))
        timebounds[0] = cur;

      cur=geometry->GetTimeBoundsInMS()[1];
      //is it before infinity, but after everything else that we found until now?
      if((cur<stmax) && (cur>timebounds[1]))
        timebounds[1] = cur;
    }
    ++_it;
  }

  if(!(timebounds[0]<stmax))
  {
    timebounds[0] = stmin;
    timebounds[1] = stmax;
  }

  return timebounds;
}
