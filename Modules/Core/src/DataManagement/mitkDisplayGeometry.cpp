/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "mitkDisplayGeometry.h"

itk::LightObject::Pointer mitk::DisplayGeometry::InternalClone() const
{
  //  itkExceptionMacro(<<"calling mitk::DisplayGeometry::Clone does not make much sense.");
  DisplayGeometry* returnValue = const_cast<DisplayGeometry *>(this);
  return returnValue;
}

bool mitk::DisplayGeometry::IsValid() const
{
  return m_WorldGeometry.IsNotNull() && m_WorldGeometry->IsValid();
}

unsigned long mitk::DisplayGeometry::GetMTime() const
{
  if((m_WorldGeometry.IsNotNull()) && (PlaneGeometry::GetMTime() < m_WorldGeometry->GetMTime()))
  {
    Modified();
  }
  return PlaneGeometry::GetMTime();
}

//const mitk::TimeBounds& mitk::DisplayGeometry::GetTimeBounds() const
//{
//  if(m_WorldGeometry.IsNull())
//  {
//    return m_TimeBounds;
//  }
//
//  return m_WorldGeometry->GetTimeBounds();
//}

// size definition methods

void mitk::DisplayGeometry::SetWorldGeometry(const PlaneGeometry* aWorldGeometry)
{
  m_WorldGeometry = aWorldGeometry;

  Modified();
}

bool mitk::DisplayGeometry::SetOriginInMM(const Vector2D& origin_mm)
{
  m_OriginInMM = origin_mm;
  WorldToDisplay(m_OriginInMM, m_OriginInDisplayUnits);

  Modified();

  return !this->RefitVisibleRect();
}

mitk::Vector2D mitk::DisplayGeometry::GetOriginInMM() const
{
  return m_OriginInMM;
}

mitk::Vector2D mitk::DisplayGeometry::GetOriginInDisplayUnits() const
{
  return m_OriginInDisplayUnits;
}

void mitk::DisplayGeometry::SetSizeInDisplayUnits(unsigned int width, unsigned int height, bool keepDisplayedRegion)
{
  Vector2D oldSizeInMM( m_SizeInMM );
  Point2D oldCenterInMM;
  if(keepDisplayedRegion)
  {
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
    shift=positionOfOldCenterInCurrentDisplayUnits-currentNewCenterInDisplayUnits;

    MoveBy(shift);
    Zoom(m_SizeInMM.GetNorm()/oldSizeInMM.GetNorm(), currentNewCenterInDisplayUnits);
  }

  Modified();
}

mitk::Vector2D mitk::DisplayGeometry::GetSizeInDisplayUnits() const
{
  return m_SizeInDisplayUnits;
}

mitk::Vector2D mitk::DisplayGeometry::GetSizeInMM() const
{
  return m_SizeInMM;
}

unsigned int mitk::DisplayGeometry::GetDisplayWidth() const
{
  assert(m_SizeInDisplayUnits[0] >= 0);
  return (unsigned int)m_SizeInDisplayUnits[0];
}

unsigned int mitk::DisplayGeometry::GetDisplayHeight() const
{
  assert(m_SizeInDisplayUnits[1] >= 0);
  return (unsigned int)m_SizeInDisplayUnits[1];
}

// zooming, panning, restriction of both

void mitk::DisplayGeometry::SetConstrainZoomingAndPanning(bool constrain)
{
  m_ConstrainZoomingAndPanning = constrain;
  if (m_ConstrainZoomingAndPanning)
  {
    this->RefitVisibleRect();
  }
}

bool mitk::DisplayGeometry::GetConstrainZommingAndPanning() const
{
  return m_ConstrainZoomingAndPanning;
}

bool mitk::DisplayGeometry::SetScaleFactor(ScalarType mmPerDisplayUnit)
{
  if(mmPerDisplayUnit<0.0001)
  {
    mmPerDisplayUnit=0.0001;
  }

  m_ScaleFactorMMPerDisplayUnit = mmPerDisplayUnit;
  assert(m_ScaleFactorMMPerDisplayUnit < itk::NumericTraits<mitk::ScalarType>::infinity());

  DisplayToWorld(m_SizeInDisplayUnits, m_SizeInMM);

  return !this->RefitVisibleRect();
}

