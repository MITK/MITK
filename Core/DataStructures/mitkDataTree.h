#ifndef DATATREE_H_HEADER_INCLUDED_C1C7797C

#define DATATREE_H_HEADER_INCLUDED_C1C7797C



#include "mitkCommon.h"



#include "DataTreeNode.h"



#include <Tree/LinkedTree.h>



namespace mitk {

//##ModelId=3EA93EC901B8

typedef ::LinkedTree<mitk::DataTreeNode::Pointer> DataTreeBase;
//##ModelId=3EA9438F0346
typedef DataTreeBase::TreeChangeListener DataTreeBaseTreeChangeListener;



//##ModelId=3E38F35101A0

class DataTree : public itk::Object, public DataTreeBase, public DataTreeBaseTreeChangeListener

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



    //##ModelId=3E3FE0430148

	TreeIterator<mitk::DataTreeNode::Pointer>* GetNext( const char* propertyName, const mitk::BaseProperty* property,  TreeIterator<mitk::DataTreeNode::Pointer>* startPosition = NULL );



 protected:

    //##ModelId=3E38F46A0190

    DataTree();



    //##ModelId=3E38F46A01AE

    virtual ~DataTree();



    //##ModelId=3EA6ADB7029F

	void treeChanged( TreeIterator<DataTreeNode::Pointer>& changedTreePosition );	



};

//##ModelId=3E394E99028E
typedef	::TreeIterator<DataTreeNode::Pointer> DataTreeIterator;

} // namespace mitk

#endif /* DATATREE_H_HEADER_INCLUDED_C1C7797C */



