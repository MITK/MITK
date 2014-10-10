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
#ifndef _MITK_CONTOURMODEL_H_
#define _MITK_CONTOURMODEL_H_

#include "mitkCommon.h"
#include <MitkContourModelExports.h>
#include "mitkBaseData.h"

#include <mitkContourElement.h>


namespace mitk
{

  /**
  \brief ContourModel is a structure of linked vertices defining a contour in 3D space.
  The vertices are stored in a mitk::ContourElement is stored for each timestep.
  The contour line segments are implicitly defined by the given linked vertices.
  By default two control points are are linked by a straight line.It is possible to add
  vertices at front and end of the contour and to iterate in both directions.

  Points are specified containing coordinates and additional (data) information,
  see mitk::ContourElement.
  For accessing a specific vertex either an index or a position in 3D Space can be used.
  The vertices are best accessed by using a VertexIterator.
  Interaction with the contour is thus available without any mitk interactor class using the
  api of ContourModel. It is possible to shift single vertices also as shifting the whole
  contour.

  A contour can be either open like a single curved line segment or
  closed. A closed contour can for example represent a jordan curve.

  \section mitkContourModelDisplayOptions Display Options

  The default mappers for this data structure are mitk::ContourModelGLMapper2D and
  mitk::ContourModelMapper3D. See these classes for display options which can
  can be set via properties.
  */
  class MitkContourModel_EXPORT ContourModel : public BaseData
  {

  public:

    mitkClassMacro(ContourModel, BaseData);

    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    /*+++++++++++++++ typedefs +++++++++++++++++++++++++++++++*/
    typedef mitk::ContourElement::VertexType VertexType;
    typedef mitk::ContourElement::VertexListType VertexListType;
    typedef mitk::ContourElement::VertexIterator VertexIterator;
    typedef mitk::ContourElement::ConstVertexIterator ConstVertexIterator;
    typedef std::vector< mitk::ContourElement::Pointer > ContourModelSeries;
    /*+++++++++++++++ END typedefs ++++++++++++++++++++++++++++*/


    /** \brief Possible interpolation of the line segments between control points */
    enum LineSegmentInterpolation{
      LINEAR, B_SPLINE
    };



    /*++++++++++++++++  inline methods  +++++++++++++++++++++++*/

    /** \brief Get the current selected vertex.
    */
    VertexType* GetSelectedVertex()
    {
      return this->m_SelectedVertex;
    }

    /** \brief Deselect vertex.
    */
    void Deselect()
    {
      this->m_SelectedVertex = NULL;
    }

    /** \brief Set selected vertex as control point
    */
    void SetSelectedVertexAsControlPoint(bool isControlPoint=true)
    {
      if (this->m_SelectedVertex)
      {
        m_SelectedVertex->IsControlPoint = isControlPoint;
        this->Modified();
      }
    }

    /** \brief Set the interpolation of the line segments between control points.
    */
    void SetLineSegmentInterpolation(LineSegmentInterpolation interpolation)
    {
      this->m_lineInterpolation = interpolation;
      this->Modified();
    }

    /** \brief Get the interpolation of the line segments between control points.
    */
    LineSegmentInterpolation GetLineSegmentInterpolation()
    {
      return this->m_lineInterpolation;
    }
    /*++++++++++++++++  END inline methods  +++++++++++++++++++++++*/


    /** \brief Add a vertex to the contour at given timestep.
    The vertex is added at the end of contour.

    \param vertex - coordinate representation of a control point
    \param timestep - the timestep at which the vertex will be add ( default 0)

    @Note Adding a vertex to a timestep which exceeds the timebounds of the contour
    will not be added, the TimeGeometry will not be expanded.
    */
    void AddVertex(mitk::Point3D &vertex, int timestep=0);

    /** \brief Add a vertex to the contour at given timestep.
    The vertex is added at the end of contour.

    \param vertex - coordinate representation of a control point
    \param timestep - the timestep at which the vertex will be add ( default 0)

    @Note Adding a vertex to a timestep which exceeds the timebounds of the contour
    will not be added, the TimeGeometry will not be expanded.
    */
    void AddVertex(VertexType &vertex, int timestep=0);

    /** \brief Add a vertex to the contour at given timestep.
    The vertex is added at the end of contour.

    \param vertex - coordinate representation of a control point
    \param timestep - the timestep at which the vertex will be add ( default 0)

    @Note Adding a vertex to a timestep which exceeds the timebounds of the contour
    will not be added, the TimeSlicedGeometry will not be expanded.
    */
    void AddVertex(const VertexType* vertex, int timestep=0);

    /** \brief Add a vertex to the contour.

    \param vertex - coordinate representation of a control point
    \param timestep - the timestep at which the vertex will be add ( default 0)
    \param isControlPoint - specifies the vertex to be handled in a special way (e.g. control points
    will be rendered).


    @Note Adding a vertex to a timestep which exceeds the timebounds of the contour
    will not be added, the TimeGeometry will not be expanded.
    */
    void AddVertex(mitk::Point3D &vertex, bool isControlPoint, int timestep=0);