mitk::ScalarType mitk::DisplayGeometry::GetScaleFactorMMPerDisplayUnit() const
{
  return m_ScaleFactorMMPerDisplayUnit;
}

// Zooms with a factor (1.0=identity) around the specified center in display units
bool mitk::DisplayGeometry::Zoom(ScalarType factor, const Point2D& centerInDisplayUnits)
{
  assert(factor > 0);

  if ( SetScaleFactor(m_ScaleFactorMMPerDisplayUnit/factor) )
  {
    return SetOriginInMM(m_OriginInMM-centerInDisplayUnits.GetVectorFromOrigin()*(1-factor)*m_ScaleFactorMMPerDisplayUnit);
  }
  else
  {
    return false;
  }
}

// Zooms with a factor (1.0=identity) around the specified center, but tries (if its within view contraints) to match the center in display units with the center in world coordinates.
bool mitk::DisplayGeometry::ZoomWithFixedWorldCoordinates(ScalarType factor, const Point2D& focusDisplayUnits, const Point2D& focusUnitsInMM )
{
  assert(factor > 0);

  if (factor != 1.0)
  {
    SetScaleFactor(m_ScaleFactorMMPerDisplayUnit/factor);
    SetOriginInMM(focusUnitsInMM.GetVectorFromOrigin()-focusDisplayUnits.GetVectorFromOrigin()*m_ScaleFactorMMPerDisplayUnit);
  }

  return true;
}

bool mitk::DisplayGeometry::MoveBy(const Vector2D& shiftInDisplayUnits)
{
  SetOriginInMM(m_OriginInMM+shiftInDisplayUnits*m_ScaleFactorMMPerDisplayUnit);

  Modified();

  return !this->RefitVisibleRect();
}

void mitk::DisplayGeometry::Fit()
{
  if((m_WorldGeometry.IsNull()) || (m_WorldGeometry->IsValid() == false)) return;

  /// \FIXME: try to remove all the casts
  int width=(int)m_SizeInDisplayUnits[0];
  int height=(int)m_SizeInDisplayUnits[1];

  ScalarType w = width;
  ScalarType h = height;

  const ScalarType& widthInMM = m_WorldGeometry->GetExtentInMM(0);
  const ScalarType& heightInMM = m_WorldGeometry->GetExtentInMM(1);
  ScalarType aspRatio=((ScalarType)widthInMM)/heightInMM;

  ScalarType x = (ScalarType)w/widthInMM;
  ScalarType y = (ScalarType)h/heightInMM;

  if (x > y)
  {
    w = (int) (aspRatio*h);
  }
  else
  {
    h = (int) (w/aspRatio);
  }

  if(w>0)
  {
    SetScaleFactor(widthInMM/w);
  }

  Vector2D origin_display;
  origin_display[0]=-(width-w)/2.0;
  origin_display[1]=-(height-h)/2.0;
  SetOriginInMM(origin_display*m_ScaleFactorMMPerDisplayUnit);

  this->RefitVisibleRect();

  Modified();
}

// conversion methods

void mitk::DisplayGeometry::DisplayToWorld(const Point2D &pt_display, Point2D &pt_mm) const
{
  pt_mm[0]=m_ScaleFactorMMPerDisplayUnit*pt_display[0]+m_OriginInMM[0];
  pt_mm[1]=m_ScaleFactorMMPerDisplayUnit*pt_display[1]+m_OriginInMM[1];
}

void mitk::DisplayGeometry::WorldToDisplay(const Point2D &pt_mm, Point2D &pt_display) const
{
  pt_display[0]=(pt_mm[0]-m_OriginInMM[0])*(1.0/m_ScaleFactorMMPerDisplayUnit);
  pt_display[1]=(pt_mm[1]-m_OriginInMM[1])*(1.0/m_ScaleFactorMMPerDisplayUnit);
}

