#ifndef MAPPER2D_H_HEADER_INCLUDED_C1C5453B
#define MAPPER2D_H_HEADER_INCLUDED_C1C5453B

#include "mitkCommon.h"
#include "Mapper.h"
#include "Geometry3D.h"
#include "DisplayGeometry.h"

namespace mitk {

//##ModelId=3E3AA308024B
class Mapper2D : public Mapper
{
public:
	/** Standard class typedefs. */
    //##ModelId=3E3AE1B90028
	typedef Mapper2D                 Self;
    //##ModelId=3E3AE1B90046
	typedef Mapper                   Superclass;
    //##ModelId=3E3AE1B9005A
	typedef itk::SmartPointer<Self>  Pointer;
    //##ModelId=3E3AE1B90078
	typedef itk::SmartPointer<const Self>  ConstPointer;

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
