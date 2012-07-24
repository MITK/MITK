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


    struct ContourModelControlPoint
    {
      ContourModelControlPoint(mitk::Point3D* point=NULL, bool active=false)
        : Coordinates(point), IsActive(active)
      {

      };

      bool IsActive;

      mitk::Point3D* Coordinates;
    };


    itkNewMacro(Self);

    typedef ContourModelControlPoint VertexType;



    virtual void AddVertex(mitk::Point3D* vertex);

    virtual VertexType* GetVertexAt(int index);

    virtual VertexType* GetVertexAt(mitk::Point3D* point);

    std::deque<VertexType*>* GetVertexList();

    virtual bool IsClosed();

    void Concatenate(mitk::ContourModelElement* other);

    virtual bool RemoveVertex(VertexType* vertex);

    virtual bool RemoveVertexAt(int index);

    virtual bool RemoveVertexAt(mitk::Point3D* point);



  protected:

    ContourModelElement();
    ContourModelElement(mitk::ContourModelElement &other);
    virtual ~ContourModelElement();

    std::deque<VertexType*>* m_Vertices;
    VertexType* m_SlectedVertex;
  };
}

#endif