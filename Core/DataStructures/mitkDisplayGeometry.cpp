/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include "mitkDisplayGeometry.h"

//##ModelId=3E3AE9FD00C5
void mitk::DisplayGeometry::Zoom(mitk::ScalarType factor, const mitk::Point2D& centerInDisplayUnits)
{
  assert(factor > 0);

  SetScaleFactor(m_ScaleFactorUnitsPerDisplayUnit/factor);
  SetOriginInUnits(m_OriginInUnits-centerInDisplayUnits.GetVectorFromOrigin()*(1-factor)*m_ScaleFactorUnitsPerDisplayUnit);

  Modified();
}

//##ModelId=3E3AEE700380
void mitk::DisplayGeometry::MoveBy(const mitk::Vector2D& shiftInDisplayUnits)
{
  SetOriginInUnits(m_OriginInUnits+shiftInDisplayUnits*m_ScaleFactorUnitsPerDisplayUnit);

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
  pt_units[0]=m_ScaleFactorUnitsPerDisplayUnit*pt_display[0]+m_OriginInUnits[0];
  pt_units[1]=m_ScaleFactorUnitsPerDisplayUnit*pt_display[1]+m_OriginInUnits[1];
}

//##ModelId=3E3AEBEF039C
void mitk::DisplayGeometry::UnitsToDisplay(const mitk::Point2D &pt_units, mitk::Point2D &pt_display) const
{
  pt_display[0]=(pt_units[0]-m_OriginInUnits[0])*(1.0/m_ScaleFactorUnitsPerDisplayUnit);
  pt_display[1]=(pt_units[1]-m_OriginInUnits[1])*(1.0/m_ScaleFactorUnitsPerDisplayUnit);
}

//##ModelId=3E3B02710239
void mitk::DisplayGeometry::DisplayToUnits(const mitk::Vector2D &vec_display, mitk::Vector2D &vec_units) const
{
  vec_units=vec_display*m_ScaleFactorUnitsPerDisplayUnit;
}

//##ModelId=3E3B02760128
void mitk::DisplayGeometry::UnitsToDisplay(const mitk::Vector2D &vec_units, mitk::Vector2D &vec_display) const
{
  vec_display=vec_units*(1.0/m_ScaleFactorUnitsPerDisplayUnit);
}

bool mitk::DisplayGeometry::Project(const mitk::Point3D &pt3d_mm, mitk::Point3D &projectedPt3d_mm) const
{
  if(m_WorldGeometry.IsNull()) return false;
  return m_WorldGeometry->Project(pt3d_mm, projectedPt3d_mm);
 }

bool mitk::DisplayGeometry::Project(const mitk::Point3D & atPt3d_mm, const mitk::Vector3D &vec3d_mm, mitk::Vector3D &projectedVec3d_mm) const
{
  if(m_WorldGeometry.IsNull()) return false;
  return m_WorldGeometry->Project(atPt3d_mm, vec3d_mm, projectedVec3d_mm);
}

//##ModelId=3E3AEB460259
bool mitk::DisplayGeometry::Map(const mitk::Point3D &pt3d_mm, mitk::Point2D &pt2d_mm) const
{
  if(m_WorldGeometry.IsNull()) return false;
  return m_WorldGeometry->Map(pt3d_mm, pt2d_mm);
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
  m_WorldGeometry->MMToUnits(pt_mm, pt_display);

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
  m_WorldGeometry->MMToUnits(vec_mm, vec_display);

  UnitsToDisplay(vec_display, vec_display);
}

//##ModelId=3EF4364802A9
void mitk::DisplayGeometry::ULDisplayToDisplay(const mitk::Point2D &pt_ULdisplay, mitk::Point2D &pt_display) const
{
  pt_display[0]=pt_ULdisplay[0];
  pt_display[1]=GetDisplayHeight()-pt_ULdisplay[1];
}

//##ModelId=3EF43649007A
void mitk::DisplayGeometry::DisplayToULDisplay(const mitk::Point2D &pt_display, mitk::Point2D &pt_ULdisplay) const
{
  ULDisplayToDisplay(pt_display, pt_ULdisplay);
}

//##ModelId=3EF436490251
void mitk::DisplayGeometry::ULDisplayToDisplay(const mitk::Vector2D &vec_ULdisplay, mitk::Vector2D &vec_display) const
{
  vec_display[0]= vec_ULdisplay[0];
  vec_display[1]=-vec_ULdisplay[1];
}

//##ModelId=3EF4364A0049
void mitk::DisplayGeometry::DisplayToULDisplay(const mitk::Vector2D &vec_display, mitk::Vector2D &vec_ULdisplay) const
{
  ULDisplayToDisplay(vec_display, vec_ULdisplay);
}

