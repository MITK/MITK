#include "DisplayGeometry.h"

//##ModelId=3E3AE9FD00C5
void mitk::DisplayGeometry::Zoom(float factor, const mitk::Vector2D& centerInDisplayUnits)
{
    assert(factor > 0);

    SetScaleFactor(m_ScaleFactorUnitsPerDisplayUnit/factor);
    SetOriginInUnits(m_OriginInUnits-(1-factor)*m_ScaleFactorUnitsPerDisplayUnit*centerInDisplayUnits);

    Modified();
}

//##ModelId=3E3AEE700380
void mitk::DisplayGeometry::MoveBy(const mitk::Vector2D& shiftInDisplayUnits)
{
    SetOriginInUnits(m_OriginInUnits+m_ScaleFactorUnitsPerDisplayUnit*shiftInDisplayUnits);

    Modified();
}

//##ModelId=3E3AEA5101A2
void mitk::DisplayGeometry::SetWorldGeometry(const mitk::Geometry2D* aWorldGeometry)
{
    m_WorldGeometry = aWorldGeometry;

    Modified();
}

//##ModelId=3E3AEB8F0286
void mitk::DisplayGeometry::DisplayToUnits(const mitk::Point2D &pt_display, mitk::Point2D &pt_units) const
{
    pt_units.x=m_ScaleFactorUnitsPerDisplayUnit*pt_display.x+m_OriginInUnits.x;
    pt_units.y=m_ScaleFactorUnitsPerDisplayUnit*pt_display.y+m_OriginInUnits.y;
}

//##ModelId=3E3AEBEF039C
void mitk::DisplayGeometry::UnitsToDisplay(const mitk::Point2D &pt_units, mitk::Point2D &pt_display) const
{
    pt_display.x=(pt_units.x-m_OriginInUnits.x)*(1.0/m_ScaleFactorUnitsPerDisplayUnit);
    pt_display.y=(pt_units.y-m_OriginInUnits.y)*(1.0/m_ScaleFactorUnitsPerDisplayUnit);
}

//##ModelId=3E3B02710239
void mitk::DisplayGeometry::DisplayToUnits(const mitk::Vector2D &vec_display, mitk::Vector2D &vec_units) const
{
    vec_units=m_ScaleFactorUnitsPerDisplayUnit*vec_display;
}

//##ModelId=3E3B02760128
void mitk::DisplayGeometry::UnitsToDisplay(const mitk::Vector2D &vec_units, mitk::Vector2D &vec_display) const
{
    vec_display=vec_units*(1.0/m_ScaleFactorUnitsPerDisplayUnit);
}

//##ModelId=3E3AEB460259
void mitk::DisplayGeometry::Map(const mitk::Point3D &pt3d_mm, mitk::Point2D &pt2d_mm) const
{
    if(m_WorldGeometry.IsNull()) return;

    m_WorldGeometry->Map(pt3d_mm, pt2d_mm);
}

//##ModelId=3E3AEB490380
void mitk::DisplayGeometry::Map(const mitk::Point2D &pt2d_mm, mitk::Point3D &pt3d_mm) const
{
    if(m_WorldGeometry.IsNull()) return;

    m_WorldGeometry->Map(pt2d_mm, pt3d_mm);    
}

//##ModelId=3E3AEB52008A
void mitk::DisplayGeometry::UnitsToMM(const mitk::Point2D &pt_units, mitk::Point2D &pt_mm) const
{
    if(m_WorldGeometry.IsNull()) return;
    m_WorldGeometry->UnitsToMM(pt_units, pt_mm);
}

//##ModelId=3E3AEB54017D
void mitk::DisplayGeometry::MMToUnits(const mitk::Point2D &pt_mm, mitk::Point2D &pt_units) const
{
    if(m_WorldGeometry.IsNull()) return;
    m_WorldGeometry->MMToUnits(pt_mm, pt_units);
}

//##ModelId=3E3B994802B6
void mitk::DisplayGeometry::UnitsToMM(const mitk::Vector2D &vec_units, mitk::Vector2D &vec_mm) const
{
    if(m_WorldGeometry.IsNull()) return;
    m_WorldGeometry->UnitsToMM(vec_units, vec_mm);
}

//##ModelId=3E3B994B02A6
void mitk::DisplayGeometry::MMToUnits(const mitk::Vector2D &vec_mm, mitk::Vector2D &vec_units) const
{
    if(m_WorldGeometry.IsNull()) return;
    m_WorldGeometry->MMToUnits(vec_mm, vec_units);
}

//##ModelId=3E48D5B40210
void mitk::DisplayGeometry::DisplayToMM(const mitk::Point2D &pt_display, mitk::Point2D &pt_mm) const
{
    DisplayToUnits(pt_display, pt_mm);

    if(m_WorldGeometry.IsNull()) return;
    m_WorldGeometry->UnitsToMM(pt_mm, pt_mm);
}

