#ifdef _MSC_VER
// This warns about truncation to 255 characters in debug/browse info
#pragma warning (disable : 4786)
#endif

//add only those headers here that are really necessary for all classes!
#include "itkObject.h"

//#include "itkRegion.h"
//#include "vecmath.h"
//#include "itkTransform.h"
//#include "itkProcessObject.h"
//#include "ipPic.h"
//#include "vtkImageData.h"
//#include "vtkImageReslice.h"
//
typedef  int MapperSlotId;
//typedef	Tree<mitk::DataTreeNode::Pointer> DataTree;
//typedef	TreeIterator<mitk::DataTreeNode::Pointer> DataTreeIterator;


#define mitkClassMacro(className,SuperClassName) \
	typedef className				Self; \
	typedef SuperClassName			Superclass; \
	typedef itk::SmartPointer<Self> Pointer; \
	typedef itk::SmartPointer<const Self>  ConstPointer; \
    itkTypeMacro(className,SuperClassName)


