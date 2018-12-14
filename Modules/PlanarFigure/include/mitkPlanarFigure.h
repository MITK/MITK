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

#ifndef _MITK_PLANAR_FIGURE_H_
#define _MITK_PLANAR_FIGURE_H_

#include "mitkBaseData.h"
#include "mitkCommon.h"
#include <MitkPlanarFigureExports.h>

#include <deque>

namespace mitk
{
  class PlaneGeometry;

  /**
   * \brief Base-class for geometric planar (2D) figures, such as
   * lines, circles, rectangles, polygons, etc.
   *
   * \warning Currently does not support time-resolved data handling
   *
   * Behavior and appearance of PlanarFigures are controlled by various properties; for a detailed
   * list of appearance properties see mitk::PlanarFigureMapper2D
   *
   * The following properties control general PlanarFigure behavior:
   *
   * <ul>
   *   <li>"selected": true if the planar figure is selected
   *   <li>"planarfigure.ishovering": true if the mouse "hovers" over the planar figure
   *   <li>"planarfigure.iseditable": true if the planar figure can be edited (otherwise,
   *       it can only be picked/selected, but its control points cannot be edited); default is true
   *   <li>"planarfigure.isextendable": true if new control points can be inserted into the list of control points;
   *        default is false
   * </ul>
   *
   *
   * TODO: Implement local 2D transform (including center of rotation...)
   *
   */
  class MITKPLANARFIGURE_EXPORT PlanarFigure : public BaseData
  {
  public:
    mitkClassMacro(PlanarFigure, BaseData) itkCloneMacro(Self)

      typedef Point2D PolyLineElement;

    typedef itk::VectorContainer<unsigned long, bool> BoolContainerType;

    typedef std::deque<Point2D> ControlPointListType;
    typedef std::vector<PolyLineElement> PolyLineType;

    /** \brief Sets the 2D geometry on which this figure will be placed.
     *
     * In most cases, this is a Geometry already owned by another object, e.g.
     * describing the slice of the image on which measurements will be
     * performed.
     */
    virtual void SetPlaneGeometry(mitk::PlaneGeometry *geometry);

    /** \brief Returns (previously set) 2D geometry of this figure. */
    virtual const PlaneGeometry *GetPlaneGeometry() const;

    /** \brief True if the planar figure is closed.
     *
     * Default is false. The "closed" boolean property must be set in sub-classes. */
    virtual bool IsClosed() const;

    /** \brief True if the planar figure has been placed (and can be
     * displayed/interacted with). */
    virtual bool IsPlaced() const { return m_FigurePlaced; };
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
    virtual void PlaceFigure(const Point2D &point);

    /**
    * \brief Adds / inserts new control-points
    *
    * This method adds a new control-point with the coordinates defined by point at the given index.
    * If 'index' == -1 or index is greater than the number of control-points the new point is appended
    * to the back of the list of control points.
    * If a control-point already exists for 'index', an additional point is inserted at that position.
    * It is not possible to add more points if the maximum number of control-points (GetMaximumNumberOfControlPoints())
    * has been reached.
    */
    virtual bool AddControlPoint(const Point2D &point, int index = -1);

    virtual bool SetControlPoint(unsigned int index, const Point2D &point, bool createIfDoesNotExist = false);

    virtual bool SetCurrentControlPoint(const Point2D &point);

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
    virtual bool SelectControlPoint(unsigned int index);

    /** \brief Deselect control point; no control point active. */
    virtual bool DeselectControlPoint();

    /** \brief Return currently selected control point. */
    virtual int GetSelectedControlPoint() const { return m_SelectedControlPoint; }
    /** \brief Returns specified control point in 2D world coordinates. */
    Point2D GetControlPoint(unsigned int index) const;

    /**
    * \brief Returns the id of the control-point that corresponds to the given
    * polyline-point.
    */
    virtual int GetControlPointForPolylinePoint(int indexOfPolylinePoint, int polyLineIndex) const;

    /** \brief Returns specified control point in world coordinates. */
    Point3D GetWorldControlPoint(unsigned int index) const;

    /** \brief Returns the polyline representing the planar figure
     * (for rendering, measurements, etc.). */
    const PolyLineType GetPolyLine(unsigned int index);

    /** \brief Returns the polyline representing the planar figure
     * (for rendering, measurments, etc.). */
    const PolyLineType GetPolyLine(unsigned int index) const;

