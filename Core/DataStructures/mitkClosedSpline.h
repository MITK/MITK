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

#ifndef MITKClosedSpline_H_HEADER_INCLUDED
#define MITKClosedSpline_H_HEADER_INCLUDED

#include <mitkPointSet.h>
#include <mitkCommon.h>

class vtkCardinalSpline;

namespace mitk {

class ClosedSpline: public PointSet 
{
public:
  mitkClassMacro(ClosedSpline, PointSet);

  itkNewMacro(Self);

  typedef mitk::ScalarType SplinePositionType;
  typedef Superclass::MeshTraits::CellTraits CellTraits;
  typedef CellTraits::PointIdConstIterator PointIdConstIterator;
  typedef CellTraits::PointIdIterator PointIdIterator;
  
  enum SplineMarkType
  {
    UNSPECIFIED = 3
  };

  //##@brief struct for spline marks
  class SplineMarkerDataType 
  {
    unsigned int id;  //to give the point a special ID
    unsigned int segment; //spline segment number - segments are defined by tow surounding points
    SplinePositionType position; // keeps the actuall position (range:0.0 - 1.0) in the segment
    SplineMarkType type; //keeps type information
    bool selected;  //information about if the point is selected
  };

  //## @brief executes the given Operation
  virtual void ExecuteOperation(Operation* operation);

  // checks if spline exists
  virtual bool SplineExist();

protected:
  ClosedSpline();
  virtual ~ClosedSpline();

  //!@brief sort points in combiniend way
  virtual void DoSortPoints() {};

private:
  /*!@brief This method constructs a spline from the given point list and retrieves
  // a number of interpolated points from it to form a ring-like structure.
  //
  // To make the spline "closed", the end point is connected to the start
  // point. For ensuring smoothness at the start-end-point transition, the
  // (intrinsically non-circular) spline array is extended on both sides
  // by wrapping a number of points from the respective other side.
  //
  // The used VTK filters do principally support this kind of "closed" spline,
  // but it does not produce results as consistent as with the method used
  // here. Also, the spline class of VTK 4.4 has only poor support for
  // arbitrary parametric coordinates (t values in vtkSpline). VTK 5.0 has
  // better support, and also provides a new class vtkParametricSpline for
  // directly calculating 3D splines.
  */
  virtual void CreateSpline();

  // for creating spline with presorting...
  virtual void OnPointSetChange();

  //!@brief Data from vtk which represents the spline
  vtkCardinalSpline* m_SplineX;
  vtkCardinalSpline* m_SplineY;
  vtkCardinalSpline* m_SplineZ;

  //!@brief keeps a list of all splinemarks in dependency of the segments
  SplineMarkerDataType m_SplineMarker;

  //## @brief Spline resolution of created Surface
  unsigned int m_SplineResolution;
};

} // namespace mitk

#endif /* MITKClosedSpline_H_HEADER_INCLUDED */

