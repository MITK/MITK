#ifndef GEOMETRY2D_H_HEADER_INCLUDED_C1F4D8E0
#define GEOMETRY2D_H_HEADER_INCLUDED_C1F4D8E0

#include "mitkCommon.h"
#include "mitkVector.h"
#include "itkTransform.h"

class vtkTransform;

namespace mitk {

//##ModelId=3DDE4DEC0291
//##Documentation
//## @brief Describes the geometry of a two-dimensional object
//## @ingroup Geometry
//## Describes the geometry of a two-dimensional object. Most often, this will
//## be a plane, which is represented by the sub-class PlaneGeometry. But
//## curved surfaces are also possible. Can transform between world coordinates
//## (in mm) and unit coordinates (parameter representation of the
//## two-dimensionale object with parameter values inside a rectangle).
//## Rule: everything is in mm (ms) if not stated otherwise.
class Geometry2D : public itk::Object
{
public:
	/** Standard class typedefs. */
    //##ModelId=3E1896DA03A6
	typedef Geometry2D         Self;
    //##ModelId=3E1896DA03E2
	typedef itk::Object        Superclass;
    //##ModelId=3E1896DB0036
	typedef itk::SmartPointer<Self> Pointer;
    //##ModelId=3E3B00C9012B
    typedef itk::SmartPointer<const Self> ConstPointer;

	/** Method for creation through the object factory. */
	//itkNewMacro(Self);

	/** Run-time type information (and related methods). */
	itkTypeMacro(Geometry2D, itk::Object);

    itkGetConstMacro(WidthInUnits, unsigned int);
    itkSetMacro(WidthInUnits, unsigned int);

    itkGetConstMacro(HeightInUnits, unsigned int);
    itkSetMacro(HeightInUnits, unsigned int);

	//##ModelId=3DDE65E00122
	virtual void Map(const mitk::Point3D &pt3d_mm, mitk::Point2D &pt2d_mm) const;

	//##ModelId=3DDE65E301DE
	virtual void Map(const mitk::Point2D &pt2d_mm, mitk::Point3D &pt3d_mm) const;

	//##ModelId=3DE7895602F7
	virtual void UnitsToMM(const mitk::Point2D &pt_units, mitk::Point2D &pt_mm) const;

	//##ModelId=3DE7895C01CE
	virtual void MMToUnits(const mitk::Point2D &pt_mm, mitk::Point2D &pt_units) const;

	//##ModelId=3DE78B870238
	virtual itk::Transform<float,3,2>::Pointer GetTransfrom() const;

	//##ModelId=3E02DFAF02AD
    //##Documentation
    //## Get the time in ms this Geometry2D is describing.
	virtual double GetTime() const;

    //##ModelId=3E3B98C5019F
    virtual void UnitsToMM(const mitk::Vector2D &vec_units, mitk::Vector2D &vec_mm) const;
    //##ModelId=3E3B98C9019B
    virtual void MMToUnits(const mitk::Vector2D &vec_mm, mitk::Vector2D &vec_units) const;
    //##ModelId=3E3C3A92033E
    virtual void SetSizeInUnits(unsigned int width, unsigned int height);

    //##ModelId=3ED91D05019E
	virtual void TransformGeometry(const vtkTransform * transform);

    //##ModelId=3E395E0802E6
    Geometry2D(unsigned int width_units, unsigned int height_units);

  protected:
    //##ModelId=3E395E080318
    virtual ~Geometry2D();

    //##ModelId=3E3C429101C5
    unsigned int m_WidthInUnits;
    //##ModelId=3E3C4291020B
    unsigned int m_HeightInUnits;
};

} // namespace mitk

#endif /* GEOMETRY2D_H_HEADER_INCLUDED_C1F4D8E0 */