    /** \brief Add a vertex to the contour at given timestep AT THE FRONT of the contour.
    The vertex is added at the FRONT of contour.

    \param vertex - coordinate representation of a control point
    \param timestep - the timestep at which the vertex will be add ( default 0)

    @Note Adding a vertex to a timestep which exceeds the timebounds of the contour
    will not be added, the TimeGeometry will not be expanded.
    */
    void AddVertexAtFront(mitk::Point3D &vertex, int timestep=0);

    /** \brief Add a vertex to the contour at given timestep AT THE FRONT of the contour.
    The vertex is added at the FRONT of contour.

    \param vertex - coordinate representation of a control point
    \param timestep - the timestep at which the vertex will be add ( default 0)

    @Note Adding a vertex to a timestep which exceeds the timebounds of the contour
    will not be added, the TimeGeometry will not be expanded.
    */
    void AddVertexAtFront(VertexType &vertex, int timestep=0);

    /** \brief Add a vertex to the contour at given timestep AT THE FRONT of the contour.

    \param vertex - coordinate representation of a control point
    \param timestep - the timestep at which the vertex will be add ( default 0)
    \param isControlPoint - specifies the vertex to be handled in a special way (e.g. control points
    will be rendered).


    @Note Adding a vertex to a timestep which exceeds the timebounds of the contour
    will not be added, the TimeGeometry will not be expanded.
    */
    void AddVertexAtFront(mitk::Point3D &vertex, bool isControlPoint, int timestep=0);

    /** \brief Insert a vertex at given index.
    */
    void InsertVertexAtIndex(mitk::Point3D &vertex, int index, bool isControlPoint=false, int timestep=0);

    /** \brief Set a coordinates for point at given index.
    */
    bool SetVertexAt(int pointId, const mitk::Point3D &point, unsigned int timestep=0);

    /** \brief Set a coordinates for point at given index.
    */
    bool SetVertexAt(int pointId, const VertexType* vertex, unsigned int timestep=0);

    /** \brief Return if the contour is closed or not.
    */
    bool IsClosed( int timestep=0) const;

    /** \brief Concatenate two contours.
    The starting control point of the other will be added at the end of the contour.
    \pararm timestep - the timestep at which the vertex will be add ( default 0)
    \pararm check - check for intersections ( default false)
    */
    void Concatenate(mitk::ContourModel* other, int timestep=0, bool check=false);

    /** \brief Returns a const VertexIterator at the start element of the contour.
    @throw mitk::Exception if the timestep is invalid.
    */
    VertexIterator Begin( int timestep=0) const;

    /** \brief Returns a const VertexIterator at the start element of the contour.
    @throw mitk::Exception if the timestep is invalid.
    */
    VertexIterator IteratorBegin( int timestep=0) const;

    /** \brief Returns a const VertexIterator at the end element of the contour.
    @throw mitk::Exception if the timestep is invalid.
    */
    VertexIterator End( int timestep=0) const;

    /** \brief Returns a const VertexIterator at the end element of the contour.
    @throw mitk::Exception if the timestep is invalid.
    */
    VertexIterator IteratorEnd( int timestep=0) const;

    /** \brief Close the contour.
    The last control point will be linked with the first point.
    */
    virtual void Close( int timestep=0);

    /** \brief Set isClosed to false contour.
    The link between the last control point the first point will be removed.
    */
    virtual void Open( int timestep=0);

    /** \brief Set closed property to given boolean.

    false - The link between the last control point the first point will be removed.
    true - The last control point will be linked with the first point.
    */
    virtual void SetClosed(bool isClosed, int timestep=0);

    /** \brief Returns the number of vertices at a given timestep.
    \param timestep - default = 0
    */
    int GetNumberOfVertices( int timestep=0) const;

    /** \brief Returns whether the contour model is empty at a given timestep.
    \pararm timestep - default = 0
    */
    virtual bool IsEmpty( int timestep) const;

    /** \brief Returns whether the contour model is empty.
    */
    virtual bool IsEmpty() const;

    /** \brief Returns the vertex at the index position within the container.
    */
    virtual const VertexType* GetVertexAt(int index, int timestep=0) const;

    /** \brief Remove a vertex at given timestep within the container.

    \return index of vertex. -1 if not found.
    */
    int GetIndex(const VertexType* vertex, int timestep=0);

    /** \brief Check if there isn't something at this timestep.
    */
    virtual bool IsEmptyTimeStep(unsigned int t) const;

    /** \brief Check if mouse cursor is near the contour.
    */
    virtual bool IsNearContour(mitk::Point3D &point, float eps, int timestep);

    /** \brief Mark a vertex at an index in the container as selected.
    */
    bool SelectVertexAt(int index, int timestep=0);

    /** \brief Mark a vertex at an index in the container as control point.
    */
    bool SetControlVertexAt(int index, int timestep=0);

