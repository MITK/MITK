/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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


#include "mitkDataTree.h"
#include <mitkXMLWriter.h>
#include <mitkXMLReader.h>
#include <mitkSmartPointerProperty.h>
#include <mitkProperties.h>

const std::string mitk::DataTree::XML_NODE_NAME = "mitkDataTree";
const std::string mitk::DataTree::XML_TAG_TREE_NODE = "treeNode";

//##ModelId=3E38F46A0190
mitk::DataTree::DataTree() : 
DataTreeBase( )
{
  DataTreeNode::Pointer emptyNode = DataTreeNode::New();
  emptyNode->SetProperty( "do not save", new BoolProperty(true) );
  SetRoot(emptyNode);
}


//##ModelId=3E38F46A01AE
mitk::DataTree::~DataTree()
{
  Clear();
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

  DataTreeNode::Pointer dtn;
  while ( !pos->IsAtEnd() )
  {
    dtn = pos->Get();
    PropertyList::Pointer propertyList = dtn->GetPropertyList();
    BaseProperty::Pointer tmp = propertyList->GetProperty( propertyKey );
    if ( (*property) == *(tmp) )
      return pos;
    ++pos;
  }
  return pos;
}

mitk::DataTreeIteratorClone mitk::DataTree::GetIteratorToNode(mitk::DataTreeBase* tree, const mitk::DataTreeNode* node, const mitk::DataTreeIteratorBase* startPosition )
{
  DataTreeIteratorClone pos;

  if(startPosition != NULL)
    pos = *startPosition;
  else
    pos = DataTreePreOrderIterator(tree);

  while ( !pos->IsAtEnd() )
  {
    if ( pos->Get().GetPointer() == node )
      return pos;
    ++pos;
  }
  return pos;

}

mitk::Geometry3D::Pointer mitk::DataTree::ComputeBoundingGeometry3D(mitk::DataTreeIteratorBase* it, const char* boolPropertyKey, mitk::BaseRenderer* renderer, const char* boolPropertyKey2)
{
  DataTreeIteratorClone _it=it;
  BoundingBox::PointsContainer::Pointer pointscontainer=BoundingBox::PointsContainer::New();

  BoundingBox::PointIdentifier pointid=0;
  Point3D point;

  Vector3D minSpacing;
  minSpacing.Fill(ScalarTypeNumericTraits::max());

  TimeBounds timeBounds;
  ScalarType stmin, stmax, cur;
  stmin= ScalarTypeNumericTraits::NonpositiveMin();
  stmax= ScalarTypeNumericTraits::max();

  timeBounds[0]=stmax; timeBounds[1]=stmin;

  while (!_it->IsAtEnd())
  {
    DataTreeNode::Pointer node = _it->Get();
    if((node.IsNotNull()) && (node->GetData() != NULL) && 
       (node->GetData()->IsEmpty()==false) && 
       node->IsOn(boolPropertyKey, renderer) && 
       node->IsOn(boolPropertyKey2, renderer)
      )
    {
      const Geometry3D* geometry = node->GetData()->GetUpdatedTimeSlicedGeometry();
      if (geometry != NULL ) 
      {
        // bounding box
        unsigned char i;
        for(i=0; i<8; ++i)
        {
          point = geometry->GetCornerPoint(i);
          if(point[0]*point[0]+point[1]*point[1]+point[2]*point[2] < large)
            pointscontainer->InsertElement( pointid++, point);
          else
          {
            itkGenericOutputMacro( << "Unrealistically distant corner point encountered. Ignored. Node: " << node );
          }
        }
        // spacing
        try
        {
          AffineTransform3D::Pointer inverseTransform = AffineTransform3D::New();
          geometry->GetIndexToWorldTransform()->GetInverse(inverseTransform);
          vnl_vector< AffineTransform3D::MatrixType::ValueType > unitVector(3);
          int axis;
          for(axis = 0; axis < 3; ++axis)
          {
            unitVector.fill(0);
            unitVector[axis] = 1.0;
            ScalarType mmPerPixel = 1.0/(inverseTransform->GetMatrix()*unitVector).magnitude();
            if(minSpacing[axis] > mmPerPixel)
            {
              minSpacing[axis] = mmPerPixel;
            }
          }
          // timebounds
          const TimeBounds & curTimeBounds = geometry->GetTimeBounds();
          cur=curTimeBounds[0];
          //is it after -infinity, but before everything else that we found until now?
          if((cur > stmin) && (cur < timeBounds[0]))
            timeBounds[0] = cur;

          cur=curTimeBounds[1];
          //is it before infinity, but after everything else that we found until now?
          if((cur < stmax) && (cur > timeBounds[1]))
            timeBounds[1] = cur;
        }
        catch(itk::ExceptionObject e)
        {
          std::cerr << e << std::endl;
        }
      }
    }
    ++_it;
  }

  BoundingBox::Pointer result = BoundingBox::New();
  result->SetPoints(pointscontainer);
  result->ComputeBoundingBox();

  Geometry3D::Pointer geometry;
  if ( result->GetPoints()->Size()>0 )
  {
    geometry = Geometry3D::New();
    geometry->Initialize();
    // correct bounding-box (is now in mm, should be in index-coordinates)
    // according to spacing
    BoundingBox::BoundsArrayType bounds = result->GetBounds();
    int i;
    for(i = 0; i < 6; ++i)
    {
      bounds[i] /= minSpacing[i/2];
    }
    geometry->SetBounds(bounds);
    geometry->SetSpacing(minSpacing);
    // timebounds
    if(!(timeBounds[0]<stmax))
    {
      timeBounds[0] = stmin;
      timeBounds[1] = stmax;
    }
    geometry->SetTimeBounds(timeBounds);
  }
  return geometry;
}

