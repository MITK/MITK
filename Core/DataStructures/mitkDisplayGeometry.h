#ifndef DISPLAYGEOMETRY_H_HEADER_INCLUDED_C1B77F88
#define DISPLAYGEOMETRY_H_HEADER_INCLUDED_C1B77F88

#include "mitkCommon.h"
#include "Geometry2D.h"
#include "ImageSliceSelector.h"

namespace mitk {

//##ModelId=3E3AE8F401DD
//##Documentation
//## @brief Describes the geometry on the Display (inside a widget)
//## @ingroup Geometry
//## Describes the geometry on the Display (inside a widget). Can convert
//## between Display units and data units (as stored in Geometry2D). Can depend
//## on a Geometry2D describing the data. If so, it can directly convert from
//## Display units to mm (via PointUnitsToMM) and vice versa (via
//## PointMMToUnits). Also provides convenience methods for zooming and
//## panning.
class DisplayGeometry : public Geometry2D
{
public:
	mitkClassMacro(DisplayGeometry,Geometry2D);

	/** Method for creation through the object factory. */
	itkNewMacro(Self);

    //##ModelId=3E3AEA5101A2
    virtual void SetWorldGeometry(const mitk::Geometry2D* aWorldGeometry);
  	itkGetConstObjectMacro(WorldGeometry, Geometry2D);

    //##ModelId=3E3AE9FD00C5
    virtual void Zoom(float factor, const mitk::Vector2D& centerInDisplayUnits);
    //##ModelId=3E3AEE700380
    virtual void MoveBy(const mitk::Vector2D& shiftInDisplayUnits);
    //##ModelId=3E3C38660226
    virtual void Fit();

    //##ModelId=3E3AEB8F0286
    virtual void DisplayToUnits(const mitk::Point2D &pt_display, mitk::Point2D &pt_units) const;
    //##ModelId=3E3AEBEF039C
    virtual void UnitsToDisplay(const mitk::Point2D &pt_units, mitk::Point2D &pt_display) const;

    //##ModelId=3E3C36920345
    virtual void SetSizeInDisplayUnits(unsigned int width, unsigned int height);
    //##ModelId=3E3C36CD02D2
    virtual void SetOriginInUnits(const mitk::Point2D& origin_units);
    //##ModelId=3E3C516B0045
    mitk::Point2D GetOriginInUnits() const;
    //##ModelId=3E3C51890107
    mitk::Point2D GetOriginInDisplayUnits() const;

    //##ModelId=3E3AEB460259
    virtual void Map(const mitk::Point3D &pt3d_mm, mitk::Point2D &pt2d_mm) const;
    //##ModelId=3E3AEB490380
    virtual void Map(const mitk::Point2D &pt2d_mm, mitk::Point3D &pt3d_mm) const;

    //##ModelId=3E3AEB52008A
    virtual void UnitsToMM(const mitk::Point2D &pt_units, mitk::Point2D &pt_mm) const;
    //##ModelId=3E3AEB54017D
    virtual void MMToUnits(const mitk::Point2D &pt_mm, mitk::Point2D &pt_units) const;

    //##ModelId=3E3AEB56037E
    virtual itk::Transform<float,3,2>::Pointer GetTransfrom() const;

    //##ModelId=3E3AEB620231
    virtual double GetTime() const;

    //##ModelId=3E3B02710239
    virtual void DisplayToUnits(const mitk::Vector2D &vec_display, mitk::Vector2D &vec_units) const;
    //##ModelId=3E3B02760128
    virtual void UnitsToDisplay(const mitk::Vector2D &vec_units, mitk::Vector2D &vec_display) const;

    //##ModelId=3E3B994B02A6
    virtual void MMToUnits(const mitk::Vector2D &vec_mm, mitk::Vector2D &vec_units) const;
    //##ModelId=3E3B994802B6
    virtual void UnitsToMM(const mitk::Vector2D &vec_units, mitk::Vector2D &vec_mm) const;

    //##ModelId=3E3C666E0299
    unsigned int GetDisplayWidth() const;
    //##ModelId=3E3C668B0363
    unsigned int GetDisplayHeight() const;

    //##ModelId=3E3D86CB0039
    mitk::Vector2D GetSizeInDisplayUnits() const;
    //##ModelId=3E3D86E9015E
    mitk::Vector2D GetSizeInUnits() const;
    //##ModelId=3E3D883B0132
    virtual void SetScaleFactor(float unitsPerDisplayUnit);
    //##ModelId=3E3ED45900A1
    float GetScaleFactorUnitsPerDisplayUnit() const;
    //##ModelId=3E48D5B40210
    virtual void DisplayToMM(const mitk::Point2D &pt_display, mitk::Point2D &pt_mm) const;


    //##ModelId=3E48D5D7027E
    virtual void MMToDisplay(const mitk::Point2D &pt_mm, mitk::Point2D &pt_display) const;
    //##ModelId=3E48E2AE03A7
    virtual void DisplayToMM(const mitk::Vector2D &vec_display, mitk::Vector2D &vec_mm) const;


    //##ModelId=3E48E2B40374
    virtual void MMToDisplay(const mitk::Vector2D &vec_mm, mitk::Vector2D &vec_display) const;

    /** Return this objects modified time.  */
    //##ModelId=3E66CC5A0118
    virtual unsigned long GetMTime() const;

protected:
    //##ModelId=3E3AE91A035E
    DisplayGeometry();
    //##ModelId=3E3AE91A037C
    virtual ~DisplayGeometry();

    //##ModelId=3E3AE9350140
    mitk::Vector2D m_OriginInUnits;
    //##ModelId=3E3C521C01EE
    mitk::Vector2D m_OriginInDisplayUnits;
    //##ModelId=3E3AE94D00AE
    float m_ScaleFactorUnitsPerDisplayUnit;
    //##ModelId=3E3D88E70252
    mitk::Vector2D m_SizeInUnits;
    //##ModelId=3E3AE97701A9
    mitk::Vector2D m_SizeInDisplayUnits;
    //##ModelId=3E3AEA9001D5
    mitk::Geometry2D::ConstPointer m_WorldGeometry;
};

} // namespace mitk



#endif /* DISPLAYGEOMETRY_H_HEADER_INCLUDED_C1B77F88 */