//##ModelId=3E48D5D7027E
void mitk::DisplayGeometry::MMToDisplay(const mitk::Point2D &pt_mm, mitk::Point2D &pt_display) const
{
    if(m_WorldGeometry.IsNull()) return;
    m_WorldGeometry->UnitsToMM(pt_mm, pt_display);

    UnitsToDisplay(pt_display, pt_display);
}

//##ModelId=3E48E2AE03A7
void mitk::DisplayGeometry::DisplayToMM(const mitk::Vector2D &vec_display, mitk::Vector2D &vec_mm) const
{
    DisplayToUnits(vec_display, vec_mm);

    if(m_WorldGeometry.IsNull()) return;
    m_WorldGeometry->UnitsToMM(vec_mm, vec_mm);
}

//##ModelId=3E48E2B40374
void mitk::DisplayGeometry::MMToDisplay(const mitk::Vector2D &vec_mm, mitk::Vector2D &vec_display) const
{
    if(m_WorldGeometry.IsNull()) return;
    m_WorldGeometry->UnitsToMM(vec_mm, vec_display);

    UnitsToDisplay(vec_display, vec_display);
}

void mitk::DisplayGeometry::ULDisplayToDisplay(const mitk::Point2D &pt_ULdisplay, mitk::Point2D &pt_display) const
{
    pt_display.x=pt_ULdisplay.x;
    pt_display.y=GetDisplayHeight()-pt_ULdisplay.y;
}

void mitk::DisplayGeometry::DisplayToULDisplay(const mitk::Point2D &pt_display, mitk::Point2D &pt_ULdisplay) const
{
    ULDisplayToDisplay(pt_display, pt_ULdisplay);
}

void mitk::DisplayGeometry::ULDisplayToDisplay(const mitk::Vector2D &vec_ULdisplay, mitk::Vector2D &vec_display) const
{
    vec_display.x= vec_ULdisplay.x;
    vec_display.y=-vec_ULdisplay.y;
}

void mitk::DisplayGeometry::DisplayToULDisplay(const mitk::Vector2D &vec_display, mitk::Vector2D &vec_ULdisplay) const
{
    ULDisplayToDisplay(vec_display, vec_ULdisplay);
}

void mitk::DisplayGeometry::ULDisplayToUnits(const mitk::Point2D &pt_ULdisplay, mitk::Point2D &pt_units) const
{
    pt_units.x=pt_ULdisplay.x;
    pt_units.y=GetDisplayHeight()-pt_ULdisplay.y;
    DisplayToUnits(pt_units, pt_units);
}

void mitk::DisplayGeometry::UnitsToULDisplay(const mitk::Point2D &pt_units, mitk::Point2D &pt_ULdisplay) const
{
    UnitsToDisplay(pt_units, pt_ULdisplay);
    pt_ULdisplay.y=GetDisplayHeight()-pt_ULdisplay.y;
}

void mitk::DisplayGeometry::ULDisplayToUnits(const mitk::Vector2D &vec_ULdisplay, mitk::Vector2D &vec_units) const
{
    vec_units.x= m_ScaleFactorUnitsPerDisplayUnit*vec_ULdisplay.x;
    vec_units.y=-m_ScaleFactorUnitsPerDisplayUnit*vec_ULdisplay.y;
}

void mitk::DisplayGeometry::UnitsToULDisplay(const mitk::Vector2D &vec_units, mitk::Vector2D &vec_ULdisplay) const
{
    vec_ULdisplay.x= vec_units.x*(1.0/m_ScaleFactorUnitsPerDisplayUnit);
    vec_ULdisplay.y=-vec_units.y*(1.0/m_ScaleFactorUnitsPerDisplayUnit);
}

void mitk::DisplayGeometry::ULDisplayToMM(const mitk::Point2D &pt_ULdisplay, mitk::Point2D &pt_mm) const
{
    ULDisplayToUnits(pt_ULdisplay, pt_mm);

    if(m_WorldGeometry.IsNull()) return;
    m_WorldGeometry->UnitsToMM(pt_mm, pt_mm);
}

void mitk::DisplayGeometry::MMToULDisplay(const mitk::Point2D &pt_mm, mitk::Point2D &pt_ULdisplay) const
{
    if(m_WorldGeometry.IsNull()) return;
    m_WorldGeometry->UnitsToMM(pt_mm, pt_ULdisplay);

    UnitsToULDisplay(pt_ULdisplay, pt_ULdisplay);
}

void mitk::DisplayGeometry::ULDisplayToMM(const mitk::Vector2D &vec_ULdisplay, mitk::Vector2D &vec_mm) const
{
    ULDisplayToUnits(vec_ULdisplay, vec_mm);

    if(m_WorldGeometry.IsNull()) return;
    m_WorldGeometry->UnitsToMM(vec_mm, vec_mm);
}

void mitk::DisplayGeometry::MMToULDisplay(const mitk::Vector2D &vec_mm, mitk::Vector2D &vec_ULdisplay) const
{
    if(m_WorldGeometry.IsNull()) return;
    m_WorldGeometry->UnitsToMM(vec_mm, vec_ULdisplay);

    UnitsToULDisplay(vec_ULdisplay, vec_ULdisplay);
}

