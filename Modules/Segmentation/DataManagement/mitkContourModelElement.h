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
#ifndef _MITK_CONTOURMODELELEMENT_H_
#define _MITK_CONTOURMODELELEMENT_H_

#include "mitkCommon.h"
#include "SegmentationExports.h"
#include <mitkVector.h>


#include <deque>

namespace mitk 
{

  /**
  * Represents a contour element consisting of several control points.
  *
  */
  class Segmentation_EXPORT ContourModelElement : public itk::LightObject
  {

  public:

    mitkClassMacro(ContourModelElement, itk::LightObject);

    itkNewMacro(Self);

    mitkCloneMacro(Self);


/*+++++++++++++++++++++ Data container representing vertices +++++++++++++++++*/
    struct ContourModelControlPoint
    {
      ContourModelControlPoint(mitk::Point3D* point=NULL, bool active=false)
        : Coordinates(point), IsActive(active)
      {

      };

      bool IsActive;

      mitk::Point3D* Coordinates;
    };
/*+++++++++++++++++++++ END Data container representing vertices ++++++++++++++*/


/*+++++++++++++++ typedefs +++++++++++++++++++++++++++++++*/
    typedef ContourModelControlPoint VertexType;
    typedef std::deque<VertexType*> VertexListType;
    typedef VertexListType::iterator VertexIterator;
    typedef VertexListType::const_iterator ConstVertexIterator;
/*+++++++++++++++ END typedefs ++++++++++++++++++++++++++++*/


    virtual void AddVertex(mitk::Point3D &vertex);

    virtual VertexType* GetVertexAt(int index);

    virtual VertexType* GetVertexAt(const mitk::Point3D &point, float eps);

    VertexListType* GetVertexList();

    virtual bool IsClosed();

    void Concatenate(mitk::ContourModelElement* other);

    virtual void RemoveVertex(VertexType* vertex);

    virtual void RemoveVertexAt(int index);

    virtual bool RemoveVertexAt(mitk::Point3D &point, float eps);

    virtual ConstVertexIterator GetConstVertexIterator()
    {
      return this->m_Vertices->begin();
    }

    virtual VertexIterator GetVertexIterator()
    {
      return this->m_Vertices->begin();
    }

    virtual int GetSize()
    {
      return this->m_Vertices->size();
    }

  protected:

    ContourModelElement();
    ContourModelElement(const mitk::ContourModelElement &other);
    virtual ~ContourModelElement();

    VertexListType* m_Vertices;

  };
}

#endif