#ifndef PLANEGEOMETRY_H_HEADER_INCLUDED_C1C68A2C
#define PLANEGEOMETRY_H_HEADER_INCLUDED_C1C68A2C

#include "mitkCommon.h"
#include "Geometry2D.h"
#include "SpaceGeometry.h"

namespace mitk {

//##ModelId=3E3963A80214
typedef View3f PlaneView;

//##ModelId=3DDE570F010A
class PlaneGeometry : public Geometry2D
{
public:
    /** Standard class typedefs. */
    //##ModelId=3E395F1702BE
    typedef PlaneGeometry           Self;
    //##ModelId=3E395F1702DC
    typedef Geometry2D              Superclass;
    //##ModelId=3E395F170304
    typedef itk::SmartPointer<Self> Pointer;
    //##ModelId=3E395F170322
    typedef itk::SmartPointer<const Self>  ConstPointer;

    /** Method for creation through the object factory. */
    itkNewMacro(Self);

    /** Run-time type information (and related methods). */
    itkTypeMacro(PlaneGeometry,Geometry2D);

    //##ModelId=3E395E3E0077
    virtual const mitk::PlaneView& GetView();

protected:
    //##ModelId=3E395F22035A
    PlaneGeometry();
    //##ModelId=3E395F220382
    virtual ~PlaneGeometry();

    //##ModelId=3E3963A90022
    mitk::PlaneView m_View;
};

} // namespace mitk



#endif /* PLANEGEOMETRY_H_HEADER_INCLUDED_C1C68A2C */
