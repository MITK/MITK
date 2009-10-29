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


  /** \brief True if the planar figure has been placed (and can be
   * displayed/interacted with). */
  virtual bool IsPlaced() const { return m_FigurePlaced; };


  /** \brief Sets the 2D geometry on which this figure will be placed.
   *
   * In most cases, this is a Geometry already owned by another object, e.g.
   * describing the slice of the image on which measurements will be
   * performed.
   */
  virtual void SetGeometry2D( mitk::Geometry2D *geometry );


  /** \brief Returns (previously set) 2D geometry of this figure. */
  virtual const Geometry2D *GetGeometry2D() const;


  /** \brief Place figure at the given point (in 2D index coordinates) onto
   * the given 2D geometry.
   *
   * By default, the first two control points of the figure are set to the
   * passed point. Further points can be set via AddControlPoint(), if the
   * current number of control points is below the maximum number of control
   * points.
   *
   * Can be re-implemented in sub-classes as needed.
   */
  virtual void PlaceFigure( const Point2D &point );


  virtual bool AddControlPoint( const Point2D &point );

  virtual bool SetControlPoint( unsigned int index, const Point2D &point );

  virtual bool SetCurrentControlPoint( const Point2D &point );


  
  /** \brief Returns the current number of 2D control points defining this figure. */
  unsigned int GetNumberOfControlPoints() const;


  /** \brief Returns the minimum number of control points needed to represent
   * this figure.
   *
   * Must be implemented in sub-classes.
   */
  virtual unsigned int GetMinimumNumberOfControlPoints() const = 0;


  /** \brief Returns the maximum number of control points allowed for
  * this figure (e.g. 3 for triangles).
  *
  * Must be implemented in sub-classes.
  */
  virtual unsigned int GetMaximumNumberOfControlPoints() const = 0;


  /** \brief Selects currently active control points. */
  virtual bool SelectControlPoint( unsigned int index );

  
  /** \brief Deselect control point; no control point active. */
  virtual void DeselectControlPoint();


  /** \brief Return currently selected control point. */
  virtual int GetSelectedControlPoint() const { return m_SelectedControlPoint; }


  
  /** \brief Returns 2D control points vector. */
  const VertexContainerType *GetControlPoints() const;

  
  /** \brief Returns 2D control points vector. */
  VertexContainerType *GetControlPoints();


  /** \brief Returns specified control point in (2D) index coordinates. */
  Point2D GetControlPoint( unsigned int index ) const;

  /** \brief Returns specified control point in world coordinates. */
  Point3D GetWorldControlPoint( unsigned int index ) const;

  /** \brief Returns specified control point in 2D world coordinates. */
  Point2D GetWorldControlPoint2D( unsigned int index ) const;

  /** \brief Returns the polyline representing the planar figure
   * (for rendering, measurements, etc.). */
  const VertexContainerType *GetPolyLine();


  /** \brief Returns the number of features available for this PlanarFigure
   * (such as, radius, area, ...). */
  unsigned int GetNumberOfFeatures() const;


  /** \brief Returns the name (identifier) of the specified features. */
  const char *GetFeatureName( unsigned int index ) const;


  /** \brief Returns the physical unit of the specified features. */
  const char *GetFeatureUnit( unsigned int index ) const;


  /** Returns quantity of the specified feature (e.g., length, radius,
   * area, ... ) */
  double GetQuantity( unsigned int index ) const;


  /** \brief Calculates quantities of all features of this planar figure. */
  virtual void EvaluateFeatures();


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

  /** Adds feature (e.g., circumference, radius, angle, ...) to feature vector
   * of a planar figure object and returns integer ID for the feature element.
   * Should be called in sub-class constructors. */
  virtual unsigned int AddFeature( const char *featureName, const char *unitName );

  /** Sets quantity of the specified feature. INTERNAL METHOD. */
  void SetQuantity( unsigned int index, double quantity );

  /** \brief Generates the poly-line representation of the planar figure.
   * Must be implemented in sub-classes. */
  virtual void GeneratePolyLine() = 0;

  /** \brief Calculates quantities of all features of this planar figure.
   * Must be implemented in sub-classes. */
  virtual void EvaluateFeaturesInternal() = 0;

  /** \brief Initializes the TimeSlicedGeometry describing the (time-resolved)
   * geometry of this figure. Note that each time step holds one Geometry2D. */
  virtual void InitializeTimeSlicedGeometry( unsigned int timeSteps = 1 );

  virtual void PrintSelf( std::ostream &os, itk::Indent indent ) const;


  VertexContainerType::Pointer m_ControlPoints;

  VertexContainerType::Pointer m_PolyLine;

  bool m_FigurePlaced;

  // Currently selected control point; -1 means no point selected
  int m_SelectedControlPoint;


private:

  struct Feature
  {
    Feature( const char *name, const char *unit ) : Name( name ), Unit( unit)
    {
    };

    std::string Name;
    std::string Unit;
    double Quantity;
  };

  Geometry2D *m_Geometry2D;


  // Vector of features available for this geometric figure
  typedef std::vector< Feature > FeatureVectorType;
  FeatureVectorType m_Features;

  unsigned long m_FeaturesMTime;

};

} // namespace mitk

#endif //_MITK_PLANAR_FIGURE_H_
