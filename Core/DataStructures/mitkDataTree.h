#ifndef DATATREE_H_HEADER_INCLUDED_C1C7797C
#define DATATREE_H_HEADER_INCLUDED_C1C7797C

#include "mitkCommon.h"
#include "mitkDataTreeNode.h"

//#include "DataTreeIterator.h"
#include "mitkGeometry3D.h"
#include <Tree/LinkedTree.h>

namespace mitk {
//##ModelId=3EA93EC901B8
typedef ::LinkedTree<mitk::DataTreeNode::Pointer> DataTreeBase;
//##ModelId=3EA9438F0346
typedef DataTreeBase::TreeChangeListener DataTreeBaseTreeChangeListener;

//##ModelId=3E394E99028E
//##Documentation
//## @brief typedef to an iterator on the data tree
//## @ingroup DataTree
typedef	::TreeIterator<DataTreeNode::Pointer> DataTreeIterator;


//##ModelId=3E38F35101A0
//##Documentation
//## @brief Main run-time data management class defining a data tree
//## @ingroup DataTree
class DataTree : public itk::Object, public DataTreeBase, public DataTreeBaseTreeChangeListener
{

public:
  mitkClassMacro(DataTree, itk::Object);    

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  //##ModelId=3E3FE0430148
  TreeIterator<mitk::DataTreeNode::Pointer>* GetNext( const char* propertyName, const mitk::BaseProperty* property,  TreeIterator<mitk::DataTreeNode::Pointer>* startPosition = NULL );

  //##ModelId=3ED91D050085
  //##Documentation
  //## @brief Compute the bounding box of data tree structure
  //## @param it an iterator to a data tree structure
  //## @param boolPropertyKey if a BoolProperty with this boolPropertyKey exists for a node (for @a renderer) 
  //## and is set to @a false, the node is ignored for the bounding-box calculation.
  //## @param renderer see @a boolPropertyKey
  //## @param boolPropertyKey2 a second condition that is applied additionally to @a boolPropertyKey
  static mitk::BoundingBox::Pointer ComputeBoundingBox(mitk::DataTreeIterator * it, const char* boolPropertyKey = NULL, mitk::BaseRenderer* renderer = NULL, const char* boolPropertyKey2 = NULL);

  //##ModelId=3ED91D050085
  //##Documentation
  //## \brief Compute the bounding box of all visible parts of the data tree structure, for general 
  //## rendering or renderer specific visibility property checking
  //## 
  //## Simply calls ComputeBoundingBox(it, "visible", renderer, boolPropertyKey).
  //## @param it an iterator of a data tree structure
  //## @param renderer the reference to the renderer
  //## @param boolPropertyKey if a BoolProperty with this boolPropertyKey exists for a node (for @a renderer) 
  //## and is set to @a false, the node is ignored for the bounding-box calculation.
  static mitk::BoundingBox::Pointer ComputeVisibleBoundingBox(mitk::DataTreeIterator * it, mitk::BaseRenderer* renderer = NULL, const char* boolPropertyKey = NULL)
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
  static TimeBounds mitk::DataTree::ComputeTimeBoundsInMS(mitk::DataTreeIterator * it, const char* boolPropertyKey, mitk::BaseRenderer* renderer, const char* boolPropertyKey2);

  //##Documentation
  //## @brief Compute the time-bounds of all visible parts of the data tree structure, for general 
  //## rendering or renderer specific visibility property checking
  //##
  //## The methods returns only [-infinity, +infinity], if all data-objects have an infinite live-span. Otherwise,
  //## all data-objects with infinite live-span are ignored.
  //## Simply calls ComputeTimeBoundsInMS(it, "visible", renderer, boolPropertyKey).
  //## @param it an iterator to a data tree structure
  //## @param boolPropertyKey if a BoolProperty with this boolPropertyKey exists for a node (for @a renderer) 
  //## and is set to @a false, the node is ignored for the time-bounds calculation.
  //## @param renderer see @a boolPropertyKey
  static TimeBounds mitk::DataTree::ComputeTimeBoundsInMS(mitk::DataTreeIterator * it, mitk::BaseRenderer* renderer, const char* boolPropertyKey)
  {
    return ComputeTimeBoundsInMS(it, "visible", renderer, boolPropertyKey);
  }
protected:
  //##ModelId=3E38F46A0190
  DataTree();

  //##ModelId=3E38F46A01AE
  virtual ~DataTree();

  //##ModelId=3EA6ADB7029F
  void treeChanged( TreeIterator<DataTreeNode::Pointer>& changedTreePosition );	

};

} // namespace mitk
#endif /* DATATREE_H_HEADER_INCLUDED_C1C7797C */

