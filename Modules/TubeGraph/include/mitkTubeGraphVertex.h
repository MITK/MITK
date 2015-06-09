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

#ifndef _MITK_TubeGraphVertex_H
#define _MITK_TubeGraphVertex_H

#include <MitkTubeGraphExports.h>

#include "mitkTubeElement.h"

namespace mitk {
  /**
  * \brief Base Class for Tube Graph Vertices
  */
  class  MITKTUBEGRAPH_EXPORT TubeGraphVertex {

  public:

    TubeGraphVertex();
    TubeGraphVertex(TubeElement* element);
    virtual ~TubeGraphVertex();

    /**
    * Set the TubeElement which contains the information about diameter and position.
    * @param element The TubeElement for the vertex.
    */
    void SetTubeElement(TubeElement* element);

    /**
    * Returns the TubeElement.
    * @returns The TubeElement
    */
    const TubeElement* GetTubeElement() const;

    /**
    * Comparison operation between this object and the given object.
    * @param right The object to compare with.
    * @return true, if the object is the same;false, if not.
    */
    bool operator==(const TubeGraphVertex& right)const;

  private:

    TubeElement* m_element;

  };//class

}//namespace
#endif
