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

#ifndef mitkIndexROI_h
#define mitkIndexROI_h

#include <sofa/core/topology/BaseMeshTopology.h>
#include <sofa/core/DataEngine.h>
#include <MitkSimulationExports.h>

namespace mitk
{
  class MITKSIMULATION_EXPORT IndexROI : public sofa::core::DataEngine
  {
  public:
    SOFA_CLASS(IndexROI, sofa::core::DataEngine);

    void init() override;
    void update() override;

    sofa::core::objectmodel::Data<unsigned int> m_First;
    sofa::core::objectmodel::Data<unsigned int> m_Last;
    sofa::core::objectmodel::Data<sofa::core::topology::BaseMeshTopology::SetIndex> m_Individual;
    sofa::core::objectmodel::Data<sofa::core::topology::BaseMeshTopology::SetIndex> m_Indices;
    sofa::core::objectmodel::Data<sofa::defaulttype::ExtVec3fTypes::VecCoord> m_PointsInROI;
    sofa::core::objectmodel::Data<sofa::defaulttype::ExtVec3fTypes::VecCoord> m_Position;

  private:
    IndexROI();
    ~IndexROI();

    IndexROI(const MyType&);
    MyType& operator=(const MyType&);
  };
}

#endif
