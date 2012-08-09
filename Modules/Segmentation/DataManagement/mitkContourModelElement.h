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

#include <ANN/ANN.h>


#include <deque>

namespace mitk 
{

  //##Documentation
  //## @brief
  //##
  class Segmentation_EXPORT ContourModelElement : public itk::LightObject
  {

  public:

    mitkClassMacro(ContourModelElement, itk::LightObject);

    itkNewMacro(Self);

    mitkCloneMacro(Self);


/*+++++++++++++++++++++ Data container representing vertices +++++++++++++++++*/
    
    //##Documentation
    //## @brief
    //##
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

    //##Documentation
    //## @brief
    //##
    virtual ConstVertexIterator ConstIteratorBegin()
    {
      return this->m_Vertices->begin();
    }

    //##Documentation
    //## @brief
    //##
    virtual ConstVertexIterator ConstIteratorEnd()
    {
      return this->m_Vertices->end();
    }

    //##Documentation
    //## @brief
    //##
    virtual VertexIterator IteratorBegin()
    {
      return this->m_Vertices->begin();
    }

    //##Documentation
    //## @brief
    //##
    virtual VertexIterator IteratorEnd()
    {
      return this->m_Vertices->end();
    }

    //##Documentation
    //## @brief
    //##
    virtual int GetSize()
    {
      return this->m_Vertices->size();
    }
/*++++++++++++++++  END inline methods  +++++++++++++++++++++++*/



    //##Documentation
    //## @brief
    //##
    virtual void AddVertex(mitk::Point3D &vertex, bool isActive);

    //##Documentation
    //## @brief
    //##
    virtual void InsertVertexAtIndex(mitk::Point3D &vertex, bool isActive, int index);

    //##Documentation
    //## @brief
    //##
    virtual VertexType* GetVertexAt(int index);

    //##Documentation
    //## @brief
    //##
    virtual VertexType* GetVertexAt(const mitk::Point3D &point, float eps);

    //##Documentation
    //## @brief
    //##
    VertexListType* GetVertexList();

    //##Documentation
    //## @brief
    //##
    virtual bool IsClosed();

    //##Documentation
    //## @brief
    //##
    virtual void Close();

    //##Documentation
    //## @brief
    //##
    virtual void Open();

    //##Documentation
    //## @brief
    //##
    virtual void SetIsClosed(bool isClosed);

    //##Documentation
    //## @brief
    //##
    void Concatenate(mitk::ContourModelElement* other);

    //##Documentation
    //## @brief
    //##
    virtual void RemoveVertex(VertexType* vertex);

    //##Documentation
    //## @brief
    //##
    virtual void RemoveVertexAt(int index);

    //##Documentation
    //## @brief
    //##
    virtual bool RemoveVertexAt(mitk::Point3D &point, float eps);

    //##Documentation
    //## @brief
    //##
    VertexType* BruteForceGetVertexAt(const mitk::Point3D &point, float eps);

    //##Documentation
    //## @brief
    //##
    VertexType* OptimizedGetVertexAt(const mitk::Point3D &point, float eps);


  protected:

    ContourModelElement();
    ContourModelElement(const mitk::ContourModelElement &other);
    virtual ~ContourModelElement();

    VertexListType* m_Vertices;
    bool m_IsClosed;

  };
}

#endif