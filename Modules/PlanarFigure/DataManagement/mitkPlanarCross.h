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


#ifndef _MITK_PLANAR_CROSS_H_
#define _MITK_PLANAR_CROSS_H_

#include "mitkPlanarFigure.h"
#include <MitkPlanarFigureExports.h>


namespace mitk
{

class PlaneGeometry;

/**
 * \brief Implementation of PlanarFigure modeling a cross with two orthogonal lines
 * on a plane.
 *
 * The cross consists of two two orthogonal lines, which are defined by four control points
 * lying on a plane. The two control points of the first line are freely placable within
 * the bounds of the underlying 2D geometry, while the two control points of the second line
 * are ensured to meet the following constraints:
 *
 * 1.) The lines must be orthogonal to each other
 * 2.) The second line must lie within the 2D area defined by the first line
 * 3.) The two lines must intersect (at least with their boundaries)
 *
 * When placing the second line interactively, a graphical helper polyline is provided to the
 * user to indicate the position and orthogonal orientation of the line if it would be placed
 * at the current mouse position.
 *
 * When modifying one of the lines by interactively moving its control points, the respective
 * other line is deleted and the user is prompted to draw it again.
 *
 * The class provide a special mode for drawing single lines (SingleLineModeOn/Off); in this
 * case, interaction stops after the first line has been placed.
 *
 * The class provides the lengths of both lines via the "feature" interface, ordered by size.
 *
 * \sa PlanarFigureMapper2D
 */
class MitkPlanarFigure_EXPORT PlanarCross : public PlanarFigure
{
public:
  mitkClassMacro( PlanarCross, PlanarFigure );

  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

  /** \brief Indicates whether the PlanarFigure shall represent only a single line instead of an
   * orthogonal cross. */
  void SetSingleLineMode( bool singleLineMode );

  /** \brief Indicates whether the PlanarFigure shall represent only a single line instead of an
  * orthogonal cross. */
  bool GetSingleLineMode() const;

  /** \brief Indicates whether the PlanarFigure shall represent only a single line instead of an
  * orthogonal cross. */
  itkBooleanMacro( SingleLineMode ); // No need to reimplement; calls SetSingleLineMode()


  /** \brief PlanarCross has either two or four control points, depending on the operation mode. */
  unsigned int GetMinimumNumberOfControlPoints() const
  {
    return this->GetSingleLineMode() ? 2 : 4;
  }

  /** \brief PlanarCross has either two or four control points, depending on the operation mode. */
  unsigned int GetMaximumNumberOfControlPoints() const
  {
    return this->GetSingleLineMode() ? 2 : 4;
  }

  /** \brief The cross shall be reset to a single line when a control point is selected. */
  virtual bool ResetOnPointSelect();


  /** \brief Returns the number of features available for this PlanarCross (1 or 2). */
  virtual unsigned int GetNumberOfFeatures() const;


protected:
  PlanarCross();
  virtual ~PlanarCross();

  mitkCloneMacro(Self);

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
  const unsigned int FEATURE_ID_LONGESTDIAMETER;
  const unsigned int FEATURE_ID_SHORTAXISDIAMETER;

private:

  /** Internal method for applying spatial constraints. */
  virtual Point2D InternalApplyControlPointConstraints( unsigned int index, const Point2D& point );

};

} // namespace mitk

#endif //_MITK_PLANAR_CROSS_H_