//##ModelId=3E3AEB56037E
itk::Transform<float,3,2>::Pointer mitk::DisplayGeometry::GetTransfrom() const
{
	itkExceptionMacro("Transform not yet supported."); 	
	return NULL;
}

//##ModelId=3E3AEB620231
double mitk::DisplayGeometry::GetTime() const
{
    if(m_WorldGeometry.IsNull()) return 0.0;

    
    return m_WorldGeometry->GetTime();
}

//##ModelId=3E3AE91A035E
mitk::DisplayGeometry::DisplayGeometry() : Geometry2D(10.0, 10.0), m_OriginInUnits(0,0), 
    m_ScaleFactorUnitsPerDisplayUnit(1.0), m_SizeInDisplayUnits(10,10), m_WorldGeometry(NULL)
{
}

//##ModelId=3E3AE91A037C
mitk::DisplayGeometry::~DisplayGeometry()
{
}

//##ModelId=3E3C36920345
void mitk::DisplayGeometry::SetSizeInDisplayUnits(unsigned int width, unsigned int height)
{
    m_SizeInDisplayUnits.set(width, height);

    if(m_SizeInDisplayUnits.x <= 0)
        m_SizeInDisplayUnits.x = 1;
    if(m_SizeInDisplayUnits.y <= 0)
        m_SizeInDisplayUnits.y = 1;

    DisplayToUnits(m_SizeInDisplayUnits, m_SizeInUnits);

    Modified();
}

//##ModelId=3E3C36CD02D2
void mitk::DisplayGeometry::SetOriginInUnits(const mitk::Point2D& origin_units)
{
    m_OriginInUnits=origin_units;
    UnitsToDisplay(m_OriginInUnits, m_OriginInDisplayUnits);

    Modified();
}

//##ModelId=3E3C38660226
void mitk::DisplayGeometry::Fit()
{
    if(m_WorldGeometry.IsNull()) return;

    float   x,y,w,h;

    int width, height;

    width=m_SizeInDisplayUnits.x;
    height=m_SizeInDisplayUnits.y;

// FIXME: "silly assignment" (IDEA)
	w = width;
	h = height;

    float aspRatio=((float)m_WorldGeometry->GetWidthInUnits())/m_WorldGeometry->GetHeightInUnits();

    x = (float)w/m_WorldGeometry->GetWidthInUnits();
    y = (float)h/m_WorldGeometry->GetHeightInUnits();
    if (x > y)
        w = (int) (aspRatio*h);
    else
        h = (int) (w/aspRatio);

    SetScaleFactor(m_WorldGeometry->GetWidthInUnits()/w);

    mitk::Point2D origin_display(-(width-w)/2.0,-(height-h)/2.0);
    SetOriginInUnits(origin_display*m_ScaleFactorUnitsPerDisplayUnit);

    Modified();
}

//##ModelId=3E3C516B0045
mitk::Point2D mitk::DisplayGeometry::GetOriginInUnits() const
{
    return m_OriginInUnits;
}

//##ModelId=3E3C51890107
mitk::Point2D mitk::DisplayGeometry::GetOriginInDisplayUnits() const
{
    return m_OriginInDisplayUnits;
}

//##ModelId=3E3C666E0299
unsigned int mitk::DisplayGeometry::GetDisplayWidth() const
{
    return m_SizeInDisplayUnits.x;
}

//##ModelId=3E3C668B0363
unsigned int mitk::DisplayGeometry::GetDisplayHeight() const
{
    return m_SizeInDisplayUnits.y;
}

//##ModelId=3E3D86CB0039
mitk::Vector2D mitk::DisplayGeometry::GetSizeInDisplayUnits() const
{
    return m_SizeInDisplayUnits;
}

//##ModelId=3E3D86E9015E
mitk::Vector2D mitk::DisplayGeometry::GetSizeInUnits() const
{
    return m_SizeInUnits;
}

//##ModelId=3E3D883B0132
void mitk::DisplayGeometry::SetScaleFactor(float unitsPerDisplayUnit)
{
    if(unitsPerDisplayUnit<0.0001) unitsPerDisplayUnit=0.0001;

    m_ScaleFactorUnitsPerDisplayUnit = unitsPerDisplayUnit;

    DisplayToUnits(m_SizeInDisplayUnits, m_SizeInUnits);
}

//##ModelId=3E3ED45900A1
float mitk::DisplayGeometry::GetScaleFactorUnitsPerDisplayUnit() const
{
    return m_ScaleFactorUnitsPerDisplayUnit;
}

//##ModelId=3E66CC5A0118
unsigned long mitk::DisplayGeometry::GetMTime() const
{
    if(Geometry2D::GetMTime()<m_WorldGeometry->GetMTime())
        Modified();
    return Geometry2D::GetMTime();
}

