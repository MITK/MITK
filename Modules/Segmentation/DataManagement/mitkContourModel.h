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
/*+++++++++++++++ END typedefs ++++++++++++++++++++++++++++*/



/*++++++++++++++++  inline methods  +++++++++++++++++++++++*/
    void AddVertex(mitk::Point3D &vertex)
    {
      this->m_Contour->AddVertex(vertex);
    }

    bool IsClosed()
    {
      return this->m_Contour->IsClosed();
    }

    void Concatenate(mitk::ContourModel* other)
    {
      this->m_Contour->Concatenate(other->m_Contour);
    }

    VertexIterator IteratorBegin()
    {
      return this->m_Contour->ConstIteratorBegin();
    }

    VertexIterator IteratorEnd()
    {
      return this->m_Contour->ConstIteratorEnd();
    }

    int GetNumberOfVertices()
    {
      return this->m_Contour->GetSize();
    }

    VertexType* GetSelectedVertex()
    {
      return this->m_SelectedVertex;
    }

    virtual void Close()
    {
      this->m_Contour->Close();
    }
/*++++++++++++++++  END inline methods  +++++++++++++++++++++++*/



    bool SelectVertexAt(int index);

    bool SelectVertexAt(mitk::Point3D &point, float eps);

    bool RemoveVertexAt(int index);

    bool RemoveVertexAt(mitk::Point3D &point, float eps);

    void MoveSelectedVertex(mitk::Vector3D &translate);

    void MoveContour(mitk::Vector3D &translate);


/*++++++++++++++++++ method inherit from base data +++++++++++++++++++++++++++*/
    virtual void SetRequestedRegionToLargestPossibleRegion ();

    virtual bool RequestedRegionIsOutsideOfTheBufferedRegion ();

    virtual bool VerifyRequestedRegion ();

    virtual const mitk::Geometry3D* GetUpdatedGeometry (int t=0);

    virtual mitk::Geometry3D* GetGeometry (int t=0) const;

    virtual void SetRequestedRegion (itk::DataObject *data);


  protected:

    ContourModel();
    ContourModel(const mitk::ContourModel &other);
    virtual ~ContourModel();

    void MoveVertex(VertexType* vertex, mitk::Vector3D &vector);

    mitk::ContourModelElement::Pointer m_Contour;
    VertexType* m_SelectedVertex;
  };
}

#endif