    /** \brief Returns the polyline that should be drawn the same size at every scale
     * (for text, angles, etc.). */
    const PolyLineType GetHelperPolyLine(unsigned int index, double mmPerDisplayUnit, unsigned int displayHeight);

    /** \brief Sets the position of the PreviewControlPoint. Automatically sets it visible.*/
    void SetPreviewControlPoint(const Point2D &point);

    /** \brief Marks the PreviewControlPoint as invisible.*/
    void ResetPreviewContolPoint();

    /** \brief Returns whether or not the PreviewControlPoint is visible.*/
    bool IsPreviewControlPointVisible() const;

    /** \brief Returns the coordinates of the PreviewControlPoint. */
    Point2D GetPreviewControlPoint() const;

    /** \brief Returns the number of features available for this PlanarFigure
     * (such as, radius, area, ...). */
    virtual unsigned int GetNumberOfFeatures() const;

    /** \brief Returns the name (identifier) of the specified features. */
    const char *GetFeatureName(unsigned int index) const;

    /** \brief Returns the physical unit of the specified features. */
    const char *GetFeatureUnit(unsigned int index) const;

    /** Returns quantity of the specified feature (e.g., length, radius,
     * area, ... ) */
    double GetQuantity(unsigned int index) const;

    /** \brief Returns true if the feature with the specified index exists and
    * is active (an inactive feature may e.g. be the area of a non-closed
    * polygon. */
    bool IsFeatureActive(unsigned int index) const;

    /** \brief Returns true if the feature with the specified index exists and is set visible */
    bool IsFeatureVisible(unsigned int index) const;

    /** \brief Defines if the feature with the specified index will be shown as an
    * Annotation in the RenderWindow */
    void SetFeatureVisible(unsigned int index, bool visible);

    /** \brief Calculates quantities of all features of this planar figure. */
    virtual void EvaluateFeatures();

    /** \brief Intherited from parent */
    void UpdateOutputInformation() override;

    /** \brief Intherited from parent */
    void SetRequestedRegionToLargestPossibleRegion() override;

    /** \brief Intherited from parent */
    bool RequestedRegionIsOutsideOfTheBufferedRegion() override;

    /** \brief Intherited from parent */
    bool VerifyRequestedRegion() override;

    /** \brief Intherited from parent */
    void SetRequestedRegion(const itk::DataObject *data) override;

    /** \brief  Returns the current number of polylines  */
    virtual unsigned short GetPolyLinesSize();

    /** \brief  Returns the current number of helperpolylines  */
    virtual unsigned short GetHelperPolyLinesSize() const;

    /** \brief Returns whether a helper polyline should be painted or not */
    virtual bool IsHelperToBePainted(unsigned int index) const;

    /** \brief Returns true if the planar figure is reset to "add points" mode
     * when a point is selected.
     *
     * Default return value is false. Subclasses can overwrite this method and
     * execute any reset / initialization statements required. */
    virtual bool ResetOnPointSelect();
    virtual bool ResetOnPointSelectNeeded() const;

    /** \brief removes the point with the given index from the list of controlpoints. */
    virtual void RemoveControlPoint(unsigned int index);

    /** \brief Removes last control point */
    virtual void RemoveLastControlPoint();

    /** \brief Allow sub-classes to apply constraints on control points.
    *
    * Sub-classes can define spatial constraints to certain control points by
    * overwriting this method and returning a constrained point. By default,
    * the points are constrained by the image bounds. */
    virtual Point2D ApplyControlPointConstraints(unsigned int /*index*/, const Point2D &point);

    /**
    * \brief Compare two PlanarFigure objects
    * Note: all subclasses have to implement the method on their own.
    */
    virtual bool Equals(const mitk::PlanarFigure &other) const;

    /** \brief Set the initial number of control points of the planar figure */
    void ResetNumberOfControlPoints(int numberOfControlPoints);

  protected:
    PlanarFigure();

    PlanarFigure(const Self &other);

    /** Adds feature (e.g., circumference, radius, angle, ...) to feature vector
     * of a planar figure object and returns integer ID for the feature element.
     * Should be called in sub-class constructors. */
    virtual unsigned int AddFeature(const char *featureName, const char *unitName);

    /** Sets the name of the specified feature. INTERNAL METHOD. */
    void SetFeatureName(unsigned int index, const char *featureName);

