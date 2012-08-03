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
  //## @brief Represents a contour element consisting of several control points.
  //## The contour is implicitly defined by the given control points.
  //## Interaction with the contour is available without any interactor
  //## class using the api of ContourModel.
  //## A contour can be either open like a single curved line segment or
  //## closed.
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
    //## @brief
    //##
    VertexType* GetSelectedVertex()
    {
      return this->m_SelectedVertex;
    }


    //##Documentation
    //## @brief
    //##
    void Deselect()
    {
      this->m_SelectedVertex = NULL;
    }


    //##Documentation
    //## @brief
    //##
    void SetLineSegmentInterpolation(LineSegmentInterpolation interpolation)
    {
      this->m_lineInterpolation = interpolation;
    }
/*++++++++++++++++  END inline methods  +++++++++++++++++++++++*/


    //##Documentation
    //## @brief
    //##
    void AddVertex(mitk::Point3D &vertex, unsigned int timestep=0);

    //##Documentation
    //## @brief
    //##
    void AddVertex(mitk::Point3D &vertex, bool isActive, unsigned int timestep=0);

    //##Documentation
    //## @brief
    //##
    bool IsClosed(unsigned int timestep=0);

    //##Documentation
    //## @brief
    //##
    void Concatenate(mitk::ContourModel* other, unsigned int timestep=0);

    //##Documentation
    //## @brief
    //##
    VertexIterator IteratorBegin(unsigned int timestep=0);

    //##Documentation
    //## @brief
    //##
    virtual void Close(unsigned int timestep=0);

    //##Documentation
    //## @brief
    //##
    VertexIterator IteratorEnd( unsigned int timestep=0);

    //##Documentation
    //## @brief
    //##
    int GetNumberOfVertices( unsigned int timestep=0);

    //##Documentation
    //## @brief
    //##
    virtual const VertexType* GetVertexAt(int index, int timestep=0) const;

    //##Documentation
    //## @brief
    //##
    virtual bool IsEmptyTimeStep(unsigned int t) const;

    //##Documentation
    //## @brief
    //##
    bool SelectVertexAt(int index, unsigned int timestep=0);

    //##Documentation
    //## @brief
    //##
    bool SelectVertexAt(mitk::Point3D &point, float eps, unsigned int timestep=0);

    //##Documentation
    //## @brief
    //##
    bool RemoveVertexAt(int index, unsigned int timestep=0);

    //##Documentation
    //## @brief
    //##
    bool RemoveVertexAt(mitk::Point3D &point, float eps, unsigned int timestep=0);

    //##Documentation
    //## @brief
    //##
    void MoveSelectedVertex(mitk::Vector3D &translate);

    //##Documentation
    //## @brief
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
    //## @brief
    //##
    virtual mitk::Geometry3D* GetGeometry (int t=0) const;

    //##Documentation
    //## @brief
    //##
    virtual void SetRequestedRegion (itk::DataObject *data);

    //##Documentation
    //## @brief
    //##
    virtual void Expand( unsigned int timeSteps );

    //##Documentation
    //## @brief
    //##
    virtual void UpdateOutputInformation();

    //##Documentation
    //## @brief
    //##
    virtual void Clear();

  //##Documentation
  //##@brief overwrite if the Data can be called by an Interactor (StateMachine).
  //## 
  //## Empty by default. Overwrite and implement all the necessary operations here 
  //## and get the necessary information from the parameter operation.
  void ExecuteOperation(Operation* operation);

  protected:

    ContourModel();
    ContourModel(const mitk::ContourModel &other);
    virtual ~ContourModel();

    virtual void ClearData();//inherit from BaseData
    virtual void InitializeEmpty();//inherit from BaseData

    void MoveVertex(VertexType* vertex, mitk::Vector3D &vector);


    ContourModelSeries m_ContourSeries;
    VertexType* m_SelectedVertex;
    LineSegmentInterpolation m_lineInterpolation;
  };
}

#endif