void mitk::DisplayGeometry::DisplayToWorld(const Vector2D &vec_display, Vector2D &vec_mm) const
{
  vec_mm=vec_display*m_ScaleFactorMMPerDisplayUnit;
}

void mitk::DisplayGeometry::WorldToDisplay(const Vector2D &vec_mm, Vector2D &vec_display) const
{
  vec_display=vec_mm*(1.0/m_ScaleFactorMMPerDisplayUnit);
}

void mitk::DisplayGeometry::ULDisplayToMM(const Point2D &pt_ULdisplay, Point2D &pt_mm) const
{
  ULDisplayToDisplay(pt_ULdisplay, pt_mm);
  DisplayToWorld(pt_mm, pt_mm);
}

void mitk::DisplayGeometry::MMToULDisplay(const Point2D &pt_mm, Point2D &pt_ULdisplay) const
{
  WorldToDisplay(pt_mm, pt_ULdisplay);
  DisplayToULDisplay(pt_ULdisplay, pt_ULdisplay);
}

void mitk::DisplayGeometry::ULDisplayToMM(const Vector2D &vec_ULdisplay, Vector2D &vec_mm) const
{
  ULDisplayToDisplay(vec_ULdisplay, vec_mm);
  DisplayToWorld(vec_mm, vec_mm);
}

void mitk::DisplayGeometry::MMToULDisplay(const Vector2D &vec_mm, Vector2D &vec_ULdisplay) const
{
  WorldToDisplay(vec_mm, vec_ULdisplay);
  DisplayToULDisplay(vec_ULdisplay, vec_ULdisplay);
}

void mitk::DisplayGeometry::ULDisplayToDisplay(const Point2D &pt_ULdisplay, Point2D &pt_display) const
{
  pt_display[0]=pt_ULdisplay[0];
  pt_display[1]=GetDisplayHeight()-pt_ULdisplay[1];
}

void mitk::DisplayGeometry::DisplayToULDisplay(const Point2D &pt_display, Point2D &pt_ULdisplay) const
{
  ULDisplayToDisplay(pt_display, pt_ULdisplay);
}

void mitk::DisplayGeometry::ULDisplayToDisplay(const Vector2D &vec_ULdisplay, Vector2D &vec_display) const
{
  vec_display[0]= vec_ULdisplay[0];
  vec_display[1]=-vec_ULdisplay[1];
}

void mitk::DisplayGeometry::DisplayToULDisplay(const Vector2D &vec_display, Vector2D &vec_ULdisplay) const
{
  ULDisplayToDisplay(vec_display, vec_ULdisplay);
}

bool mitk::DisplayGeometry::Project(const Point3D &pt3d_mm, Point3D &projectedPt3d_mm) const
{
  if(m_WorldGeometry.IsNotNull())
  {
    return m_WorldGeometry->Project(pt3d_mm, projectedPt3d_mm);
  }
  else
  {
    return false;
  }
}

bool mitk::DisplayGeometry::Project(const Point3D & atPt3d_mm, const Vector3D &vec3d_mm, Vector3D &projectedVec3d_mm) const
{
  if(m_WorldGeometry.IsNotNull())
  {
    return m_WorldGeometry->Project(atPt3d_mm, vec3d_mm, projectedVec3d_mm);
  }
  else
  {
    return false;
  }
}

bool mitk::DisplayGeometry::Project(const Vector3D &vec3d_mm, Vector3D &projectedVec3d_mm) const
{
  if(m_WorldGeometry.IsNotNull())
  {
    return m_WorldGeometry->Project(vec3d_mm, projectedVec3d_mm);
  }
  else
  {
    return false;
  }
}

bool mitk::DisplayGeometry::Map(const Point3D &pt3d_mm, Point2D &pt2d_mm) const
{
  if(m_WorldGeometry.IsNotNull())
  {
    return m_WorldGeometry->Map(pt3d_mm, pt2d_mm);
  }
  else
  {
    return false;
  }
}

