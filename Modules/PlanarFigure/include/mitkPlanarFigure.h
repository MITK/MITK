/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPlanarFigure_h
#define mitkPlanarFigure_h

#include <mitkBaseData.h>
#include <mitkCommon.h>
#include <MitkPlanarFigureExports.h>

#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace mitk
{
  class PlaneGeometry;

  /**
   * \brief Base class of geometric figures on a plane, such as lines, circles and polygons.
   *
   * A figure is defined by its control points in the 2D coordinates of its
   * plane, in mm. Its contours, decorations, handles and measurements are
   * computed from them on demand, so a figure that does not change can be read
   * concurrently. Every change calls Modified().
   *
   * Figure types check their constraints when control points are set and keep
   * them when handles are moved. Points are not restricted to the bounds of the
   * plane; that is a matter of the interaction.
   *
   * \sa CreatePlanarFigure(), PlanarFigureInteractor, PlanarFigureVtkMapper2D
   */
  class MITKPLANARFIGURE_EXPORT PlanarFigure : public BaseData
  {
  public:
    mitkClassMacro(PlanarFigure, BaseData);
    itkCloneMacro(Self);

    /** \brief Kind of a measurement of a planar figure. Its unit follows from the kind. */
    enum class MeasurementType
    {
      Length,
      Circumference,
      Area,
      Angle,
      Radius,
      Diameter,
      MajorAxis,
      MinorAxis,
      Thickness,
      LongAxis,
      ShortAxis
    };

    /** \brief A measured value in the unit of its type, see GetUnitSymbol(). */
    struct Measurement
    {
      MeasurementType Type;
      double Value;
    };

    /** \brief Role of a contour within its figure. */
    enum class ContourRole
    {
      Outline,
      Hole
    };

    /** \brief A polyline of a figure in 2D plane coordinates (mm). */
    struct Contour
    {
      ContourRole Role;
      bool Closed;
      std::vector<Point2D> Points;
    };

    /** \brief Construction line that is not part of the shape, e.g. the control polygon of a Bezier curve. */
    struct HelperPolyline
    {
      std::vector<Point2D> Points;
    };

    /** \brief Marks the angle at a vertex between two arms. Its radius is up to the display. */
    struct AngleArc
    {
      Point2D Vertex;
      Point2D End1;
      Point2D End2;
    };

    /** \brief Head of an arrow at its tip. Its size is up to the display. */
    struct ArrowHead
    {
      Point2D Tip;
      Point2D Tail;
    };

    /** \brief Visual aid of a figure that is not part of its measured shape. */
    using Decoration = std::variant<HelperPolyline, AngleArc, ArrowHead>;

    /** \brief Meaning of a handle for editing. */
    enum class HandleRole
    {
      Vertex, /**< A point of the shape. */
      Center, /**< The center of the shape; moving it moves the whole figure. */
      Extent  /**< Defines the size of the shape in one direction. */
    };

    /** \brief A point at which the figure is edited, in 2D plane coordinates (mm). */
    struct Handle
    {
      HandleRole Role;
      Point2D Position;
    };

    /** \brief How a figure is placed. */
    enum class PlacementKind
    {
      Span,    /**< Defined by two points, e.g. by pressing and dragging, see SetFromSpan(). */
      Sequence /**< Defined by placing its control points one after another. */
    };

    /** \brief Name of a measurement type for display, e.g. "Major Axis". */
    static std::string GetDisplayName(MeasurementType type);

    /** \brief Unit of a measurement type: "mm", "mm2" or "deg". */
    static std::string GetUnitSymbol(MeasurementType type);

    /** \brief Value and typeset unit for display, e.g. "12.34 mm²" or "45.00°", in UTF-8. */
    static std::string FormatValue(const Measurement &measurement);

    /** \brief Stable identifier of the figure type for file formats and interfaces, e.g. "four_point_angle". */
    virtual std::string GetTypeId() const = 0;

    /**
     * \brief Sets the plane the figure lies on.
     *
     * The figure keeps a copy of the plane, which defines the coordinate frame
     * of the control points.
     *
     * \throw mitk::Exception if the plane is null, curved or has no extent.
     */
    void SetPlaneGeometry(const PlaneGeometry *geometry);

    /** \brief The plane the figure lies on. */
    const PlaneGeometry *GetPlaneGeometry() const;

    /**
     * \brief Takes a time geometry of a single time step that holds a flat plane.
     *
     * \throw mitk::Exception for any other geometry.
     */
    void SetTimeGeometry(TimeGeometry *geometry) override;

    using Superclass::SetClonedGeometry;

    /** \brief Takes a copy of a flat plane for time step 0. \throw mitk::Exception for any other geometry or time step. */
    void SetClonedGeometry(const BaseGeometry *geometry, TimeStepType time) override;

    /** \throw mitk::Exception if the time geometry of the data is not one a figure can take. */
    void CopyInformation(const itk::DataObject *data) override;

    /** \brief The control points in 2D plane coordinates (mm). */
    std::vector<Point2D> GetControlPoints() const;

    std::size_t GetNumberOfControlPoints() const;

    /** \throw mitk::Exception if the index is out of range. */
    Point2D GetControlPoint(std::size_t index) const;

    /** \brief A control point in world coordinates. \throw mitk::Exception if the index is out of range. */
    Point3D GetWorldControlPoint(std::size_t index) const;

    /**
     * \brief Replaces all control points.
     *
     * \throw mitk::Exception if the number of points is outside of the range of
     *        the figure type (while being placed, only the maximum applies), if
     *        a coordinate is not finite, or if the points violate a constraint
     *        of the figure type.
     */
    void SetControlPoints(const std::vector<Point2D> &points);

    /** \brief The number of control points a complete figure has at least. */
    virtual std::size_t GetMinimumNumberOfControlPoints() const = 0;

    /** \brief The number of control points a figure has at most. */
    virtual std::size_t GetMaximumNumberOfControlPoints() const = 0;

    /** \brief Whether control points can be inserted into and removed from a placed figure. */
    virtual bool CanInsertControlPoints() const;

    /**
     * \brief Inserts a control point before the given index, or appends it at the end.
     *
     * Allowed for figures that can insert control points, and for appending
     * while a figure is being placed.
     *
     * \throw mitk::Exception if inserting is not allowed, the index is out of
     *        range or SetControlPoints() would not take the resulting points.
     */
    void InsertControlPoint(std::size_t index, const Point2D &point);

    /**
     * \brief Removes a control point.
     *
     * Allowed for figures that can insert control points, and for any figure
     * while it is being placed.
     *
     * \throw mitk::Exception if removing is not allowed, the index is out of
     *        range or SetControlPoints() would not take the resulting points,
     *        e.g. because a figure that is not being placed would fall below
     *        its minimum number of points.
     */
    void RemoveControlPoint(std::size_t index);

    /** \brief The handles of the figure. By default, every control point is a vertex handle. */
    virtual std::vector<Handle> GetHandles() const;

    /**
     * \brief Moves a handle and keeps the figure consistent, e.g. the axes of
     *        an ellipse perpendicular or the center of a circle in the center.
     *
     * Positions are not restricted to the bounds of the plane.
     *
     * \throw mitk::Exception if the index is out of range.
     */
    virtual void MoveHandle(std::size_t index, const Point2D &position);

    /** \brief Moves all control points by an offset. */
    void Translate(const Vector2D &offset);

    /** \brief How the figure is placed. */
    virtual PlacementKind GetPlacementKind() const = 0;

    /**
     * \brief Defines the whole figure by two points, as placing it by pressing and dragging does.
     *
     * \throw mitk::Exception for figures that are not placed by a span.
     */
    virtual void SetFromSpan(const Point2D &start, const Point2D &end);

    /** \brief Marks the figure as being placed, e.g. by an interactor. */
    void BeginPlacement();

    /**
     * \brief Marks the placement of the figure as finished.
     *
     * \throw mitk::Exception if the figure has fewer than its minimum number of
     *        control points; it is still being placed then.
     */
    void EndPlacement();

    /** \brief Whether the figure is being placed. */
    bool IsBeingPlaced() const;

    /** \brief Whether the figure has been placed completely and has at least the minimum number of control points. */
    bool IsComplete() const;

    /** \brief Sets whether the shape of the figure is locked, so that editing may only move it. */
    void SetShapeLocked(bool locked);

    /** \brief Whether the shape of the figure is locked, so that editing may only move it. */
    bool IsShapeLocked() const;

    /** \brief Whether the outline of a complete figure is closed. */
    virtual bool IsClosed() const = 0;

    /** \brief The contours of the figure, computed from its control points. */
    virtual std::vector<Contour> GetContours() const = 0;

    /** \brief The decorations of the figure, computed from its control points. None by default. */
    virtual std::vector<Decoration> GetDecorations() const;

    /** \brief The measurements of the figure, computed from its control points in world coordinates. */
    virtual std::vector<Measurement> GetMeasurements() const = 0;

    /** \brief A single measurement, if the figure has one of this type. */
    std::optional<double> GetMeasurement(MeasurementType type) const;

    /** \brief A figure without control points is empty; its plane does not count for bounding boxes, for example. */
    bool IsEmptyTimeStep(TimeStepType timeStep) const override;

    void UpdateOutputInformation() override;
    void SetRequestedRegionToLargestPossibleRegion() override;
    bool RequestedRegionIsOutsideOfTheBufferedRegion() override;
    bool VerifyRequestedRegion() override;
    void SetRequestedRegion(const itk::DataObject *data) override;

  protected:
    PlanarFigure();
    PlanarFigure(const Self &other) = default;

    /**
     * \brief Checks control points before SetControlPoints() takes them.
     *
     * The default checks the number of points and that their coordinates are
     * finite. Figure types with constraints between their points extend it.
     *
     * \throw mitk::Exception if the points are not valid for the figure type.
     */
    virtual void CheckControlPoints(const std::vector<Point2D> &points) const;

    /** \brief Takes control points without any check, for figure types that build them. */
    void AssignControlPoints(const std::vector<Point2D> &points);

    /** \brief Sets one control point without any check, for figure types that move handles. */
    void ReplaceControlPoint(std::size_t index, const Point2D &point);

    /**
     * \brief Compares what defines the shape beyond the control points, e.g. the
     *        thickness of a double ellipse. Nothing by default.
     *
     * Called by mitk::Equal() for figures of the same class only.
     */
    virtual bool EqualShape(const PlanarFigure &other, ScalarType eps, bool verbose) const;

    void InitializeTimeGeometry(TimeStepType timeSteps = 1) override;

    void PrintSelf(std::ostream &os, itk::Indent indent) const override;

  private:
    // Not implemented, which prevents an itk::Object from being created by PlanarFigure::New().
    static Pointer New();

    friend MITKPLANARFIGURE_EXPORT bool Equal(const PlanarFigure &, const PlanarFigure &, ScalarType, bool);

    std::vector<Point2D> m_ControlPoints;
    bool m_IsBeingPlaced = false;
    bool m_IsShapeLocked = false;
  };

  /**
   * \brief Compares two planar figures: their class, whether they are being
   *        placed or locked, their planes, their control points and their
   *        shape parameters.
   *
   * Derived data like contours and measurements follows from these.
   *
   * \param[in] leftHandSide  The first figure.
   * \param[in] rightHandSide The second figure.
   * \param[in] eps           Tolerance for coordinates and parameters.
   * \param[in] verbose       If true, differences are logged.
   */
  MITKPLANARFIGURE_EXPORT bool Equal(const PlanarFigure &leftHandSide,
                                     const PlanarFigure &rightHandSide,
                                     ScalarType eps,
                                     bool verbose);

  /**
   * \brief Creates an empty planar figure of a type.
   *
   * \param[in] typeId Type identifier, see PlanarFigure::GetTypeId().
   * \throw mitk::Exception for an unknown type identifier.
   */
  MITKPLANARFIGURE_EXPORT PlanarFigure::Pointer CreatePlanarFigure(const std::string &typeId);
}

#endif
