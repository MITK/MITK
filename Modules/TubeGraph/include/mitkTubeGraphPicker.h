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

    void SetTubeGraph (const TubeGraph* tubeGraph);

    std::pair<mitk::TubeGraph::TubeDescriptorType, mitk::TubeElement*> GetPickedTube( const Point3D pickedPosition);

    TubeGraphPicker();
    virtual ~TubeGraphPicker();


  protected:

    Point3D m_WorldPosition;
    TubeGraph::Pointer m_TubeGraph;
    TubeGraphProperty::Pointer m_TubeGraphProperty;
  };

} // namespace

#endif
