#ifndef SLICENAVIGATIONCONTROLLER_H_HEADER_INCLUDED_C1C55A2F
#define SLICENAVIGATIONCONTROLLER_H_HEADER_INCLUDED_C1C55A2F

#include "mitkCommon.h"
#include "BaseController.h"
#include "ImageToImageFilter.h"

namespace mitk {

//##ModelId=3DD523E00048
//##Documentation
//## Subclass of BaseController. Controls the selection of the slice the
//## associated BaseRenderer will display.
class SliceNavigationController : public BaseController
{
  public:
	/** Standard class typedefs. */
    //##ModelId=3E189B050197
	typedef SliceNavigationController      Self;
    //##ModelId=3E189B0501BF
	typedef BaseController                 Superclass;
    //##ModelId=3E189B0501E7
	typedef itk::SmartPointer<Self>        Pointer;
    //##ModelId=3E189B05020F
	typedef itk::SmartPointer<const Self>  ConstPointer;

	/** Method for creation through the object factory. */
	itkNewMacro(Self);

	/** Run-time type information (and related methods). */
	itkTypeMacro(SliceNavigationController,BaseController);
 
    //##ModelId=3DD524D7038C
    const mitk::Geometry3D* GetGeometry();

    //##ModelId=3DD524F9001A
    virtual void SetGeometry2D(const mitk::Geometry2D &aGeometry2D);

    //##ModelId=3E3AA1E20393
    virtual void SetGeometry3D(const mitk::Geometry3D &aGeometry3D);

    //##ModelId=3DFF3D9A038E
    virtual void SetPoint(const mitk::Point3D &point);

  protected:
    //##ModelId=3E189B1D008D
    SliceNavigationController();

    //##ModelId=3E189B1D00BF
    virtual ~SliceNavigationController();

};

} // namespace mitk



#endif /* SLICENAVIGATIONCONTROLLER_H_HEADER_INCLUDED_C1C55A2F */