//##ModelId=3EF436450223
void mitk::DisplayGeometry::ULDisplayToUnits(const mitk::Point2D &pt_ULdisplay, mitk::Point2D &pt_units) const
{
  pt_units[0]=pt_ULdisplay[0];
  pt_units[1]=GetDisplayHeight()-pt_ULdisplay[1];
  DisplayToUnits(pt_units, pt_units);
}

//##ModelId=3EF43645039F
void mitk::DisplayGeometry::UnitsToULDisplay(const mitk::Point2D &pt_units, mitk::Point2D &pt_ULdisplay) const
{
  UnitsToDisplay(pt_units, pt_ULdisplay);
  pt_ULdisplay[1]=GetDisplayHeight()-pt_ULdisplay[1];
}

//##ModelId=3EF436460120
void mitk::DisplayGeometry::ULDisplayToUnits(const mitk::Vector2D &vec_ULdisplay, mitk::Vector2D &vec_units) const
{
  vec_units[0]= m_ScaleFactorUnitsPerDisplayUnit*vec_ULdisplay[0];
  vec_units[1]=-m_ScaleFactorUnitsPerDisplayUnit*vec_ULdisplay[1];
}

//##ModelId=3EF436460292
void mitk::DisplayGeometry::UnitsToULDisplay(const mitk::Vector2D &vec_units, mitk::Vector2D &vec_ULdisplay) const
{
  vec_ULdisplay[0]= vec_units[0]*(1.0/m_ScaleFactorUnitsPerDisplayUnit);
  vec_ULdisplay[1]=-vec_units[1]*(1.0/m_ScaleFactorUnitsPerDisplayUnit);
}

//##ModelId=3EF436470027
void mitk::DisplayGeometry::ULDisplayToMM(const mitk::Point2D &pt_ULdisplay, mitk::Point2D &pt_mm) const
{
  ULDisplayToUnits(pt_ULdisplay, pt_mm);

  if(m_WorldGeometry.IsNull()) return;
  m_WorldGeometry->UnitsToMM(pt_mm, pt_mm);
}

//##ModelId=3EF4364701AE
void mitk::DisplayGeometry::MMToULDisplay(const mitk::Point2D &pt_mm, mitk::Point2D &pt_ULdisplay) const
{
  if(m_WorldGeometry.IsNull()) return;
  m_WorldGeometry->MMToUnits(pt_mm, pt_ULdisplay);

  UnitsToULDisplay(pt_ULdisplay, pt_ULdisplay);
}

//##ModelId=3EF436470348
void mitk::DisplayGeometry::ULDisplayToMM(const mitk::Vector2D &vec_ULdisplay, mitk::Vector2D &vec_mm) const
{
  ULDisplayToUnits(vec_ULdisplay, vec_mm);

  if(m_WorldGeometry.IsNull()) return;
  m_WorldGeometry->UnitsToMM(vec_mm, vec_mm);
}

//##ModelId=3EF436480105
void mitk::DisplayGeometry::MMToULDisplay(const mitk::Vector2D &vec_mm, mitk::Vector2D &vec_ULdisplay) const
{
  if(m_WorldGeometry.IsNull()) return;
  m_WorldGeometry->MMToUnits(vec_mm, vec_ULdisplay);

  UnitsToULDisplay(vec_ULdisplay, vec_ULdisplay);
}

//##ModelId=3E3AEB620231
const mitk::TimeBounds& mitk::DisplayGeometry::GetTimeBoundsInMS() const
{
  if(m_WorldGeometry.IsNull()) 
    return m_TimeBoundsInMS;

  return m_WorldGeometry->GetTimeBoundsInMS();
}

//##ModelId=3E3AE91A035E
mitk::DisplayGeometry::DisplayGeometry() : 
  m_ScaleFactorUnitsPerDisplayUnit(1.0),
  m_WorldGeometry(NULL)
{
  m_SizeInDisplayUnits.Fill(10);
  m_OriginInUnits.Fill(0);
}

//##ModelId=3E3AE91A037C
mitk::DisplayGeometry::~DisplayGeometry()
{
}