void mitk::DisplayGeometry::Map(const Point2D &pt2d_mm, Point3D &pt3d_mm) const
{
  if(m_WorldGeometry.IsNull()) return;
  m_WorldGeometry->Map(pt2d_mm, pt3d_mm);
}

bool mitk::DisplayGeometry::Map(const Point3D & atPt3d_mm, const Vector3D &vec3d_mm, Vector2D &vec2d_mm) const
{
  if(m_WorldGeometry.IsNotNull())
  {
    return m_WorldGeometry->Map(atPt3d_mm, vec3d_mm, vec2d_mm);
  }
  else
  {
    return false;
  }
}

void mitk::DisplayGeometry::Map(const Point2D & atPt2d_mm, const Vector2D &vec2d_mm, Vector3D &vec3d_mm) const
{
  if(m_WorldGeometry.IsNull()) return;

  m_WorldGeometry->Map(atPt2d_mm, vec2d_mm, vec3d_mm);
}

// protected methods

mitk::DisplayGeometry::DisplayGeometry()
  : PlaneGeometry()
  ,m_ScaleFactorMMPerDisplayUnit(1.0)
  ,m_WorldGeometry(NULL)
  ,m_ConstrainZoomingAndPanning(true)
  ,m_MaxWorldViewPercentage(1.0)
  ,m_MinWorldViewPercentage(0.1)
{
  m_OriginInMM.Fill(0.0);
  m_OriginInDisplayUnits.Fill(0.0);
  m_SizeInMM.Fill(1.0);
  m_SizeInDisplayUnits.Fill(10.0);
}

mitk::DisplayGeometry::~DisplayGeometry()
{
}

