#ifndef BASEDATA_H_HEADER_INCLUDED_C1EBB6FA
#define BASEDATA_H_HEADER_INCLUDED_C1EBB6FA

#include "mitkCommon.h"
#include "Geometry3D.h"

namespace mitk {

//##ModelId=3D6A0D7D00EC
class BaseData : public itk::DataObject
{
  public:
    //##ModelId=3E10262200CE
    typedef Geometry3D::Pointer Geometry3DPointer;

    //##ModelId=3DCBE2BA0139
    const mitk::Geometry3D& GetGeometry() const;

  protected:
    //##ModelId=3E15551A03CE
    Geometry3DPointer m_Geometry3D;
};

} // namespace mitk



#endif /* BASEDATA_H_HEADER_INCLUDED_C1EBB6FA */
