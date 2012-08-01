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

  /**
  * Represents a contour element consisting of several control points.
  *
  */
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



/*++++++++++++++++  inline methods  +++++++++++++++++++++++*/
    void AddVertex(mitk::Point3D &vertex, unsigned int timestep=0)
    {
      this->m_ContourSeries[timestep]->AddVertex(vertex);
    }

    bool IsClosed(unsigned int timestep=0)
    {
      return this->m_ContourSeries[timestep]->IsClosed();
    }

    void Concatenate(mitk::ContourModel* other, unsigned int timestep=0)
    {
      this->m_ContourSeries[timestep]->Concatenate(other->m_ContourSeries[timestep]);
    }

    VertexIterator IteratorBegin(unsigned int timestep=0)
    {
      return this->m_ContourSeries[timestep]->ConstIteratorBegin();
    }

    VertexIterator IteratorEnd( unsigned int timestep=0)
    {
      return this->m_ContourSeries[timestep]->ConstIteratorEnd();
    }

    int GetNumberOfVertices( unsigned int timestep=0)
    {
       return this->m_ContourSeries[timestep]->GetSize();
    }

    VertexType* GetSelectedVertex()
    {
      return this->m_SelectedVertex;
    }

    virtual void Close(unsigned int timestep=0)
    {
      this->m_ContourSeries[timestep]->Close();
    }

    void Deselect()
    {
      this->m_SelectedVertex = NULL;
    }

    virtual const VertexType* GetVertexAt(int index, int timestep=0) const
    {
      return this->m_ContourSeries[timestep]->GetVertexAt(index);
    }

    virtual bool IsEmptyTimeStep(unsigned int t) const
    {
      return this->m_ContourSeries.size() <= t;
    }
/*++++++++++++++++  END inline methods  +++++++++++++++++++++++*/



    bool SelectVertexAt(int index, unsigned int timestep=0);

    bool SelectVertexAt(mitk::Point3D &point, float eps, unsigned int timestep=0);

    bool RemoveVertexAt(int index, unsigned int timestep=0);

    bool RemoveVertexAt(mitk::Point3D &point, float eps, unsigned int timestep=0);

    void MoveSelectedVertex(mitk::Vector3D &translate);

    void MoveContour(mitk::Vector3D &translate, unsigned int timestep=0);


/*++++++++++++++++++ method inherit from base data +++++++++++++++++++++++++++*/
    virtual void SetRequestedRegionToLargestPossibleRegion ();

    virtual bool RequestedRegionIsOutsideOfTheBufferedRegion ();

    virtual bool VerifyRequestedRegion ();

    virtual const mitk::Geometry3D* GetUpdatedGeometry (int t=0);

    virtual mitk::Geometry3D* GetGeometry (int t=0) const;

    virtual void SetRequestedRegion (itk::DataObject *data);

    virtual void Expand( unsigned int timeSteps );

    virtual void UpdateOutputInformation();


  protected:

    ContourModel();
    ContourModel(const mitk::ContourModel &other);
    virtual ~ContourModel();

    void MoveVertex(VertexType* vertex, mitk::Vector3D &vector);


    ContourModelSeries m_ContourSeries;
    VertexType* m_SelectedVertex;
  };
}

#endif