//##ModelId=3E3C36920345
void mitk::DisplayGeometry::SetSizeInDisplayUnits(unsigned int width, unsigned int height, bool keepDisplayedRegion)
{
  Vector2D centerInUnits;
  centerInUnits =  m_SizeInUnits*0.5;

  m_SizeInDisplayUnits[0]=width;
  m_SizeInDisplayUnits[1]=height;

  if(m_SizeInDisplayUnits[0] <= 0)
    m_SizeInDisplayUnits[0] = 1;
  if(m_SizeInDisplayUnits[1] <= 0)
    m_SizeInDisplayUnits[1] = 1;

  DisplayToUnits(m_SizeInDisplayUnits, m_SizeInUnits);

  if(keepDisplayedRegion)
  {
    Vector2D newCenterInUnits;
    newCenterInUnits = m_SizeInUnits*0.5;

    Vector2D shift;
    shift=centerInUnits-newCenterInUnits;
    UnitsToDisplay(shift, shift);

    m_ScaleFactorUnitsPerDisplayUnit *= sqrt(centerInUnits.GetSquaredNorm()/newCenterInUnits.GetSquaredNorm());
    assert(m_ScaleFactorUnitsPerDisplayUnit<ScalarTypeNumericTraits::infinity());

    MoveBy(shift);
  }

  Modified();
}

//##ModelId=3E3C36CD02D2
void mitk::DisplayGeometry::SetOriginInUnits(const mitk::Vector2D& origin_units)
{
  m_OriginInUnits=origin_units;
  UnitsToDisplay(m_OriginInUnits, m_OriginInDisplayUnits);

  Modified();
}

//##ModelId=3E3C38660226
void mitk::DisplayGeometry::Fit()
{
  if(m_WorldGeometry.IsNull()) return;

  mitk::ScalarType   x,y,w,h;

  int width, height;
  // @FIXME: try to remove all the casts

  width=(int)m_SizeInDisplayUnits[0];
  height=(int)m_SizeInDisplayUnits[1];

  w = width;
  h = height;

  const ScalarType& widthInUnits(m_WorldGeometry->GetExtent(0));
  const ScalarType& heightInUnits(m_WorldGeometry->GetExtent(1));
  mitk::ScalarType aspRatio=((mitk::ScalarType)widthInUnits)/heightInUnits;

  x = (mitk::ScalarType)w/widthInUnits;
  y = (mitk::ScalarType)h/heightInUnits;
  if (x > y)
    w = (int) (aspRatio*h);
  else
    h = (int) (w/aspRatio);

  if(w>0)
    SetScaleFactor(widthInUnits/w);

  mitk::Vector2D origin_display;
  origin_display[0]=-(width-w)/2.0;
  origin_display[1]=-(height-h)/2.0;
  SetOriginInUnits(origin_display*m_ScaleFactorUnitsPerDisplayUnit);

  Modified();
}

//##ModelId=3E3C516B0045
mitk::Vector2D mitk::DisplayGeometry::GetOriginInUnits() const
{
  return m_OriginInUnits;
}

//##ModelId=3E3C51890107
mitk::Vector2D mitk::DisplayGeometry::GetOriginInDisplayUnits() const
{
  return m_OriginInDisplayUnits;
}

//##ModelId=3E3C666E0299
unsigned int mitk::DisplayGeometry::GetDisplayWidth() const
{
  assert(m_SizeInDisplayUnits[0] >= 0);  
  return (unsigned int)m_SizeInDisplayUnits[0];
}

//##ModelId=3E3C668B0363
unsigned int mitk::DisplayGeometry::GetDisplayHeight() const
{
  assert(m_SizeInDisplayUnits[1] >= 0);  
  return (unsigned int)m_SizeInDisplayUnits[1];
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
void mitk::DisplayGeometry::SetScaleFactor(mitk::ScalarType unitsPerDisplayUnit)
{
  if(unitsPerDisplayUnit<0.0001) unitsPerDisplayUnit=0.0001;

  m_ScaleFactorUnitsPerDisplayUnit = unitsPerDisplayUnit;
  assert(m_ScaleFactorUnitsPerDisplayUnit<ScalarTypeNumericTraits::infinity());

  DisplayToUnits(m_SizeInDisplayUnits, m_SizeInUnits);
}

//##ModelId=3E3ED45900A1
mitk::ScalarType mitk::DisplayGeometry::GetScaleFactorUnitsPerDisplayUnit() const
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

//##ModelId=3EF4935A01B6
bool mitk::DisplayGeometry::Map(const mitk::Point3D & atPt3d_mm, const mitk::Vector3D &vec3d_mm, mitk::Vector2D &vec2d_mm) const
{
  if(m_WorldGeometry.IsNull()) return false;

  return m_WorldGeometry->Map(atPt3d_mm, vec3d_mm, vec2d_mm);
}

//##ModelId=3EF4935C03B8
void mitk::DisplayGeometry::Map(const mitk::Point2D & atPt2d_mm, const mitk::Vector2D &vec2d_mm, mitk::Vector3D &vec3d_mm) const
{
  if(m_WorldGeometry.IsNull()) return;

  m_WorldGeometry->Map(atPt2d_mm, vec2d_mm, vec3d_mm);    
}