mitk::Geometry3D::Pointer mitk::DataTree::ComputeVisibleBoundingGeometry3D(mitk::DataTreeIteratorBase* it, mitk::BaseRenderer* renderer, const char* boolPropertyKey)
{
  return ComputeBoundingGeometry3D(it, "visible", renderer, boolPropertyKey);
}

mitk::BoundingBox::Pointer mitk::DataTree::ComputeBoundingBox(mitk::DataTreeIteratorBase* it, const char* boolPropertyKey, mitk::BaseRenderer* renderer, const char* boolPropertyKey2)
{
  if(it == NULL)
  {
    return NULL;
  }

  DataTreeIteratorClone _it=it;
  BoundingBox::PointsContainer::Pointer pointscontainer=BoundingBox::PointsContainer::New();

  BoundingBox::PointIdentifier pointid=0;
  Point3D point;

  while (!_it->IsAtEnd())
  {
    DataTreeNode::Pointer node = _it->Get();
    if((node.IsNotNull()) && (node->GetData() != NULL) && 
       (node->GetData()->IsEmpty()==false) && 
       node->IsOn(boolPropertyKey, renderer) && 
       node->IsOn(boolPropertyKey2, renderer)
      )
    {
      const Geometry3D* geometry = node->GetData()->GetUpdatedTimeSlicedGeometry();
      if (geometry != NULL ) 
      {
        unsigned char i;
        for(i=0; i<8; ++i)
        {
          point = geometry->GetCornerPoint(i);
          if(point[0]*point[0]+point[1]*point[1]+point[2]*point[2] < large)
            pointscontainer->InsertElement( pointid++, point);
          else
          {
            itkGenericOutputMacro( << "Unrealistically distant corner point encountered. Ignored. Node: " << node );
          }
        }
      }
    }
    ++_it;
  }

  BoundingBox::Pointer result = BoundingBox::New();
  result->SetPoints(pointscontainer);
  result->ComputeBoundingBox();

  return result;
}

