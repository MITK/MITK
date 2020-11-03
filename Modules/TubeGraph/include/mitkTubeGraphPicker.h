/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef _mitk_TubeGraphPicker_h
#define _mitk_TubeGraphPicker_h

#include <MitkTubeGraphExports.h>

#include "mitkCircularProfileTubeElement.h"
#include "mitkTubeGraph.h"
#include "mitkTubeGraphProperty.h"

namespace mitk
{
  class MITKTUBEGRAPH_EXPORT TubeGraphPicker
  {
  public:
    /* mitkClassMacro( TubeGraphPicker, BaseDataSource );
     itkNewMacro( Self );*/

    void SetTubeGraph(const TubeGraph *tubeGraph);

    std::pair<mitk::TubeGraph::TubeDescriptorType, mitk::TubeElement *> GetPickedTube(const Point3D pickedPosition);

    TubeGraphPicker();
    virtual ~TubeGraphPicker();

  protected:
    Point3D m_WorldPosition;
    TubeGraph::ConstPointer m_TubeGraph;
    TubeGraphProperty::Pointer m_TubeGraphProperty;
  };

} // namespace

#endif
