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

#ifndef mitkMeshMitkLoader_h
#define mitkMeshMitkLoader_h

#include <sofa/core/loader/MeshLoader.h>
#include <org_mitk_simulation_Export.h>

namespace mitk
{
  class SIMULATION_INIT_EXPORT MeshMitkLoader : public sofa::core::loader::MeshLoader
  {
  public:
    SOFA_CLASS(MeshMitkLoader, sofa::core::loader::MeshLoader);

    bool canLoad() override;
    bool load() override;

  private:
    MeshMitkLoader();
    ~MeshMitkLoader();

    MeshMitkLoader(const MyType&);
    MyType& operator=(const MyType&);
  };
}

#endif
