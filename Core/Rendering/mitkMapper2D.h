#ifndef MAPPER2D_H_HEADER_INCLUDED_C1C5453B
#define MAPPER2D_H_HEADER_INCLUDED_C1C5453B

#include "mitkCommon.h"
#include "Mapper.h"
#include "Geometry3D.h"
#include "DisplayGeometry.h"
#include "BaseRenderer.h"
#include "ImageChannelSelector.h"

namespace mitk {

//##ModelId=3E3AA308024B
//##Documentation
//## @brief Base class of all Mappers for 2D display
//## @ingroup Mapper
//## Base class of all Mappers for 2D display, i.e., a frontal view on a plane
//## display area, so nothing rotated in 3D space as, e.g., a plane in 3D space
//## (such things are done by subclasses of Mapper3D).
//## 
//## [not yet used:] SetGeometry3D() tells the Mapper2D which slices will
//## potentially be requested.
class Mapper2D : public Mapper
{
public:
    //##ModelId=3E3AE1B90028
    typedef Mapper2D Self;

	/** Standard class typedefs. */
    //##ModelId=3E3AE1B90046
    typedef Mapper Superclass;

    
    //##ModelId=3E3AE1B9005A
    typedef itk::SmartPointer<Self> Pointer;

    
    //##ModelId=3E3AE1B90078
    typedef itk::SmartPointer<const Self> ConstPointer;


    //##ModelId=3E3C45A0009E
     virtual void SetGeometry3D(const mitk::Geometry3D* aGeometry3D);

    //##ModelId=3E3C468102DC
  //    virtual void SetDisplayGeometry(const mitk::DisplayGeometry* aDisplayGeometry);

	/** Method for creation through the object factory. */
//	itkNewMacro(Self);

	/** Run-time type information (and related methods). */
	itkTypeMacro(Mapper2D,Mapper);

protected:
    //##ModelId=3E3C46810341
    Mapper2D();

    //##ModelId=3E3C4681035F
    virtual ~Mapper2D();

    //##ModelId=3E3C5C980267
    Geometry3D::ConstPointer m_Geometry3D;
    //##ModelId=3E33ECF20328
    int m_SliceNr;

    //##ModelId=3E3AE0CA0129
    int m_TimeNr;

    //##ModelId=3E3AE0CC00E6
    int m_ChannelNr;

};

} // namespace mitk



#endif /* MAPPER2D_H_HEADER_INCLUDED_C1C5453B */
