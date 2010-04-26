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
#include "MitkExtExports.h"
#include "mitkBaseData.h"
#include <itkSmartPointerVectorContainer.h>


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
class MitkExt_EXPORT PlanarFigure : public BaseData
{
public:
  mitkClassMacro( PlanarFigure, BaseData );


  typedef itk::VectorContainer< unsigned long, mitk::Point2D > VertexContainerType;
  typedef itk::SmartPointerVectorContainer< unsigned long, VertexContainerType> VertexContainerVectorType;
  typedef itk::VectorContainer< unsigned long, bool>  BoolContainerType;


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
  virtual void PlaceFigure( const Point2D& point );


  virtual bool AddControlPoint( const Point2D& point );

  virtual bool SetControlPoint( unsigned int index, const Point2D& point, bool createIfDoesNotExist = false);

  virtual bool SetCurrentControlPoint( const Point2D& point );

   
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


  /** \brief Returns specified control point in 2D world coordinates. */
  Point2D& GetControlPoint( unsigned int index ) const;


  /** \brief Returns specified control point in world coordinates. */
  Point3D GetWorldControlPoint( unsigned int index ) const;


  /** \brief Returns the polyline representing the planar figure
   * (for rendering, measurements, etc.). */
  VertexContainerType *GetPolyLine(unsigned int index);

  /** \brief Returns the polyline representing the planar figure
   * (for rendering, measurements, etc.). */
  const VertexContainerType *GetPolyLine(unsigned int index) const;

  /** \brief Returns the polyline that should be drawn the same size at every scale
   * (for text, angles, etc.). */
  const VertexContainerType *GetHelperPolyLine(unsigned int index, double mmPerDisplayUnit, unsigned int displayHeight);

  /** \brief Returns the number of features available for this PlanarFigure
   * (such as, radius, area, ...). */
  virtual unsigned int GetNumberOfFeatures() const;


  /** \brief Returns the name (identifier) of the specified features. */
  const char *GetFeatureName( unsigned int index ) const;


  /** \brief Returns the physical unit of the specified features. */
  const char *GetFeatureUnit( unsigned int index ) const;


  /** Returns quantity of the specified feature (e.g., length, radius,
   * area, ... ) */
  double GetQuantity( unsigned int index ) const;

  
  /** \brief Returns true if the feature with the specified index exists and
  * is active (an inactive feature may e.g. be the area of a non-closed
  * polygon. */
  bool IsFeatureActive( unsigned int index ) const;


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
 
  /** \brief  Returns the current number of polylines  */
  virtual unsigned short GetPolyLinesSize();

  /** \brief  Returns the current number of helperpolylines  */
  virtual unsigned short GetHelperPolyLinesSize();

  /** \brief Returns whether a helper polyline should be painted or not */
  virtual bool IsHelperToBePainted(unsigned int index);

  /** \brief Returns true if the planar figure is reset to "add points" mode
   * when a point is selected.
   *
   * Default return falue is false. Subclasses can overwrite this method and
   * execute any reset / initialization statements required. */
  virtual bool ResetOnPointSelect();


protected:
  PlanarFigure();
  virtual ~PlanarFigure();

  /** \brief Set the initial number of control points of the planar figure */
  void ResetNumberOfControlPoints( int numberOfControlPoints );

  /** \brief Allow sub-classes to apply constraints on control points.
  *
  * Sub-classes can define spatial constraints to certain control points by
  * overwriting this method and returning a constrained point. By default,
  * the points are constrained by the image bounds. */
  virtual Point2D ApplyControlPointConstraints( unsigned int index, const Point2D& point );

  /** Adds feature (e.g., circumference, radius, angle, ...) to feature vector
   * of a planar figure object and returns integer ID for the feature element.
   * Should be called in sub-class constructors. */
  virtual unsigned int AddFeature( const char *featureName, const char *unitName );

  /** Sets the name of the specified feature. INTERNAL METHOD. */
  void SetFeatureName( unsigned int index, const char *featureName );

  /** Sets the physical unit of the specified feature. INTERNAL METHOD. */
  void SetFeatureUnit( unsigned int index, const char *unitName );

  /** Sets quantity of the specified feature. INTERNAL METHOD. */
  void SetQuantity( unsigned int index, double quantity );

  /** Sets the specified feature as active. INTERAL METHOD. */
  void ActivateFeature( unsigned int index );

  /** Sets the specified feature as active. INTERAL METHOD. */
  void DeactivateFeature( unsigned int index );

  /** \brief Generates the poly-line representation of the planar figure.
   * Must be implemented in sub-classes. */
  virtual void GeneratePolyLine() = 0;

    /** \brief Generates the poly-lines that should be drawn the same size regardless of zoom.
   * Must be implemented in sub-classes. */
  virtual void GenerateHelperPolyLine(double mmPerDisplayUnit, unsigned int displayHeight) = 0;

  /** \brief Calculates quantities of all features of this planar figure.
   * Must be implemented in sub-classes. */
  virtual void EvaluateFeaturesInternal() = 0;

  /** \brief Initializes the TimeSlicedGeometry describing the (time-resolved)
   * geometry of this figure. Note that each time step holds one Geometry2D. */
  virtual void InitializeTimeSlicedGeometry( unsigned int timeSteps = 1 );

  virtual void PrintSelf( std::ostream& os, itk::Indent indent ) const;


  VertexContainerType::Pointer m_ControlPoints;
  unsigned int m_NumberOfControlPoints;

  VertexContainerVectorType::Pointer m_PolyLines;
  VertexContainerVectorType::Pointer m_HelperPolyLines;
  BoolContainerType::Pointer         m_HelperPolyLinesToBePainted;


  bool m_FigurePlaced;

  // Currently selected control point; -1 means no point selected
  int m_SelectedControlPoint;


private:

  struct Feature
  {
    Feature( const char *name, const char *unit ) 
    : Name( name ), Unit( unit ), Quantity( 0.0 ), Active( true )
    {
    };

    std::string Name;
    std::string Unit;
    double Quantity;
    bool Active;
  };

  Geometry2D *m_Geometry2D;


  // Vector of features available for this geometric figure
  typedef std::vector< Feature > FeatureVectorType;
  FeatureVectorType m_Features;

  unsigned long m_FeaturesMTime;

};

} // namespace mitk

#endif //_MITK_PLANAR_FIGURE_H_
