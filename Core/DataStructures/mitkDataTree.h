#ifndef DATATREE_H_HEADER_INCLUDED_C1C7797C
#define DATATREE_H_HEADER_INCLUDED_C1C7797C

#include "mitkCommon.h"

#include "DataTreeNode.h"

#include <Tree/LinkedTree.h>

namespace mitk {
typedef ::LinkedTree<DataTreeNode::Pointer> DataTreeBase;

//##ModelId=3E38F35101A0
class DataTree : public itk::Object, public DataTreeBase 
{
   public:
	/** Standard class typedefs. */
    //##ModelId=3E38FFC80202
	typedef DataTree                 Self;
    //##ModelId=3E38FFC80220
	typedef itk::Object              Superclass;
    //##ModelId=3E38FFC8023E
	typedef itk::SmartPointer<Self>  Pointer;
    //##ModelId=3E38FFC80266
	typedef itk::SmartPointer<const Self>  ConstPointer;

	/** Method for creation through the object factory. */
	itkNewMacro(Self);

	/** Run-time type information (and related methods). */
	itkTypeMacro(DataTree,itk::Object);

	// TreeIterator<DataTreeNode::Pointer>* getNext( const char* propertyName, const itk::BaseProperty* property,  TreeIterator<DataTreeNode::Pointer>* startPosition ){ return NULL;};
	// TreeIterator<DataTreeNode::Pointer>* getNext( const char* propertyName, const itk::BaseProperty* property ){ return NULL;};
 protected:
    //##ModelId=3E38F46A0190
    DataTree();

    //##ModelId=3E38F46A01AE
    virtual ~DataTree();

};

typedef	::TreeIterator<DataTreeNode::Pointer> DataTreeIterator;

} // namespace mitk



#endif /* DATATREE_H_HEADER_INCLUDED_C1C7797C */