mitk::TimeBounds mitk::DataTree::ComputeTimeBounds(mitk::DataTreeIteratorBase* it, const char* boolPropertyKey, mitk::BaseRenderer* renderer, const char* boolPropertyKey2)
{
  if(it == NULL)
  {
    return NULL;
  }

  TimeBounds timeBounds;

  DataTreeIteratorClone _it=it;

  ScalarType stmin, stmax, cur;

  stmin= ScalarTypeNumericTraits::NonpositiveMin();
  stmax= ScalarTypeNumericTraits::max();

  timeBounds[0]=stmax; timeBounds[1]=stmin;

  while (!_it->IsAtEnd())
  {
    DataTreeNode::Pointer node = _it->Get();
    if((node.IsNotNull()) && (node->GetData() != NULL) && 
       (node->GetData()->IsEmpty()==false) && 
       node->IsOn(boolPropertyKey, renderer) && 
       node->IsOn(boolPropertyKey2, renderer)
      )
    {
      const Geometry3D* geometry = node->GetData()->GetUpdatedTimeSlicedGeometry();
      if (geometry != NULL ) 
      {
        const TimeBounds & curTimeBounds = geometry->GetTimeBounds();
        cur=curTimeBounds[0];
        //is it after -infinity, but before everything else that we found until now?
        if((cur > stmin) && (cur < timeBounds[0]))
          timeBounds[0] = cur;

        cur=curTimeBounds[1];
        //is it before infinity, but after everything else that we found until now?
        if((cur < stmax) && (cur > timeBounds[1]))
          timeBounds[1] = cur;
      }
    }
    ++_it;
  }

  if(!(timeBounds[0]<stmax))
  {
    timeBounds[0] = stmin;
    timeBounds[1] = stmax;
  }

  return timeBounds;
}

bool mitk::DataTree::Load( const mitk::DataTreeIteratorBase* it, const char* filename )
{
  PropertyList::PrepareXML_IO(); // to get "shared" properties right  
  bool result = XMLReader::Load( filename, it );
  SmartPointerProperty::PostProcessXMLReading(); // to get "shared" properties right  

  return result;
}

bool mitk::DataTree::Save( const mitk::DataTreeIteratorBase* it, const char* fileName, bool writeRootNodeToo ) 
{
  if ( fileName == NULL || it == NULL || it->IsAtEnd() )
    return false;
  
  XMLWriter writer( fileName );

  if ( !Save( it, writer, writeRootNodeToo ) )
    writer.WriteComment( "Error" );

  return true;
}

bool mitk::DataTree::SaveNext( const mitk::DataTreeIteratorBase* it, mitk::XMLWriter& xmlWriter, bool writeRootNodeToo ) 
{
  if ( it == NULL || it->IsAtEnd() )
    return false;

  DataTreeNode* node = it->Get();
  xmlWriter.BeginNode(XML_TAG_TREE_NODE);

  if (node) 
  {

    if (writeRootNodeToo)
    {
      StringProperty* sprop = dynamic_cast<StringProperty*>( node->GetProperty("FILENAME").GetPointer() );
      if (sprop) // if this property is set, then use it as a filename
      {
        xmlWriter.SetSourceFileName( sprop->GetValue() );      
      }

      node->WriteXML( xmlWriter );
    }

    if ( it->HasChild() ) 
    {
      DataTreeChildIterator children(*it);

      while (!children.IsAtEnd())
      {
        SaveNext( &children, xmlWriter, true );
        ++children;
      }
    }
  }

  xmlWriter.EndNode(); // TreeNode

  return true;
}

bool mitk::DataTree::Save( const mitk::DataTreeIteratorBase* it, mitk::XMLWriter& xmlWriter, bool writeRootNodeToo ) 
{
  PropertyList::PrepareXML_IO(); // to get "shared" properties right  
  xmlWriter.BeginNode(XML_NODE_NAME);
  bool result = SaveNext( it, xmlWriter, writeRootNodeToo );
  xmlWriter.EndNode(); // mitkDataTree
  return result;
}
