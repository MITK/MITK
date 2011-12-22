/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-04-18 20:20:25 +0200 (Sa, 18 Apr 2009) $
Version:   $Revision: 13129 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef MITKPLANARFIGUREINTERACTOR_H_HEADER_INCLUDED
#define MITKPLANARFIGUREINTERACTOR_H_HEADER_INCLUDED

#include "PlanarFigureExports.h"

#include "mitkCommon.h"
#include "mitkVector.h"
#include "mitkInteractor.h"
#include "mitkBaseRenderer.h"

#pragma GCC visibility push(default)
#include <itkEventObject.h>
#pragma GCC visibility pop

namespace mitk
{

class DataNode;
class Geometry2D;
class DisplayGeometry;
class PlanarFigure;
class PositionEvent;

#pragma GCC visibility push(default)

// Define events for PlanarFigure interaction notifications
itkEventMacro( PlanarFigureEvent, itk::AnyEvent );
itkEventMacro( StartPlacementPlanarFigureEvent, PlanarFigureEvent );
itkEventMacro( EndPlacementPlanarFigureEvent, PlanarFigureEvent );
itkEventMacro( SelectPlanarFigureEvent, PlanarFigureEvent );
itkEventMacro( StartInteractionPlanarFigureEvent, PlanarFigureEvent );
itkEventMacro( EndInteractionPlanarFigureEvent, PlanarFigureEvent );
itkEventMacro( StartHoverPlanarFigureEvent, PlanarFigureEvent );
itkEventMacro( EndHoverPlanarFigureEvent, PlanarFigureEvent );

#pragma GCC visibility pop


/**
  * \brief Interaction with mitk::PlanarFigure objects via control-points
  *
  * \ingroup Interaction
  */
class PlanarFigure_EXPORT PlanarFigureInteractor : public Interactor
{
public:
  mitkClassMacro(PlanarFigureInteractor, Interactor);
  mitkNewMacro3Param(Self, const char *, DataNode *, int);
  mitkNewMacro2Param(Self, const char *, DataNode *);

  /** \brief Sets the amount of precision */
  void SetPrecision( ScalarType precision );

  /** \brief Sets the minimal distance between two control points. */
  void SetMinimumPointDistance( ScalarType minimumDistance );

  /**
    * \brief calculates how good the data, this statemachine handles, is hit
    * by the event.
    *
    * overwritten, cause we don't look at the boundingbox, we look at each point
    */
  virtual float CanHandleEvent(StateEvent const *stateEvent) const;


protected:
  /**
    * \brief Constructor with Param n for limited Set of Points
    *
    * if no n is set, then the number of points is unlimited*
    */
  PlanarFigureInteractor(const char *type, 
    DataNode *dataNode, int n = -1);

  /**
    * \brief Default Destructor
    **/
  virtual ~PlanarFigureInteractor();

  virtual bool ExecuteAction( Action *action, 
    mitk::StateEvent const *stateEvent );

  /**
    \brief Used when clicking to determine if a point is too close to the previous point.
    */
  bool IsMousePositionAcceptableAsNewControlPoint( const PositionEvent*, const PlanarFigure* );

  bool TransformPositionEventToPoint2D( const StateEvent *stateEvent,
    Point2D &point2D,
    const Geometry2D *planarFigureGeometry );

  bool TransformObjectToDisplay( const mitk::Point2D &point2D,
    mitk::Point2D &displayPoint,
    const mitk::Geometry2D *objectGeometry,
    const mitk::Geometry2D *rendererGeometry,
    const mitk::DisplayGeometry *displayGeometry ) const;

  /** \brief Returns true if the first specified point is in proximity of the line defined
   * the other two point; false otherwise.
   *
   * Proximity is defined as the rectangle around the line with pre-defined distance
   * from the line. */
  bool IsPointNearLine( const mitk::Point2D& point,
    const mitk::Point2D& startPoint, const mitk::Point2D& endPoint, mitk::Point2D& projectedPoint ) const;

  /** \brief Returns true if the point contained in the passed event (in display coordinates)
   * is over the planar figure (with a pre-defined tolerance range); false otherwise. */
  int IsPositionOverFigure(
    const StateEvent *StateEvent, PlanarFigure *planarFigure,
    const Geometry2D *planarFigureGeometry,
    const Geometry2D *rendererGeometry,
    const DisplayGeometry *displayGeometry,
    Point2D& pointProjectedOntoLine) const;

  /** \brief Returns the index of the marker (control point) over which the point contained
   * in the passed event (in display coordinates) currently is; -1 if the point is not over
   * a marker. */
  int IsPositionInsideMarker(
    const StateEvent *StateEvent, const PlanarFigure *planarFigure,
    const Geometry2D *planarFigureGeometry,
    const Geometry2D *rendererGeometry,
    const DisplayGeometry *displayGeometry ) const;

  void LogPrintPlanarFigureQuantities( const PlanarFigure *planarFigure );
 
private:

  /** \brief to store the value of precision to pick a point */
  ScalarType m_Precision;

  /** \brief Store the minimal distance between two control points. */
  ScalarType m_MinimumPointDistance;

  /** \brief True if the mouse is currently hovering over the image. */
  bool m_IsHovering;

  bool m_LastPointWasValid;
};

}

#endif // MITKPLANARFIGUREINTERACTOR_H_HEADER_INCLUDED
