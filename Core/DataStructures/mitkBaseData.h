#ifndef BASEDATA_H_HEADER_INCLUDED_C1EBB6FA
#define BASEDATA_H_HEADER_INCLUDED_C1EBB6FA

#include "mitkCommon.h"
#include "Geometry3D.h"
#include "Geometry2D.h"
#include "PropertyList.h"

namespace mitk {

//##ModelId=3D6A0D7D00EC
//##Documentation
//## Base of all data objects, e.g., images, contours, surfaces etc. Inherits
//## from itk::DataObject and thus can be included in a pipeline.
class BaseData : public itk::DataObject
{	
  public:
    //##ModelId=3E10262200CE
    typedef Geometry3D::Pointer Geometry3DPointer;

	mitkClassMacro(mitk::BaseData,itk::DataObject)

	//itkNewMacro(Self);  

    //##ModelId=3DCBE2BA0139
    const mitk::Geometry3D& GetGeometry() const;
    //##ModelId=3E3C4ACB0046
    //##Documentation
    //## Return the Geometry2D of the slice (@a s, @a t). The method does not
    //## simply call GetGeometry()->GetGeometry2D(). Before doing this, it
    //## makes sure that the Geometry2D is up-to-date before returning it (by
    //## setting the update extent appropriately and calling
    //## UpdateOutputInformation).
    //## 
    //## @warning GetGeometry2D not yet completely implemented. Appropriate
    //## setting of the update extent is missing.
    virtual mitk::Geometry2D::ConstPointer GetGeometry2D(int s, int t);

	mitk::PropertyList::Pointer GetPropertyList();

  protected:
	BaseData();
	~BaseData();
    Geometry3DPointer m_Geometry3D;
	PropertyList::Pointer m_PropertyList;
};

} // namespace mitk



#endif /* BASEDATA_H_HEADER_INCLUDED_C1EBB6FA */