    /** Sets the physical unit of the specified feature. INTERNAL METHOD. */
    void SetFeatureUnit(unsigned int index, const char *unitName);

    /** Sets quantity of the specified feature. INTERNAL METHOD. */
    void SetQuantity(unsigned int index, double quantity);

    /** Sets the specified feature as active. INTERAL METHOD. */
    void ActivateFeature(unsigned int index);

    /** Sets the specified feature as active. INTERAL METHOD. */
    void DeactivateFeature(unsigned int index);

    /** \brief Generates the poly-line representation of the planar figure.
     * Must be implemented in sub-classes. */
    virtual void GeneratePolyLine() = 0;

    /** \brief Generates the poly-lines that should be drawn the same size regardless of zoom.
   * Must be implemented in sub-classes. */
    virtual void GenerateHelperPolyLine(double mmPerDisplayUnit, unsigned int displayHeight) = 0;

    /** \brief Calculates quantities of all features of this planar figure.
     * Must be implemented in sub-classes. */
    virtual void EvaluateFeaturesInternal() = 0;

    /** \brief Initializes the TimeGeometry describing the (time-resolved)
     * geometry of this figure. Note that each time step holds one PlaneGeometry.
     */
    void InitializeTimeGeometry(unsigned int timeSteps = 1) override;

    /** \brief defines the number of PolyLines that will be available */
    void SetNumberOfPolyLines(unsigned int numberOfPolyLines);

    /** \brief Append a point to the PolyLine # index */
    void AppendPointToPolyLine(unsigned int index, PolyLineElement element);

    /** \brief clears the list of PolyLines. Call before re-calculating a new Polyline. */
    void ClearPolyLines();

    /** \brief defines the number of HelperPolyLines that will be available */
    void SetNumberOfHelperPolyLines(unsigned int numberOfHelperPolyLines);

    /** \brief Append a point to the HelperPolyLine # index */
    void AppendPointToHelperPolyLine(unsigned int index, PolyLineElement element);

    /** \brief clears the list of HelperPolyLines. Call before re-calculating a new HelperPolyline. */
    void ClearHelperPolyLines();

    void PrintSelf(std::ostream &os, itk::Indent indent) const override;

    ControlPointListType m_ControlPoints;
    unsigned int m_NumberOfControlPoints;

    // Currently selected control point; -1 means no point selected
    int m_SelectedControlPoint;

    std::vector<PolyLineType> m_PolyLines;
    std::vector<PolyLineType> m_HelperPolyLines;
    BoolContainerType::Pointer m_HelperPolyLinesToBePainted;

    // this point is used to store the coordiantes an additional 'ControlPoint' that is rendered
    // when the mouse cursor is above the figure (and not a control-point) and when the
    // property 'planarfigure.isextendable' is set to true
    Point2D m_PreviewControlPoint;
    bool m_PreviewControlPointVisible;

    bool m_FigurePlaced;

  private:
    // not implemented to prevent PlanarFigure::New() calls which would create an itk::Object.
    static Pointer New();

    struct Feature
    {
      Feature(const char *name, const char *unit) : Name(name), Unit(unit), Quantity(0.0), Active(true), Visible(true)
      {
      }

      std::string Name;
      std::string Unit;
      double Quantity;
      bool Active;
      bool Visible;
    };

    itk::LightObject::Pointer InternalClone() const override = 0;

    PlaneGeometry *m_PlaneGeometry;

    bool m_PolyLineUpToDate;
    bool m_HelperLinesUpToDate;
    bool m_FeaturesUpToDate;

    // Vector of features available for this geometric figure
    typedef std::vector<Feature> FeatureVectorType;
    FeatureVectorType m_Features;

    unsigned long m_FeaturesMTime;

    // this pair is used to store the mmInDisplayUnits (m_DisplaySize.first) and the displayHeight
    // (m_DisplaySize.second)
    // that the helperPolyLines have been calculated for.
    // It's used to determine whether or not GetHelperPolyLine() needs to recalculate the HelperPolyLines.
    std::pair<double, unsigned int> m_DisplaySize;
  };

  MITKPLANARFIGURE_EXPORT bool Equal(const mitk::PlanarFigure &leftHandSide,
                                     const mitk::PlanarFigure &rightHandSide,
                                     ScalarType eps,
                                     bool verbose);

} // namespace mitk

#endif //_MITK_PLANAR_FIGURE_H_
