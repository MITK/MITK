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
      ContourModelControlPoint(mitk::Point3D &point, bool active=false)
        : Coordinates(point), IsActive(active)
      {

      };

      bool IsActive;

      mitk::Point3D Coordinates;
    };
/*+++++++++++++++++++++ END Data container representing vertices ++++++++++++++*/


/*+++++++++++++++ typedefs +++++++++++++++++++++++++++++++*/
    typedef ContourModelControlPoint VertexType;
    typedef std::deque<VertexType*> VertexListType;
    typedef VertexListType::iterator VertexIterator;
    typedef VertexListType::const_iterator ConstVertexIterator;
/*+++++++++++++++ END typedefs ++++++++++++++++++++++++++++*/



/*++++++++++++++++  inline methods  +++++++++++++++++++++++*/
    virtual ConstVertexIterator ConstIteratorBegin()
    {
      return this->m_Vertices->begin();
    }

    virtual ConstVertexIterator ConstIteratorEnd()
    {
      return this->m_Vertices->end();
    }

    virtual VertexIterator IteratorBegin()
    {
      return this->m_Vertices->begin();
    }

    virtual VertexIterator IteratorEnd()
    {
      return this->m_Vertices->end();
    }

    virtual int GetSize()
    {
      return this->m_Vertices->size();
    }
/*++++++++++++++++  END inline methods  +++++++++++++++++++++++*/



    virtual void AddVertex(mitk::Point3D &vertex);

    virtual VertexType* GetVertexAt(int index);

    virtual VertexType* GetVertexAt(const mitk::Point3D &point, float eps);

    VertexListType* GetVertexList();

    virtual bool IsClosed();

    virtual void Close();

    void Concatenate(mitk::ContourModelElement* other);

    virtual void RemoveVertex(VertexType* vertex);

    virtual void RemoveVertexAt(int index);

    virtual bool RemoveVertexAt(mitk::Point3D &point, float eps);


  protected:

    ContourModelElement();
    ContourModelElement(const mitk::ContourModelElement &other);
    virtual ~ContourModelElement();

    VertexListType* m_Vertices;
    bool m_IsClosed;

  };
}

#endif