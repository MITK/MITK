#ifndef DISPLAYGEOMETRY_H_HEADER_INCLUDED_C1B77F88
#define DISPLAYGEOMETRY_H_HEADER_INCLUDED_C1B77F88

#include "mitkCommon.h"
#include "mitkGeometry2D.h"
#include "mitkImageSliceSelector.h"

namespace mitk {

//##ModelId=3E3AE8F401DD
//##Documentation
//## @brief Describes the geometry on the display/screen for 2D display.
//## @ingroup Geometry
//## The main purpose of this class is to convert between display coordinates
//## (in display-units) and world coordinates (in mm).
//## DisplayGeometry depends on the size of the display area (widget width and
//## height, m_SizeInDisplayUnits) and on a Geometry2D (m_WoldGeometry). It
//## represents a recangular view on this world-geometry. E.g., you can tell
//## the DisplayGeometry to fit the world-geometry in the display area by
//## calling Fit(). Provides methods for zooming and panning.
//## @warning @em Units refers to the units of the underlying world-geometry.
//## Take care, whether these are really the units you want to convert to.
//## E.g., when you want to convert a point @a pt_display (which is 2D) given
//## in display coordinates into a point in units of a BaseData-object @a datum
//## (the requested point is 3D!), use
//## @code
//## displaygeometry->DisplayToMM(pt_display, pt_mm);
//## datum->GetGeometry()->MMToUnits(pt_mm, pt_datum_units);
//## @endcode
//## Even, if you want to convert the 2D point @a pt_display into a 2D point in
//## units on a certain 2D geometry @a certaingeometry, it is safer to use
//## @code
//## displaygeometry->DisplayToMM(pt_display, pt_mm);
//## certaingeometry->MMToUnits(pt_mm, pt_certain_geometry_units);
//## @endcode
//## unless you can be sure that the underlying geometry of @a displaygeometry
//## is really the @a certaingeometry.
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

    //##ModelId=3EF436450223
    virtual void ULDisplayToUnits(const mitk::Point2D &pt_ULdisplay, mitk::Point2D &pt_units) const;
    //##ModelId=3EF43645039F
    virtual void UnitsToULDisplay(const mitk::Point2D &pt_units, mitk::Point2D &pt_ULdisplay) const;

    //##ModelId=3E3C36920345
    //##Documentation
    //## @brief Set the size of the display in display units.
    //## 
    //## This method must be called every time the display is resized (normally, the GUI-toolkit 
    //## informs about resizing).
    //## @param keepDisplayedRegion: if @a true (the default), the displayed contents is zoomed/shrinked
    //## so that the displayed region is (approximately) the same as before: The point at the center will 
    //## be kept at the center and the length of the diagonal of the displayed region @em in @em units
    //## will also be kept.
    //## When the aspect ration changes, the displayed region includes the old displayed region, but
    //## cannot be exaclty the same.
    //## @warning the described behaviour is implemented, but does not work yet. Sorry, didn't have the time to debug it yet.
    //## Therefore, the old behaviour is used in BaseRenderer::Resize (calling Fit).
    //## @todo debug!
    virtual void SetSizeInDisplayUnits(unsigned int width, unsigned int height, bool keepDisplayedRegion=true);
    //##ModelId=3E3C36CD02D2
    virtual void SetOriginInUnits(const mitk::Vector2D& origin_units);
    //##ModelId=3E3C516B0045
    mitk::Point2D GetOriginInUnits() const;
    //##ModelId=3E3C51890107
    mitk::Point2D GetOriginInDisplayUnits() const;

    //##ModelId=3E3AEB460259
    virtual bool Map(const mitk::Point3D &pt3d_mm, mitk::Point2D &pt2d_mm) const;
    //##ModelId=3E3AEB490380
    virtual void Map(const mitk::Point2D &pt2d_mm, mitk::Point3D &pt3d_mm) const;

    //##ModelId=3E3AEB52008A
    virtual void UnitsToMM(const mitk::Point2D &pt_units, mitk::Point2D &pt_mm) const;
    //##ModelId=3E3AEB54017D
    virtual void MMToUnits(const mitk::Point2D &pt_mm, mitk::Point2D &pt_units) const;

    //##ModelId=3E3AEB620231
    virtual double GetTime(int t) const;

    //##ModelId=3E3B02710239
    virtual void DisplayToUnits(const mitk::Vector2D &vec_display, mitk::Vector2D &vec_units) const;
    //##ModelId=3E3B02760128
    virtual void UnitsToDisplay(const mitk::Vector2D &vec_units, mitk::Vector2D &vec_display) const;

    //##ModelId=3EF436460120
    virtual void ULDisplayToUnits(const mitk::Vector2D &vec_ULdisplay, mitk::Vector2D &vec_units) const;
    //##ModelId=3EF436460292
    virtual void UnitsToULDisplay(const mitk::Vector2D &vec_units, mitk::Vector2D &vec_ULdisplay) const;

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

    //##ModelId=3EF436470027
    virtual void ULDisplayToMM(const mitk::Point2D &pt_ULdisplay, mitk::Point2D &pt_mm) const;
    //##ModelId=3EF4364701AE
    virtual void MMToULDisplay(const mitk::Point2D &pt_mm, mitk::Point2D &pt_ULdisplay) const;

    //##ModelId=3EF436470348
    virtual void ULDisplayToMM(const mitk::Vector2D &vec_ULdisplay, mitk::Vector2D &vec_mm) const;
    //##ModelId=3EF436480105
    virtual void MMToULDisplay(const mitk::Vector2D &vec_mm, mitk::Vector2D &vec_ULdisplay) const;

    //##ModelId=3EF4364802A9
    virtual void ULDisplayToDisplay(const mitk::Point2D &pt_ULdisplay, mitk::Point2D &pt_display) const;
    //##ModelId=3EF43649007A
    virtual void DisplayToULDisplay(const mitk::Point2D &pt_display, mitk::Point2D &pt_ULdisplay) const;

    //##ModelId=3EF436490251
    virtual void ULDisplayToDisplay(const mitk::Vector2D &vec_ULdisplay, mitk::Vector2D &vec_display) const;
    //##ModelId=3EF4364A0049
    virtual void DisplayToULDisplay(const mitk::Vector2D &vec_display, mitk::Vector2D &vec_ULdisplay) const;

    /** Return this objects modified time.  */
    //##ModelId=3E66CC5A0118
    virtual unsigned long GetMTime() const;
    //##ModelId=3EF4935A01B6
    virtual bool Map(const mitk::Point3D & atPt3d_mm, const mitk::Vector3D &vec3d_mm, mitk::Vector2D &vec2d_mm) const;

    //##ModelId=3EF4935C03B8
    virtual void Map(const mitk::Point2D & atPt2d_mm, const mitk::Vector2D &vec2d_mm, mitk::Vector3D &vec3d_mm) const;

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
