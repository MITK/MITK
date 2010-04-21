/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2010-01-28 18:32:03 +0100 (Do, 28 Jan 2010) $
Version:   $Revision: 17258 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef _MITK_PLANAR_CROSS_H_
#define _MITK_PLANAR_CROSS_H_

#include "mitkPlanarFigure.h"
#include "MitkExtExports.h"


namespace mitk 
{

class Geometry2D;

/**
 * \brief Implementation of PlanarFigure representing a line
 * through two control points
 */
class MitkExt_EXPORT PlanarCross : public PlanarFigure
{
public:
  mitkClassMacro( PlanarCross, PlanarFigure );

  itkNewMacro( Self );

  itkSetMacro( SingleLineMode, bool );
  itkGetMacro( SingleLineMode, bool );
  itkBooleanMacro( SingleLineMode );


  virtual bool IsClosed() const { return false; };

  /** \brief Place figure in its minimal configuration (a point at least)
   * onto the given 2D geometry.
   *
   * Must be implemented in sub-classes.
   */
  //virtual void Initialize();

  
  /** \brief Line has 2 control points per definition. */
  unsigned int GetMinimumNumberOfControlPoints() const
  {
    return m_SingleLineMode ? 2 : 4;
  }


  /** \brief Line has 2 control points per definition. */
  unsigned int GetMaximumNumberOfControlPoints() const
  {
    return m_SingleLineMode ? 2 : 4;
  }


  /** \brief The cross shall be reset to a single line when a control point
   * is selected. */
  virtual bool ResetOnPointSelect();


protected:
  PlanarCross();
  virtual ~PlanarCross();

  /** \brief Spatially constrain control points of second (orthogonal) line */
  virtual Point2D ApplyControlPointConstraints( unsigned int index, const Point2D& point );

  /** \brief Generates the poly-line representation of the planar figure. */
  virtual void GeneratePolyLine();

  /** \brief Generates the poly-lines that should be drawn the same size regardless of zoom.*/
  virtual void GenerateHelperPolyLine(double mmPerDisplayUnit, unsigned int displayHeight);

  /** \brief Calculates feature quantities of the planar figure. */
  virtual void EvaluateFeaturesInternal();


  virtual void PrintSelf( std::ostream &os, itk::Indent indent ) const;

  // Feature identifiers
  const unsigned int FEATURE_ID_LENGTH;


  // TRUE if this object does not represent a cross, but a single line
  bool m_SingleLineMode;


private:

};

} // namespace mitk

#endif //_MITK_PLANAR_CROSS_H_
