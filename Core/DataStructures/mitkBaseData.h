#ifndef BASEDATA_H_HEADER_INCLUDED_C1EBB6FA
#define BASEDATA_H_HEADER_INCLUDED_C1EBB6FA

#include <itkDataObject.h>

#include "mitkCommon.h"
#include "Geometry3D.h"
#include "Geometry2D.h"
#include "PropertyList.h"

namespace mitk {

class BaseProcess;

//##ModelId=3D6A0D7D00EC
//##Documentation
//## @brief Base of all data objects
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
    //##Documentation
    //## Return the Geometry3D of the data. The method does not
    //## simply return the value of the m_Geometry3D member. Before doing this, it
    //## makes sure that the Geometry3D is up-to-date before returning it (by
    //## setting the update extent appropriately and calling
    //## UpdateOutputInformation).
    //## 
    //## @warning GetGeometry not yet completely implemented. Appropriate
    //## setting of the update extent is missing.
        mitk::Geometry3D::ConstPointer GetGeometry() const;
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
    virtual mitk::Geometry2D::ConstPointer GetGeometry2D(int s, int t) const;

    //##ModelId=3E8600DB0188
    virtual void UnRegister() const;

    //##ModelId=3E8600DB02DC
    virtual int GetExternalReferenceCount() const;
protected:
    //##ModelId=3E3FE04202B9
    BaseData();
    //##ModelId=3E3FE042031D
    ~BaseData();
    //##ModelId=3E15551A03CE
    Geometry3DPointer m_Geometry3D;

    //##ModelId=3E8600D9006D
    mutable itk::SmartPointer<mitk::BaseProcess> m_SmartSourcePointer;
    //##ModelId=3E8600D9021B
    mutable unsigned int m_SourceOutputIndexDuplicate;
    //##ModelId=3E8600DC0053
    virtual void ConnectSource(itk::ProcessObject *arg, unsigned int idx) const;
private:
    //##ModelId=3E8600D90384
    mutable bool m_Unregistering;
    //##ModelId=3E8600DA0118
    mutable bool m_CalculatingExternalReferenceCount;
    //##ModelId=3E8600DA02B3
    mutable int m_ExternalReferenceCount;

    friend class mitk::BaseProcess;
};

} // namespace mitk


#endif /* BASEDATA_H_HEADER_INCLUDED_C1EBB6FA */
