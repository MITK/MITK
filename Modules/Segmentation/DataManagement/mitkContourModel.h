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
#include "SegmentationExports.h"
#include "mitkBaseData.h"

#include <mitkContourModelElement.h>


namespace mitk 
{

  //##Documentation
  //## @brief ContourModel is a structure of linked control points defining a contour in 3D space.
  //## Vertices are stored in a VertexList in mitk::ContourModelControl.
  //## For timeresolved purposes a mitk::ContourModelElement is stored for each timestep.
  //## The contour line segments are implicitly defined by the given control points.
  //##
  //## Points are specified as vertices containing coordinates additional (data) information,
  //## see mitk::ContourModelElement.
  //## The vertices are best accessed by using a VertexIterator.
  //## For accessing a specific vertex either an index or a position in 3D Space can be used.
  //## Interaction with the contour is available without any mitk interactor class using the
  //## api of ContourModel. It is possible to shift single vertices also as shifting the whole
  //## contour.
  //##
  //## A contour can be either open like a single curved line segment or
  //## closed. A closed contour can for example represent a jordan curve.
  //## By default two control points are are linked by a straight line. The interpolation
  //## can be set via enum property (mitk::ContourModel::LineSegmentInterpolation).
  //##
  //## \section mitkPointSetDisplayOptions
  //##
  //## The default mappers for this data structure are mitk::ContourModelMapper2D and
  //## mitk::ContourModelMapper3D. See these classes for display options which can
  //## can be set via properties.
  //##
  class Segmentation_EXPORT ContourModel : public BaseData
  {

  public:

    mitkClassMacro(ContourModel, BaseData);

    itkNewMacro(Self);

    mitkCloneMacro(Self);


/*+++++++++++++++ typedefs +++++++++++++++++++++++++++++++*/
    typedef mitk::ContourModelElement::VertexType VertexType;
    typedef mitk::ContourModelElement::VertexListType VertexListType;
    typedef mitk::ContourModelElement::ConstVertexIterator VertexIterator;
    typedef std::vector< mitk::ContourModelElement::Pointer > ContourModelSeries;
/*+++++++++++++++ END typedefs ++++++++++++++++++++++++++++*/


    /** @brief Possible interpolation of the line segments between control points */
    enum LineSegmentInterpolation{
      LINEAR, B_SPLINE
    };



/*++++++++++++++++  inline methods  +++++++++++++++++++++++*/

    //##Documentation
    //## @brief Get the current selected vertex.
    //##
    VertexType* GetSelectedVertex()
    {
      return this->m_SelectedVertex;
    }


    //##Documentation
    //## @brief Deselect vertex.
    //##
    void Deselect()
    {
      this->m_SelectedVertex = NULL;
    }


    //##Documentation
    //## @brief Set the interpolation of the line segments between control points.
    //##
    void SetLineSegmentInterpolation(LineSegmentInterpolation interpolation)
    {
      this->m_lineInterpolation = interpolation;
    }
/*++++++++++++++++  END inline methods  +++++++++++++++++++++++*/


    //##Documentation
    //## @brief Add a vertex to the contour at given timestep.
    //## The vertex is added at the end of contour.
    //##
    //## @param vertex - coordinate representation of a control point
    //## @param timestep - the timestep at which the vertex will be add ( default 0)
    //##
    //## @Note Adding a vertex to a timestep which exceeds the timebounds of the contour
    //## will not be added, the TimeSlicedGeometry will not be expanded.
    //## @Note Also if the contour is closed the vertex will not be added.
    //##
    void AddVertex(mitk::Point3D &vertex, unsigned int timestep=0);

    //##Documentation
    //## @brief Add a vertex to the contour.
    //##
    //## @param vertex - coordinate representation of a control point
    //## @param timestep - the timestep at which the vertex will be add ( default 0)
    //## @param isActive - specifies wether the vertex is active or not (Active 
    //## vertices will be rendered).
    //##
    //##//##
    //## @Note Adding a vertex to a timestep which exceeds the timebounds of the contour
    //## will not be added, the TimeSlicedGeometry will not be expanded.
    //## @Note Also if the contour is closed the vertex will not be added.
    //##
    void AddVertex(mitk::Point3D &vertex, bool isActive, unsigned int timestep=0);

    //##Documentation
    //## @brief Return if the contour is closed or not.
    //##
    bool IsClosed(unsigned int timestep=0);

    //##Documentation
    //## @brief Concatenate two contours.
    //## The starting control point of the other will be added at the end of the contour.
    //##
    void Concatenate(mitk::ContourModel* other, unsigned int timestep=0);

    //##Documentation
    //## @brief Returns a const VertexIterator at the start element of the contour.
    //##
    VertexIterator IteratorBegin(unsigned int timestep=0);

