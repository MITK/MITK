/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-05-13 18:06:46 +0200 (Mi, 13 Mai 2009) $
Version:   $Revision: 17258 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef _MITK_PLANAR_FIGURE_H_
#define _MITK_PLANAR_FIGURE_H_

#include "mitkCommon.h"
#include "mitkBaseData.h"


namespace mitk 
{

class Geometry2D;

/**
 * \brief Base-class for geometric planar (2D) figures, such as
 * lines, circles, rectangles, polygons, etc.
 *
 * \warning Currently does not support time-resolved data handling
 *
 * TODO: Implement local 2D transform (including center of rotation...)
 *
 */
class MITKEXT_CORE_EXPORT PlanarFigure : public BaseData
{
public:
  mitkClassMacro( PlanarFigure, BaseData );


  typedef itk::VectorContainer< unsigned long, mitk::Point2D > VertexContainerType;


  /** \brief True if the planar figure is closed. Must be implemented in
   * sub-classes. */
  virtual bool IsClosed() const = 0;


  /** \brief Sets the 2D geometry on which this figure will be placed.
   *
   * In most cases, this is a Geometry already owned by another object, e.g.
   * describing the slice of the image on which measurements will be
   * performed.
   */
  virtual void SetGeometry2D( mitk::Geometry2D *geometry );


  /** \brief Place figure in its minimal configuration (a point at least)
   * onto the given 2D geometry.
   *
   * Must be implemented in sub-classes.
   */
  virtual void Initialize() = 0;

  
  /** \brief Returns the number of 2D control points defining this figure. */
  unsigned int GetNumberOfControlPoints() const;

  
  /** \brief Returns 2D control points vector. */
  const VertexContainerType *GetControlPoints() const;

  
  /** \brief Returns 2D control points vector. */
  VertexContainerType *GetControlPoints();


  /** \brief Returns the polyline representing the planar figure
   * (for rendering, measurements, etc.). */
  const VertexContainerType *GetPolyLine();


  /** \brief Intherited from parent */
  virtual void UpdateOutputInformation();

  /** \brief Intherited from parent */
  virtual void SetRequestedRegionToLargestPossibleRegion();

  /** \brief Intherited from parent */
  virtual bool RequestedRegionIsOutsideOfTheBufferedRegion();

  /** \brief Intherited from parent */
  virtual bool VerifyRequestedRegion();

  /** \brief Intherited from parent */
  virtual void SetRequestedRegion(itk::DataObject *data);

protected:
  PlanarFigure();
  virtual ~PlanarFigure();

  /** \brief Generates the poly-line representation of the planar figure.
   * Must be implemented by sub-classes. */
  virtual void GeneratePolyLine() = 0;

  /** \brief Initializes the TimeSlicedGeometry describing the (time-resolved)
   * geometry of this figure. Note that each time step holds one Geometry2D. */
  virtual void InitializeTimeSlicedGeometry( unsigned int timeSteps = 1 );

  virtual void PrintSelf( std::ostream &os, itk::Indent indent ) const;

  VertexContainerType::Pointer m_ControlPoints;

  VertexContainerType::Pointer m_PolyLine;

private:

};

} // namespace mitk

#endif //_MITK_PLANAR_FIGURE_H_