    /** \brief Mark a vertex at a given position in 3D space.

    \param point - query point in 3D space
    \param eps - radius for nearest neighbour search (error bound).
    \param timestep - search at this timestep

    @return true = vertex found;  false = no vertex found
    */
    bool SelectVertexAt(mitk::Point3D &point, float eps, int timestep=0);
/*
    \pararm point - query point in 3D space
    \pararm eps - radius for nearest neighbour search (error bound).
    \pararm timestep - search at this timestep

    @return true = vertex found;  false = no vertex found
    */
    bool SetControlVertexAt(mitk::Point3D &point, float eps, int timestep=0);

    /** \brief Remove a vertex at given index within the container.

    @return true = the vertex was successfuly removed;  false = wrong index.
    */
    bool RemoveVertexAt(int index, int timestep=0);

    /** \brief Remove a vertex at given timestep within the container.

    @return true = the vertex was successfuly removed.
    */
    bool RemoveVertex(const VertexType* vertex, int timestep=0);

    /** \brief Remove a vertex at a query position in 3D space.

    The vertex to be removed will be search by nearest neighbour search.
    Note that possibly no vertex at this position and eps is stored inside
    the contour.

    @return true = the vertex was successfuly removed;  false = no vertex found.
    */
    bool RemoveVertexAt(mitk::Point3D &point, float eps, int timestep=0);

    /** \brief Shift the currently selected vertex by a translation vector.

    \param translate - the translation vector.
    */
    void ShiftSelectedVertex(mitk::Vector3D &translate);

    /** \brief Shift the whole contour by a translation vector at given timestep.

    \param translate - the translation vector.
    \param timestep - at this timestep the contour will be shifted.
    */
    void ShiftContour(mitk::Vector3D &translate, int timestep=0);

    /** \brief Clear the storage container at given timestep.

    All control points are removed at
    timestep.
    */
    virtual void Clear(int timestep);

    /** \brief Initialize all data objects
    */
    virtual void Initialize();

    /** \brief Initialize object with specs of other contour.
    Note: No data will be copied.
    */
    void Initialize(mitk::ContourModel &other);


    /*++++++++++++++++++ method inherit from base data +++++++++++++++++++++++++++*/
    /**
    \brief Inherit from base data - no region support available for contourModel objects.
    */
    virtual void SetRequestedRegionToLargestPossibleRegion ();

    /**
    \brief Inherit from base data - no region support available for contourModel objects.
    */
    virtual bool RequestedRegionIsOutsideOfTheBufferedRegion ();

    /**
    \brief Inherit from base data - no region support available for contourModel objects.
    */
    virtual bool VerifyRequestedRegion ();

    /**
    \brief Get the updated geometry with recomputed bounds.
    */
    virtual const mitk::BaseGeometry* GetUpdatedGeometry (int t=0);

    /**
    \brief Get the BaseGeometry for timestep t.
    */
    virtual mitk::BaseGeometry* GetGeometry (int t=0) const;

    /**
    \brief Inherit from base data - no region support available for contourModel objects.
    */
    virtual void SetRequestedRegion( const itk::DataObject *data);

    /**
    \brief Expand the timebounds of the TimeGeometry to given number of timesteps.
    */
    virtual void Expand( unsigned int timeSteps );

    /**
    \brief Update the OutputInformation of a ContourModel object

    The BoundingBox of the contour will be updated, if necessary.
    */
    virtual void UpdateOutputInformation();

    /**
    \brief Clear the storage container.

    The object is set to initial state. All control points are removed and the number of
    timesteps are set to 1.
    */
    virtual void Clear();

    /**
    \brief overwrite if the Data can be called by an Interactor (StateMachine).
    */
    void ExecuteOperation(Operation* operation);

    /** \brief Redistributes ontrol vertices with a given period (as number of vertices)
    \param period - the number of vertices between control points.
    \param timestep - at this timestep all lines will be rebuilt.
    */
    virtual void RedistributeControlVertices(int period, int timestep);

  protected:
    mitkCloneMacro(Self);

    ContourModel();
    ContourModel(const mitk::ContourModel &other);
    virtual ~ContourModel();

    //inherit from BaseData. called by Clear()
    virtual void ClearData();

    //inherit from BaseData. Initial state of a contour with no vertices and a single timestep.
    virtual void InitializeEmpty();

    //Shift a vertex
    void ShiftVertex(VertexType* vertex, mitk::Vector3D &vector);

    //Storage with time resolved support.
    ContourModelSeries m_ContourSeries;

    //The currently selected vertex.
    VertexType* m_SelectedVertex;

    //The interpolation of the line segment between control points.
    LineSegmentInterpolation m_lineInterpolation;

    //only update the bounding geometry if necessary
    bool m_UpdateBoundingBox;
  };

  itkEventMacro( ContourModelEvent, itk::AnyEvent );
  itkEventMacro( ContourModelShiftEvent, ContourModelEvent );
  itkEventMacro( ContourModelSizeChangeEvent, ContourModelEvent );
  itkEventMacro( ContourModelAddEvent, ContourModelSizeChangeEvent );
  itkEventMacro( ContourModelRemoveEvent, ContourModelSizeChangeEvent );
  itkEventMacro( ContourModelExpandTimeBoundsEvent, ContourModelEvent );
  itkEventMacro( ContourModelClosedEvent, ContourModelEvent );

}
#endif
