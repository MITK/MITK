/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
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

void mitk::DisplayGeometry::Zoom(mitk::ScalarType factor, const mitk::Point2D& centerInDisplayUnits)
{
  assert(factor > 0);

  SetScaleFactor(m_ScaleFactorMMPerDisplayUnit/factor);
  SetOriginInMM(m_OriginInMM-centerInDisplayUnits.GetVectorFromOrigin()*(1-factor)*m_ScaleFactorMMPerDisplayUnit);

  Modified();
}

void mitk::DisplayGeometry::MoveBy(const mitk::Vector2D& shiftInDisplayUnits)
{
  SetOriginInMM(m_OriginInMM+shiftInDisplayUnits*m_ScaleFactorMMPerDisplayUnit);

  Modified();
}

void mitk::DisplayGeometry::SetWorldGeometry(const mitk::Geometry2D* aWorldGeometry)
{
  m_WorldGeometry = aWorldGeometry;

  Modified();
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

bool mitk::DisplayGeometry::Map(const mitk::Point3D &pt3d_mm, mitk::Point2D &pt2d_mm) const
{
  if(m_WorldGeometry.IsNull()) return false;
  return m_WorldGeometry->Map(pt3d_mm, pt2d_mm);
}

void mitk::DisplayGeometry::Map(const mitk::Point2D &pt2d_mm, mitk::Point3D &pt3d_mm) const
{
  if(m_WorldGeometry.IsNull()) return;
  m_WorldGeometry->Map(pt2d_mm, pt3d_mm);    
}

/*
void mitk::DisplayGeometry::IndexToWorld(const mitk::Point2D &pt_units, mitk::Point2D &pt_mm) const
{
  if(m_WorldGeometry.IsNull()) return;
  m_WorldGeometry->IndexToWorld(pt_units, pt_mm);
}

void mitk::DisplayGeometry::WorldToIndex(const mitk::Point2D &pt_mm, mitk::Point2D &pt_units) const
{
  if(m_WorldGeometry.IsNull()) return;
  m_WorldGeometry->WorldToIndex(pt_mm, pt_units);
}

void mitk::DisplayGeometry::IndexToWorld(const mitk::Point2D &atPt2d_units, const mitk::Vector2D &vec_units, mitk::Vector2D &vec_mm) const
{
  if(m_WorldGeometry.IsNull()) return;
  m_WorldGeometry->IndexToWorld(atPt2d_units, vec_units, vec_mm);
}

void mitk::DisplayGeometry::WorldToIndex(const mitk::Point2D &atPt2d_mm, const mitk::Vector2D &vec_mm, mitk::Vector2D &vec_units) const
{
  if(m_WorldGeometry.IsNull()) return;
  m_WorldGeometry->WorldToIndex(atPt2d_mm, vec_mm, vec_units);
}
*/

void mitk::DisplayGeometry::DisplayToWorld(const mitk::Point2D &pt_display, mitk::Point2D &pt_mm) const
{
  pt_mm[0]=m_ScaleFactorMMPerDisplayUnit*pt_display[0]+m_OriginInMM[0];
  pt_mm[1]=m_ScaleFactorMMPerDisplayUnit*pt_display[1]+m_OriginInMM[1];
}

void mitk::DisplayGeometry::WorldToDisplay(const mitk::Point2D &pt_mm, mitk::Point2D &pt_display) const
{
  pt_display[0]=(pt_mm[0]-m_OriginInMM[0])*(1.0/m_ScaleFactorMMPerDisplayUnit);
  pt_display[1]=(pt_mm[1]-m_OriginInMM[1])*(1.0/m_ScaleFactorMMPerDisplayUnit);
}

void mitk::DisplayGeometry::DisplayToWorld(const mitk::Vector2D &vec_display, mitk::Vector2D &vec_mm) const
{
  vec_mm=vec_display*m_ScaleFactorMMPerDisplayUnit;
}

void mitk::DisplayGeometry::WorldToDisplay(const mitk::Vector2D &vec_mm, mitk::Vector2D &vec_display) const
{
  vec_display=vec_mm*(1.0/m_ScaleFactorMMPerDisplayUnit);
}

void mitk::DisplayGeometry::ULDisplayToDisplay(const mitk::Point2D &pt_ULdisplay, mitk::Point2D &pt_display) const
{
  pt_display[0]=pt_ULdisplay[0];
  pt_display[1]=GetDisplayHeight()-pt_ULdisplay[1];
}

void mitk::DisplayGeometry::DisplayToULDisplay(const mitk::Point2D &pt_display, mitk::Point2D &pt_ULdisplay) const
{
  ULDisplayToDisplay(pt_display, pt_ULdisplay);
}

void mitk::DisplayGeometry::ULDisplayToDisplay(const mitk::Vector2D &vec_ULdisplay, mitk::Vector2D &vec_display) const
{
  vec_display[0]= vec_ULdisplay[0];
  vec_display[1]=-vec_ULdisplay[1];
}

void mitk::DisplayGeometry::DisplayToULDisplay(const mitk::Vector2D &vec_display, mitk::Vector2D &vec_ULdisplay) const
{
  ULDisplayToDisplay(vec_display, vec_ULdisplay);
}

void mitk::DisplayGeometry::ULDisplayToMM(const mitk::Point2D &pt_ULdisplay, mitk::Point2D &pt_mm) const
{
  ULDisplayToDisplay(pt_ULdisplay, pt_mm);
  DisplayToWorld(pt_mm, pt_mm);
}

void mitk::DisplayGeometry::MMToULDisplay(const mitk::Point2D &pt_mm, mitk::Point2D &pt_ULdisplay) const
{
  WorldToDisplay(pt_mm, pt_ULdisplay);
  DisplayToULDisplay(pt_ULdisplay, pt_ULdisplay);
}

void mitk::DisplayGeometry::ULDisplayToMM(const mitk::Vector2D &vec_ULdisplay, mitk::Vector2D &vec_mm) const
{
  ULDisplayToDisplay(vec_ULdisplay, vec_mm);
  DisplayToWorld(vec_mm, vec_mm);
}

void mitk::DisplayGeometry::MMToULDisplay(const mitk::Vector2D &vec_mm, mitk::Vector2D &vec_ULdisplay) const
{
  WorldToDisplay(vec_mm, vec_ULdisplay);
  DisplayToULDisplay(vec_ULdisplay, vec_ULdisplay);
}

const mitk::TimeBounds& mitk::DisplayGeometry::GetTimeBounds() const
{
  if(m_WorldGeometry.IsNull()) 
    return m_TimeBounds;

  return m_WorldGeometry->GetTimeBounds();
}

mitk::DisplayGeometry::DisplayGeometry() : 
  m_ScaleFactorMMPerDisplayUnit(1.0),
  m_WorldGeometry(NULL)
{
  m_SizeInDisplayUnits.Fill(10);
  m_OriginInMM.Fill(0);
}

mitk::DisplayGeometry::~DisplayGeometry()
{
}

void mitk::DisplayGeometry::SetSizeInDisplayUnits(unsigned int width, unsigned int height, bool keepDisplayedRegion)
{
  Vector2D oldSizeInMM;
  Point2D oldCenterInMM;
  if(keepDisplayedRegion)
  {
    oldSizeInMM = m_SizeInMM;
    Point2D centerInDisplayUnits;
    centerInDisplayUnits[0] = m_SizeInDisplayUnits[0]*0.5;
    centerInDisplayUnits[1] = m_SizeInDisplayUnits[1]*0.5;
    DisplayToWorld(centerInDisplayUnits, oldCenterInMM);
  }

  m_SizeInDisplayUnits[0]=width;
  m_SizeInDisplayUnits[1]=height;

  if(m_SizeInDisplayUnits[0] <= 0)
    m_SizeInDisplayUnits[0] = 1;
  if(m_SizeInDisplayUnits[1] <= 0)
    m_SizeInDisplayUnits[1] = 1;

  DisplayToWorld(m_SizeInDisplayUnits, m_SizeInMM);

  if(keepDisplayedRegion)
  {
    Point2D positionOfOldCenterInCurrentDisplayUnits;
    WorldToDisplay(oldCenterInMM, positionOfOldCenterInCurrentDisplayUnits);

    Point2D currentNewCenterInDisplayUnits;
    currentNewCenterInDisplayUnits[0] = m_SizeInDisplayUnits[0]*0.5;
    currentNewCenterInDisplayUnits[1] = m_SizeInDisplayUnits[1]*0.5;

    Vector2D shift;
    shift=positionOfOldCenterInCurrentDisplayUnits.GetVectorFromOrigin()-currentNewCenterInDisplayUnits;

    MoveBy(shift);
    Zoom(m_SizeInMM.GetNorm()/oldSizeInMM.GetNorm(), currentNewCenterInDisplayUnits);
  }

  Modified();
}

void mitk::DisplayGeometry::SetOriginInMM(const mitk::Vector2D& origin_mm)
{
  m_OriginInMM = origin_mm;
  WorldToDisplay(m_OriginInMM, m_OriginInDisplayUnits);

  Modified();
}

void mitk::DisplayGeometry::Fit()
{
  if((m_WorldGeometry.IsNull()) || (m_WorldGeometry->IsValid() == false)) return;

  mitk::ScalarType   x,y,w,h;

  int width, height;
  // @FIXME: try to remove all the casts

  width=(int)m_SizeInDisplayUnits[0];
  height=(int)m_SizeInDisplayUnits[1];

  w = width;
  h = height;

  const ScalarType& widthInMM = m_WorldGeometry->GetParametricExtentInMM(0);
  const ScalarType& heightInMM = m_WorldGeometry->GetParametricExtentInMM(1);
  mitk::ScalarType aspRatio=((mitk::ScalarType)widthInMM)/heightInMM;

  x = (mitk::ScalarType)w/widthInMM;
  y = (mitk::ScalarType)h/heightInMM;
  if (x > y)
    w = (int) (aspRatio*h);
  else
    h = (int) (w/aspRatio);

  if(w>0)
    SetScaleFactor(widthInMM/w);

  mitk::Vector2D origin_display;
  origin_display[0]=-(width-w)/2.0;
  origin_display[1]=-(height-h)/2.0;
  SetOriginInMM(origin_display*m_ScaleFactorMMPerDisplayUnit);

  Modified();
}

void mitk::DisplayGeometry::SetScaleFactor(mitk::ScalarType mmPerDisplayUnit)
{
  if(mmPerDisplayUnit<0.0001) mmPerDisplayUnit=0.0001;

  m_ScaleFactorMMPerDisplayUnit = mmPerDisplayUnit;
  assert(m_ScaleFactorMMPerDisplayUnit<ScalarTypeNumericTraits::infinity());

  DisplayToWorld(m_SizeInDisplayUnits, m_SizeInMM);
}

mitk::ScalarType mitk::DisplayGeometry::GetScaleFactorMMPerDisplayUnit() const
{
  return m_ScaleFactorMMPerDisplayUnit;
}

unsigned long mitk::DisplayGeometry::GetMTime() const
{
  if((m_WorldGeometry.IsNotNull()) && (Geometry2D::GetMTime() < m_WorldGeometry->GetMTime()))
    Modified();
  return Geometry2D::GetMTime();
}

bool mitk::DisplayGeometry::Map(const mitk::Point3D & atPt3d_mm, const mitk::Vector3D &vec3d_mm, mitk::Vector2D &vec2d_mm) const
{
  if(m_WorldGeometry.IsNull()) return false;

  return m_WorldGeometry->Map(atPt3d_mm, vec3d_mm, vec2d_mm);
}

void mitk::DisplayGeometry::Map(const mitk::Point2D & atPt2d_mm, const mitk::Vector2D &vec2d_mm, mitk::Vector3D &vec3d_mm) const
{
  if(m_WorldGeometry.IsNull()) return;

  m_WorldGeometry->Map(atPt2d_mm, vec2d_mm, vec3d_mm);    
}

bool mitk::DisplayGeometry::IsValid() const
{
  return m_Valid && m_WorldGeometry.IsNotNull() && m_WorldGeometry->IsValid();
}

mitk::AffineGeometryFrame3D::Pointer mitk::DisplayGeometry::Clone() const
{
  itkExceptionMacro(<<"calling mitk::DisplayGeometry::Clone does not make much sense.");
}

void mitk::DisplayGeometry::PrintSelf(std::ostream& os, itk::Indent indent) const
{
  if(m_WorldGeometry.IsNull())
    os << indent << " WorldGeometry: " << "NULL" << std::endl;
  else
  {
    m_WorldGeometry->Print(os, indent);
    os << indent << " OriginInMM: " << m_OriginInMM << std::endl;
    os << indent << " OriginInDisplayUnits: " << m_OriginInDisplayUnits << std::endl;
    os << indent << " SizeInMM: " << m_SizeInMM << std::endl;
    os << indent << " SizeInDisplayUnits: " << m_SizeInDisplayUnits << std::endl;
    os << indent << " ScaleFactorMMPerDisplayUni: " << m_ScaleFactorMMPerDisplayUnit << std::endl;
  }
  Superclass::PrintSelf(os,indent);
}
