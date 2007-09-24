/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision: 11989 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef MITKPARAMETRICSPLINE_H_HEADER_INCLUDED
#define MITKPARAMETRICSPLINE_H_HEADER_INCLUDED

#include "mitkParametricCurve.h"
#include "mitkPointSet.h"

#include <vector>

class vtkCardinalSpline;

namespace mitk {

/**
 * \brief Parametric spline
 */
class ParametricSpline : public ParametricCurve
{

public:
  mitkClassMacro( ParametricSpline, ParametricCurve );

  itkNewMacro( Self );

  /** Evaluates the curve at parameter t for specified time step. */
  virtual PointType Evaluate( ScalarType t, unsigned int timeStep = 0 );

  virtual ScalarType GetRangeMin( unsigned int timeStep = 0 ) const;
  virtual ScalarType GetRangeMax( unsigned int timeStep = 0 ) const;

  /** True if the represented curve is closed. */
  virtual bool IsClosed() const;

  /** True if the object holds a valid, displayable, non-empty curve at the
   * given time step. */
  virtual bool IsValidCurve( int timeStep = 0 ) const;

  /** True if the represented curve is closed */
  itkGetMacro( Closed, bool );

  /** Automatically close the spline (on/off) */
  itkSetMacro( Closed, bool );

  /** Automatically close the spline (on/off) */
  itkBooleanMacro( Closed );

  /** Sets the PointSet holding the splines' control points */
  void SetPointSet( PointSet::Pointer );

  /** Gets the PointSet holding the splines' control points */
  PointSet::Pointer GetPointSet();

  unsigned int GetNumberOfPoints() const;


  /** \brief executes the given Operation */
  virtual void ExecuteOperation(Operation* operation);


  // virtual methods that need to be implemented
  virtual void UpdateOutputInformation();
  virtual void SetRequestedRegionToLargestPossibleRegion();
  virtual bool RequestedRegionIsOutsideOfTheBufferedRegion();
  virtual bool VerifyRequestedRegion();
  virtual void SetRequestedRegion( itk::DataObject *data );
  virtual bool WriteXMLData( XMLWriter &xmlWriter );
  virtual bool ReadXMLData( XMLReader &xmlReader );


protected:
  ParametricSpline();

  virtual ~ParametricSpline();

  /** Called when the underlying mitk::PointSet changes */
  void GenerateSplines();

  void GenerateClosedSplines();
  
  void GenerateNonClosedSplines();
  
  virtual void InitializeTimeSlicedGeometry( unsigned int timeSteps );

  PointSet::Pointer m_PointSet;
  
  typedef itk::SimpleMemberCommand< ParametricSpline > PointSetUpdatedCommand;
  PointSetUpdatedCommand::Pointer m_PointSetUpdatedCommand;

  typedef std::vector< vtkCardinalSpline * > SplineArray;
  SplineArray m_SplinesX, m_SplinesY, m_SplinesZ;

  typedef std::vector< mitk::ScalarType > ScalarArray;
  ScalarArray m_TStart, m_TEnd;

  unsigned int m_NumberOfTimeSteps;
  unsigned int m_NumberOfPoints;

  bool m_Closed;

};

} // namespace mitk

#endif /* MITKPARAMETRICSPLINE_H_HEADER_INCLUDED */