    //##Documentation
    //## @brief Close the contour.
    //## The last control point will be linked with the first point.
    //##
    virtual void Close(unsigned int timestep=0);

    //##Documentation
    //## @brief Returns a const VertexIterator at the end element of the contour.
    //##
    VertexIterator IteratorEnd( unsigned int timestep=0);

    //##Documentation
    //## @brief Returns the number of vertices at a given timestep.
    //##
    //## @param timestep - default = 0
    //##
    int GetNumberOfVertices( unsigned int timestep=0);

    //##Documentation
    //## @brief Returns the vertex at the index position within the container.
    //##
    virtual const VertexType* GetVertexAt(int index, int timestep=0) const;

    //##Documentation
    //## @brief Check if there isn't something at this timestep.
    //##
    virtual bool IsEmptyTimeStep(unsigned int t) const;

    //##Documentation
    //## @brief Mark a vertex at an index in the container as selected.
    //##
    bool SelectVertexAt(int index, unsigned int timestep=0);

    //##Documentation
    //## @brief Mark a vertex at a given position in 3D space.
    //## 
    //## @param point - query point in 3D space
    //## @param eps - radius for nearest neighbour search (error bound).
    //## @param timestep - search at this timestep
    //##
    //## @return true = vertex found;  false = no vertex found
    //##
    bool SelectVertexAt(mitk::Point3D &point, float eps, unsigned int timestep=0);

    //##Documentation
    //## @brief Remove a vertex at given index within the container.
    //##
    //## @return true = the vertex was successfuly removed;  false = wrong index.
    //##
    bool RemoveVertexAt(int index, unsigned int timestep=0);

    //##Documentation
    //## @brief Remove a vertex at a query position in 3D space.
    //##
    //## The vertex to be removed will be search by nearest neighbour search.
    //## Note that possibly no vertex at this position and eps is stored inside
    //## the contour.
    //##
    //## @return true = the vertex was successfuly removed;  false = no vertex found.
    //##
    bool RemoveVertexAt(mitk::Point3D &point, float eps, unsigned int timestep=0);

    //##Documentation
    //## @brief Shift the currently selected vertex by a translation vector.
    //##
    //## @param translate - the translation vector.
    //##
    void MoveSelectedVertex(mitk::Vector3D &translate);

    //##Documentation
    //## @brief Shift the whole contour by a translation vector at given timestep.
    //## 
    //## @param translate - the translation vector.
    //## @param timestep - at this timestep the contour will be shifted.
    //##
    void MoveContour(mitk::Vector3D &translate, unsigned int timestep=0);


/*++++++++++++++++++ method inherit from base data +++++++++++++++++++++++++++*/
    //##Documentation
    //## @brief
    //##
    virtual void SetRequestedRegionToLargestPossibleRegion ();

    //##Documentation
    //## @brief
    //##
    virtual bool RequestedRegionIsOutsideOfTheBufferedRegion ();

    //##Documentation
    //## @brief
    //##
    virtual bool VerifyRequestedRegion ();

    //##Documentation
    //## @brief
    //##
    virtual const mitk::Geometry3D* GetUpdatedGeometry (int t=0);

    //##Documentation
    //## @brief Get the Geometry3D for timestep t.
    //##
    virtual mitk::Geometry3D* GetGeometry (int t=0) const;

    //##Documentation
    //## @brief
    //##
    virtual void SetRequestedRegion (itk::DataObject *data);

    //##Documentation
    //## @brief Expand the timebounds of the TimeSlicedGeometry.
    //##
    virtual void Expand( unsigned int timeSteps );

    //##Documentation
    //## @brief Update the OutputInformation of a ContourModel object
    //##
    //## The BoundingBox of the contour will be updated, if necessary.
    //##
    virtual void UpdateOutputInformation();

    //##Documentation
    //## @brief Clear the storage container.
    //##
    //## The object is set to initial state. All control points are removed and the number of
    //## timesteps are set to 1.
    //##
    virtual void Clear();

  //##Documentation
  //##@brief overwrite if the Data can be called by an Interactor (StateMachine).
  //## 
  //## 
  void ExecuteOperation(Operation* operation);

  protected:

    ContourModel();
    ContourModel(const mitk::ContourModel &other);
    virtual ~ContourModel();

    //inherit from BaseData. called by Clear()
    virtual void ClearData();

    //inherit from BaseData. Initial state of a contour with no vertices and a single timestep.
    virtual void InitializeEmpty();

    //Shift a vertex
    void MoveVertex(VertexType* vertex, mitk::Vector3D &vector);


    //Storage with time resolved support.
    ContourModelSeries m_ContourSeries;

    //The currently selected vertex.
    VertexType* m_SelectedVertex;

    //The interpolation of the line segment between control points.
    LineSegmentInterpolation m_lineInterpolation;
  };
}

#endif