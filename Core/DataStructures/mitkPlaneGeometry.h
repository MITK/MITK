#ifndef PLANEGEOMETRY_H_HEADER_INCLUDED_C1C68A2C
#define PLANEGEOMETRY_H_HEADER_INCLUDED_C1C68A2C

#include "mitkCommon.h"
#include "Geometry2D.h"
#include "SpaceGeometry.h"

class vtkTransform;

namespace mitk {


//##ModelId=3DDE570F010A
//##Documentation
//## @brief Describes a two-dimensional, plane and rectangular surface
//## @ingroup Geometry
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
//    itkGetConstReferenceMacro(PlaneView, PlaneView);
    //##ModelId=3E395E3E0077
    virtual const mitk::PlaneView& GetPlaneView() const;
    //##ModelId=3E396ABE0385
    virtual void SetPlaneView(const mitk::PlaneView& aPlaneView);

    //##ModelId=3ED91D060363
	virtual void TransformGeometry(const vtkTransform * transform);

    /** Method for creation through the object factory. */
    itkNewMacro(Self);

    //##ModelId=3E3AEB7C001C
    virtual itk::Transform<float,3,2>::Pointer GetTransfrom() const;
    //##ModelId=3E3B9C6E02B5
    virtual void Map(const mitk::Point3D &pt3d_mm, mitk::Point2D &pt2d_mm) const;

    //##ModelId=3E3B9C7101BF
    virtual void Map(const mitk::Point2D &pt2d_mm, mitk::Point3D &pt3d_mm) const;

    //##ModelId=3E3B9C730262
    virtual void UnitsToMM(const mitk::Point2D &pt_units, mitk::Point2D &pt_mm) const;

    //##ModelId=3E3B9C760112
    virtual void MMToUnits(const mitk::Point2D &pt_mm, mitk::Point2D &pt_units) const;
    
    //##ModelId=3E3B9C8C0145
    virtual void UnitsToMM(const mitk::Vector2D &vec_units, mitk::Vector2D &vec_mm) const;
    
    //##ModelId=3E3B9C8E0152
    virtual void MMToUnits(const mitk::Vector2D &vec_mm, mitk::Vector2D &vec_units) const;

    /** Run-time type information (and related methods). */
    itkTypeMacro(PlaneGeometry,Geometry2D);

protected:
    //##ModelId=3E395F22035A
    PlaneGeometry();
    //##ModelId=3E395F220382
    virtual ~PlaneGeometry();

    //##ModelId=3E3A3F03034E
    mitk::PlaneView m_PlaneView;

    //##ModelId=3E3BE12C012B
    float m_ScaleFactorMMPerUnitX;
    //##ModelId=3E3BE12C0167
    float m_ScaleFactorMMPerUnitY;
};

} // namespace mitk
#endif /* PLANEGEOMETRY_H_HEADER_INCLUDED_C1C68A2C */
