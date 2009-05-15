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


#ifndef DATATREE_H_HEADER_INCLUDED_C1C7797C
#define DATATREE_H_HEADER_INCLUDED_C1C7797C

#include "mitkCommon.h"
#include "mitkDataTreeNode.h"
#include "mitkGeometry3D.h"

#include <itkTreeContainer.h>
#include <itkTreeIteratorClone.h>
#include <itkPreOrderTreeIterator.h>
#include <itkChildTreeIterator.h>
#include <itkEventObject.h>

namespace mitk 
{

class XMLWriter;


typedef itk::TreeContainer<mitk::DataTreeNode::Pointer> DataTreeBase;

//##Documentation
//## @brief typedef to a base-iterator on the data tree
//## @ingroup DataManagement
typedef  itk::TreeIteratorBase<DataTreeBase> DataTreeIteratorBase;

//##Documentation
//## @brief typedef to an iterator on the data tree
//## @ingroup DataManagement
typedef  itk::TreeIteratorClone<DataTreeIteratorBase> DataTreeIteratorClone;

//##Documentation
//## @brief typedef to a preorder-iterator on the data tree
//## @ingroup DataManagement
typedef  itk::PreOrderTreeIterator<DataTreeBase> DataTreePreOrderIterator;

//##Documentation
//## @brief typedef to a children-iterator on the data tree
//## @ingroup DataManagement
typedef  itk::ChildTreeIterator<DataTreeBase> DataTreeChildIterator;

typedef  itk::TreeChangeEvent<DataTreeBase> DataTreeChangeEvent;

//##Documentation
//## @brief Main run-time data management class defining a data tree
//## @ingroup DataManagement
class MITK_CORE_EXPORT DataTree : public DataTreeBase
{

public:
  mitkClassMacro(DataTree, DataTreeBase);    

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  DataTreeIteratorClone GetNext( const char* propertyName, const mitk::BaseProperty* property,  DataTreeIteratorBase* startPosition = NULL );

  DataTreeIteratorClone GetIteratorToNode(const DataTreeNode* node, const DataTreeIteratorBase* startPosition = NULL )
  {
    return GetIteratorToNode(this, node, startPosition);
  }

  static DataTreeIteratorClone GetIteratorToNode(mitk::DataTreeBase* tree, const DataTreeNode* node, const DataTreeIteratorBase* startPosition = NULL );

  //##Documentation
  //## @brief Compute the axis-parallel bounding geometry of the data tree
  //## (bounding box, minimal spacing of the considered nodes, live-span)
  //##
  //## @param it an iterator to a data tree structure
  //## @param boolPropertyKey if a BoolProperty with this boolPropertyKey exists for a node (for @a renderer)
  //## and is set to @a false, the node is ignored for the bounding-box calculation.
  //## @param renderer see @a boolPropertyKey
  //## @param boolPropertyKey2 a second condition that is applied additionally to @a boolPropertyKey
  static Geometry3D::Pointer ComputeBoundingGeometry3D(mitk::DataTreeIteratorBase* it, const char* boolPropertyKey = NULL, mitk::BaseRenderer* renderer = NULL, const char* boolPropertyKey2 = NULL);

  //##Documentation
  //## @brief Compute the axis-parallel bounding geometry of all visible parts of the
  //## data tree bounding box, minimal spacing of the considered nodes, live-span)
  //##
  //## Simply calls ComputeBoundingGeometry3D(it, "visible", renderer, boolPropertyKey).
  //## @param it an iterator of a data tree structure
  //## @param renderer the reference to the renderer
  //## @param boolPropertyKey if a BoolProperty with this boolPropertyKey exists for a node (for @a renderer)
  //## and is set to @a false, the node is ignored for the bounding-box calculation.
  static mitk::Geometry3D::Pointer ComputeVisibleBoundingGeometry3D(mitk::DataTreeIteratorBase* it, mitk::BaseRenderer* renderer = NULL, const char* boolPropertyKey = NULL);

