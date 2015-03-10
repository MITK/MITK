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

#ifndef _MITK_TubeGraphEdge_H
#define _MITK_TubeGraphEdge_H

#include <MitkTubeGraphExports.h>

#include "mitkTubeElement.h"
#include "mitkCircularProfileTubeElement.h"
#include "mitkTubeGraphVertex.h"

namespace mitk {
  /**
  * \brief Base Class for Tube Graph Edges
  */
  class MITKTUBEGRAPH_EXPORT TubeGraphEdge {

  public:

    TubeGraphEdge();
    virtual ~TubeGraphEdge();

    /**
    * Set the vector which contains all TubeElements.
    * @param elementVector Vector with all TubeElements
    */
    void SetElementVector(std::vector<TubeElement*> elementVector);

    /**
    * Returns the vector which contains all TubeElements.
    * @returns vector with all TubeElements.
    */
    std::vector<TubeElement*> GetElementVector();

    /**
    * Returns the number of TubeElements which are contained in this tube.
    * @returns Number of TubeElements.
    */
    unsigned int GetNumberOfElements() const;

    /**
    * Adds a new TubeElement at the end of this edge.
    * @param element The new TubeElement.
    */
    void AddTubeElement(TubeElement* element);

    /**
    * Adds a new TubeElement at the given position.
    * @param position The position for the TubeElement.
    * @param element The new TubeElement.
    */
    void AddTubeElementAt(unsigned int position, TubeElement* element);

    /**
    * Returns the TubeElement at the given position.
    * @param position The position of the Element.
    * @returns The TubeElement at the given position.
    */
    TubeElement* GetTubeElement( unsigned int position );

    /**
    * Calculates the length of tube graph edge inc. the source and target vertex..
    *
    * @param source the source vertex of the edge
    * @param target the target vertex of the edge
    * @returns the length of the edge in mm
    */
    float GetEdgeLength (TubeGraphVertex& source, TubeGraphVertex& target);

    /**
    * Calculates the average diameter of tube graph edge inc. the source and target vertex.
    *
    * @param source the source vertex of the edge
    * @param target the target vertex of the edge
    * @returns the average diameter of the edge
    */
    float GetEdgeAverageDiameter(TubeGraphVertex& source, TubeGraphVertex& target);

    /**
    * Comparison operation between this object and the given object.
    * @param right The object to compare with.
    * @return true, if the object is the same;false, if not.
    */
    bool operator== (const TubeGraphEdge& right) const;

  private:
    std::vector<TubeElement*> m_ElementVector;

  };//class

}//namespace
#endif
