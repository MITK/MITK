#ifndef GEOMETRY2D_H_HEADER_INCLUDED_C1F4D8E0
#define GEOMETRY2D_H_HEADER_INCLUDED_C1F4D8E0

#include "mitkCommon.h"

namespace mitk {

//##ModelId=3DDE4DEC0291
//##Documentation
//## Rule: everything is in mm (ms) if not stated otherwise.
class Geometry2D : public itk::LightObject
{
public:
	/** Standard class typedefs. */
    //##ModelId=3E1896DA03A6
	typedef Geometry2D         Self;
    //##ModelId=3E1896DA03E2
	typedef itk::LightObject   Superclass;
    //##ModelId=3E1896DB0036
	typedef itk::SmartPointer<Self> Pointer;

	/** Method for creation through the object factory. */
	itkNewMacro(Self);

	/** Run-time type information (and related methods). */
	itkTypeMacro(Geometry2D, itk::LightObject);

	//##ModelId=3DDE65E00122
	virtual void MapPoint(const Point3f &pt3d, Point2f &pt2d);

	//##ModelId=3DDE65E301DE
	virtual void MapPoint(const Point2f &pt2d, Point3f &pt3d);

	//##ModelId=3DE76C1D023D
	virtual void MapVector(const Vector2f &pt2d, Vector3f &pt3d);

	//##ModelId=3DE76C7C02F8
	virtual void MapVector(const Vector3f &pt3d, Vector2f &pt2d);

	//##ModelId=3DE7895602F7
	virtual void PointUnitsToMM(const Point2f &pt_units, Point2f &pt_mm);

	//##ModelId=3DE7895C01CE
	virtual void PointMMToUnits(const Point2f &pt_mm, Point2f &pt_units);

	//##ModelId=3DE78B870238
	virtual itk::Transform<float,3,2>::Pointer GetTransfrom();

	//##ModelId=3E02DFAF02AD
	virtual double GetTime();

};

} // namespace mitk



#endif /* GEOMETRY2D_H_HEADER_INCLUDED_C1F4D8E0 */