  //##Documentation
  //## @brief Compute the bounding box of data tree structure
  //## @param it an iterator to a data tree structure
  //## @param boolPropertyKey if a BoolProperty with this boolPropertyKey exists for a node (for @a renderer) 
  //## and is set to @a false, the node is ignored for the bounding-box calculation.
  //## @param renderer see @a boolPropertyKey
  //## @param boolPropertyKey2 a second condition that is applied additionally to @a boolPropertyKey
  static mitk::BoundingBox::Pointer ComputeBoundingBox(mitk::DataTreeIteratorBase* it, const char* boolPropertyKey = NULL, mitk::BaseRenderer* renderer = NULL, const char* boolPropertyKey2 = NULL);

  //##Documentation
  //## \brief Compute the bounding box of all visible parts of the data tree structure, for general 
  //## rendering or renderer specific visibility property checking
  //## 
  //## Simply calls ComputeBoundingBox(it, "visible", renderer, boolPropertyKey).
  //## @param it an iterator of a data tree structure
  //## @param renderer the reference to the renderer
  //## @param boolPropertyKey if a BoolProperty with this boolPropertyKey exists for a node (for @a renderer) 
  //## and is set to @a false, the node is ignored for the bounding-box calculation.
  static mitk::BoundingBox::Pointer ComputeVisibleBoundingBox(mitk::DataTreeIteratorBase* it, mitk::BaseRenderer* renderer = NULL, const char* boolPropertyKey = NULL)
  {
    return ComputeBoundingBox(it, "visible", renderer, boolPropertyKey);
  }

  //##Documentation
  //## @brief Compute the time-bounds of the contents of a data tree structure
  //##
  //## The methods returns only [-infinity, +infinity], if all data-objects have an infinite live-span. Otherwise,
  //## all data-objects with infinite live-span are ignored.
  //## @param it an iterator to a data tree structure
  //## @param boolPropertyKey if a BoolProperty with this boolPropertyKey exists for a node (for @a renderer) 
  //## and is set to @a false, the node is ignored for the time-bounds calculation.
  //## @param renderer see @a boolPropertyKey
  //## @param boolPropertyKey2 a second condition that is applied additionally to @a boolPropertyKey
  static TimeBounds ComputeTimeBounds(mitk::DataTreeIteratorBase* it, const char* boolPropertyKey, mitk::BaseRenderer* renderer, const char* boolPropertyKey2);

  //##Documentation
  //## @brief Compute the time-bounds of all visible parts of the data tree structure, for general 
  //## rendering or renderer specific visibility property checking
  //##
  //## The methods returns only [-infinity, +infinity], if all data-objects have an infinite live-span. Otherwise,
  //## all data-objects with infinite live-span are ignored.
  //## Simply calls ComputeTimeBounds(it, "visible", renderer, boolPropertyKey).
  //## @param it an iterator to a data tree structure
  //## @param boolPropertyKey if a BoolProperty with this boolPropertyKey exists for a node (for @a renderer) 
  //## and is set to @a false, the node is ignored for the time-bounds calculation.
  //## @param renderer see @a boolPropertyKey
  static TimeBounds ComputeTimeBounds(mitk::DataTreeIteratorBase* it, mitk::BaseRenderer* renderer, const char* boolPropertyKey)
  {
    return ComputeTimeBounds(it, "visible", renderer, boolPropertyKey);
  }

  //## 
  static bool Save( const mitk::DataTreeIteratorBase* it, const char* fileName, bool writeRootNodeToo = true );
  static bool Save( const mitk::DataTreeIteratorBase* it, mitk::XMLWriter& xmlWriter, bool writeRootNodeToo = true );
  static bool Load( const mitk::DataTreeIteratorBase* it, const char* filename );

  static const std::string   XML_NODE_NAME;
  static const std::string   XML_TAG_TREE_NODE;

protected:
  DataTree();

  virtual ~DataTree();

  static bool SaveNext( const mitk::DataTreeIteratorBase* it, mitk::XMLWriter& xmlWriter, bool writeRootNodeToo = true );  
};

} // namespace mitk
#endif /* DATATREE_H_HEADER_INCLUDED_C1C7797C */


