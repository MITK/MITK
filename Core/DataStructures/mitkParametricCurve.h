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


#ifndef MITKPARAMETRICCURVE_H_HEADER_INCLUDED
#define MITKPARAMETRICCURVE_H_HEADER_INCLUDED

#include "mitkBaseData.h"


namespace mitk {

/**
 * \brief Abstract base class for parametric curves
 */
class ParametricCurve : public BaseData
{

public:
  mitkClassMacro( ParametricCurve, BaseData );

  static const unsigned int PointDimension = 3;

  typedef Point3D PointType;

  /** Evaluates the curve at parameter t for specified time step. */
  virtual PointType Evaluate( ScalarType t, unsigned int timeStep = 0 ) = 0;

  virtual ScalarType GetRangeMin( unsigned int timeStep = 0 ) const = 0;
  virtual ScalarType GetRangeMax( unsigned int timeStep = 0 ) const = 0;

  /** True if the represented curve is closed. To be implemented by subclasses. */
  virtual bool IsClosed() const = 0;

  /** True if the object holds a valid, displayable, non-empty curve at the
   * given time-step. To be implemented by subclasses. */
  virtual bool IsValidCurve( int timeStep = 0 ) const = 0;

  /** \brief executes the given Operation */
  virtual void ExecuteOperation( Operation* operation );


  // virtual methods that need to be implemented
  virtual void UpdateOutputInformation();
  virtual void SetRequestedRegionToLargestPossibleRegion();
  virtual bool RequestedRegionIsOutsideOfTheBufferedRegion();
  virtual bool VerifyRequestedRegion();
  virtual void SetRequestedRegion( itk::DataObject *data );
  virtual bool WriteXMLData( XMLWriter &xmlWriter );
  virtual bool ReadXMLData( XMLReader &xmlReader );


protected:
  ParametricCurve();

  virtual ~ParametricCurve();
  
  virtual void InitializeTimeSlicedGeometry( unsigned int timeSteps );

};

} // namespace mitk

#endif /* MITKPARAMETRICCURVE_H_HEADER_INCLUDED */
