#include "DataTree.h"

//##ModelId=3E38F46A0190
mitk::DataTree::DataTree() : 
    DataTreeBase( )
{
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
		mitk::PropertyList::Pointer propertyList = dtn->GetData()->GetPropertyList();

		if ( property == propertyList->GetProperty( propertyKey ) )
			return pos;			
	}
	
	delete pos;
	return NULL;
}