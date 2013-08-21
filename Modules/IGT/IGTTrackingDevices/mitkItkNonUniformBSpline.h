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

/*===================================================================

This file is based heavily on a corresponding ITK filter.

===================================================================*/

/*********************************
  This file was taken from ITK, CVS version 1.8 to circumvent a bug in ITK release 3.18 (see http://public.kitware.com/Bug/view.php?id=10633
 *********************************/

#ifndef __itkNonUniformBSpline_h
#define __itkNonUniformBSpline_h

#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

#include <vector>

#include "itkPoint.h"
#include "itkObject.h"
#include "itkObjectFactory.h"
#include "itkArray.h"

namespace itk {

/**
 * \class NonUniformBSpline
 * \brief BSpline with nonuniform knot spacing.
 *
 * This class is a bspline with nonuniform knot spacing. The
 * use can specify a set of points and a set of knots and the
 * spline will attempt to find the control points which will
 * cause the spline to interpolate the points.
 *
 * CAUTION: THIS CLASS IS STILL UNDER DEVELOPMENT.
 *
 */

template < unsigned int TDimension = 3 >
class NonUniformBSpline
  : public Object
{
public:
  /**
  Typedefs
  */
  typedef NonUniformBSpline                            Self;
  typedef Object                                       Superclass;
  typedef SmartPointer < Self >                        Pointer;
  typedef SmartPointer < const Self >                  ConstPointer;
  typedef double                                       ScalarType;
  typedef itk::Point< ScalarType, TDimension >         PointType;
  typedef std::vector < PointType >                    PointListType;
  typedef PointListType *                              PointListPointer;
  typedef std::vector < double >                       KnotListType;
  typedef std::vector<double>                          CoordinateListType;
  typedef itk::Point<double, TDimension >              ControlPointType;
  typedef std::vector< ControlPointType >              ControlPointListType;
  typedef ControlPointListType *                       ControlPointListPointer;
  typedef std::vector<double>                          ChordLengthListType;

  /** Method for creation through the object factory. */
  itkNewMacro( Self );

  /** Method for creation through the object factory. */
  itkTypeMacro( NonUniformBSpline, Object );

  /**
   * Set points which the spline will attempt to interpolate.
   */
  void SetPoints( PointListType & newPoints );

  /**
   * Get the points the spline is trying to interpolate.
   */
  const PointListType & GetPoints() const;

  /**
   * Set the knot vector. Knots may be nonuniformly spaced.
   * Knots will be rescaled to be between 0 and 1.
   */
  void SetKnots( KnotListType & newKnots);

  /**
   * Get the knot vector.
   */
  const KnotListType & GetKnots() const;

  /**
   * Computes the chord lengths based on the points.
   */
  void ComputeChordLengths();

  /**
   * Methods for evaluating the spline.
   * The parameterization is always between 0 and 1.
   */
  PointType  EvaluateSpline(const Array<double> & p) const;
  PointType  EvaluateSpline( double t ) const;

  /**
   * Compute the control points.
   */
  void ComputeControlPoints();

  /**
   * Set the control points for the spline.
   */
  void SetControlPoints( ControlPointListType& ctrlpts );

  /**
   * Get the control points for the spline
   */
  const ControlPointListType & GetControlPoints() const;

  /**
   * Evaluate the basis functions directly.
   * order - order of the basis function, i.e. 3 = cubic.
   * i - basis function number, zero based.
   * t - parameter of the spline.
   */
  double NonUniformBSplineFunctionRecursive(unsigned int order, unsigned int i, double t) const;

  /**
   * Set the order of the spline.
   */
  itkSetMacro( SplineOrder, unsigned int );
  itkGetConstReferenceMacro( SplineOrder, unsigned int );


protected:

  /**
   * Constructor
   */
  NonUniformBSpline();

  /**
   * Virtual destructor
   */
  virtual ~NonUniformBSpline();

  /**
   * Method to print the object.
   */
  virtual void PrintSelf( std::ostream& os, Indent indent ) const;

  /**
   * Points that the spline attempts to intepolate.
   */
  PointListType         m_Points;

  /**
   * The knots of spline.
   */
  KnotListType          m_Knots;

  /**
   * The control points of the spline.
   */
  ControlPointListType  m_ControlPoints;

  /**
   * The chord length computed from m_Points.
   */
  ChordLengthListType   m_ChordLength;

  /**
   * The cumulative chord length computed from m_Points
   */
  ChordLengthListType   m_CumulativeChordLength;

  /**
   * The order of the spline.
   */
  unsigned int          m_SplineOrder;

  /**
   * The spatial dimension. Saved from the template parameter.
   */
  unsigned int          m_SpatialDimension;

};

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "mitkItkNonUniformBSpline.txx"
#endif


#endif // __itkNonUniformBSpline_h