bool mitk::DisplayGeometry::RefitVisibleRect()
{
  // do nothing if not asked to
  if (!m_ConstrainZoomingAndPanning) return false;

  // don't allow recursion (need to be fixed, singleton)
  static bool inRecalculate = false;
  if (inRecalculate) return false;
  inRecalculate = true;

  // rename some basic measures of the current viewport and world geometry (MM = milimeters Px = Pixels = display units)
  float displayXMM = m_OriginInMM[0];
  float displayYMM = m_OriginInMM[1];
  float displayWidthPx  = m_SizeInDisplayUnits[0];
  float displayHeightPx = m_SizeInDisplayUnits[1];
  float displayWidthMM  = m_SizeInDisplayUnits[0] * m_ScaleFactorMMPerDisplayUnit;
  float displayHeightMM = m_SizeInDisplayUnits[1] * m_ScaleFactorMMPerDisplayUnit;

  float worldWidthMM  = m_WorldGeometry->GetExtentInMM(0);
  float worldHeightMM = m_WorldGeometry->GetExtentInMM(1);

  // reserve variables for the correction logic to save a corrected origin and zoom factor
  Vector2D newOrigin = m_OriginInMM;
  bool correctPanning = false;

  float newScaleFactor = m_ScaleFactorMMPerDisplayUnit;
  bool correctZooming = false;

  // start of the correction logic

  // zoom to big means:
  // at a given percentage of the world's width/height should be visible. Otherwise
  // the whole screen could show only one pixel
  //
  // zoom to small means:
  // zooming out should be limited at the point where the smaller of the world's sides is completely visible

  bool zoomXtooSmall = displayWidthPx * m_ScaleFactorMMPerDisplayUnit > m_MaxWorldViewPercentage * worldWidthMM;
  bool zoomXtooBig = displayWidthPx * m_ScaleFactorMMPerDisplayUnit < m_MinWorldViewPercentage * worldWidthMM;

  bool zoomYtooSmall = displayHeightPx * m_ScaleFactorMMPerDisplayUnit > m_MaxWorldViewPercentage * worldHeightMM;
  bool zoomYtooBig = displayHeightPx * m_ScaleFactorMMPerDisplayUnit < m_MinWorldViewPercentage * worldHeightMM;

  // constrain zooming in both direction
  if ( zoomXtooBig && zoomYtooBig)
  {
    double fx = worldWidthMM * m_MinWorldViewPercentage / displayWidthPx;
    double fy = worldHeightMM * m_MinWorldViewPercentage / displayHeightPx;
    newScaleFactor = fx < fy ? fx : fy;
    correctZooming = true;
  }
  // constrain zooming in x direction
  else if ( zoomXtooBig )
  {
    newScaleFactor = worldWidthMM * m_MinWorldViewPercentage / displayWidthPx;
    correctZooming = true;
  }

  // constrain zooming in y direction
  else if ( zoomYtooBig )
  {
    newScaleFactor = worldHeightMM * m_MinWorldViewPercentage / displayHeightPx;
    correctZooming = true;
  }

  // constrain zooming out
  // we stop zooming out at these situations:
  //
  // *** display
  // --- image
  //
  //   **********************
  //   *                    *     x side maxed out
  //   *                    *
  //   *--------------------*
  //   *|                  |*
  //   *|                  |*
  //   *--------------------*
  //   *                    *
  //   *                    *
  //   *                    *
  //   **********************
  //
  //   **********************
  //   *     |------|       *     y side maxed out
  //   *     |      |       *
  //   *     |      |       *
  //   *     |      |       *
  //   *     |      |       *
  //   *     |      |       *
  //   *     |      |       *
  //   *     |      |       *
  //   *     |------|       *
  //   **********************
  //
  // In both situations we center the not-maxed out direction
  //
  if ( zoomXtooSmall && zoomYtooSmall )
  {
    // determine and set the bigger scale factor
    float fx = worldWidthMM * m_MaxWorldViewPercentage / displayWidthPx;
    float fy = worldHeightMM * m_MaxWorldViewPercentage / displayHeightPx;
    newScaleFactor = fx > fy ? fx : fy;

    correctZooming = true;
  }

  // actually execute correction
  if (correctZooming)
  {
    SetScaleFactor(newScaleFactor);
  }

  displayWidthMM  = m_SizeInDisplayUnits[0] * m_ScaleFactorMMPerDisplayUnit;
  displayHeightMM = m_SizeInDisplayUnits[1] * m_ScaleFactorMMPerDisplayUnit;

  // constrain panning
  if(worldWidthMM<displayWidthMM)
  {
    // zoomed out too much in x (but tolerated because y is still ok)
    // --> center x
    newOrigin[0] = (worldWidthMM - displayWidthMM) / 2.0;
    correctPanning = true;
  }
  else
  {
    // make sure left display border inside our world
    if (displayXMM < 0)
    {
      newOrigin[0] = 0;
      correctPanning = true;
    }
    // make sure right display border inside our world
    else  if (displayXMM + displayWidthMM > worldWidthMM)
    {
      newOrigin[0] = worldWidthMM - displayWidthMM;
      correctPanning = true;
    }
  }

  if (worldHeightMM<displayHeightMM)
  {
    // zoomed out too much in y (but tolerated because x is still ok)
    // --> center y
    newOrigin[1] = (worldHeightMM - displayHeightMM) / 2.0;
    correctPanning = true;
  }
  else
  {
    // make sure top display border inside our world
    if (displayYMM + displayHeightMM >  worldHeightMM)
    {
      newOrigin[1] = worldHeightMM - displayHeightMM;
      correctPanning = true;
    }
    // make sure bottom display border inside our world
    else
      if (displayYMM < 0)
      {
        newOrigin[1] = 0;
        correctPanning = true;
      }
  }

  if (correctPanning)
  {
    SetOriginInMM( newOrigin );
  }

  inRecalculate = false;

  if ( correctPanning || correctZooming )
  {
    Modified();
  }

  // return true if any correction has been made
  return correctPanning || correctZooming;
}

void mitk::DisplayGeometry::PrintSelf(std::ostream& os, itk::Indent indent) const
{
  if(m_WorldGeometry.IsNull())
  {
    os << indent << " WorldGeometry: " << "NULL" << std::endl;
  }
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
