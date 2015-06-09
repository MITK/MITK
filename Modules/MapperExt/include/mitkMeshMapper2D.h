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


#ifndef MITKMESHMAPPER2D_H_HEADER_INCLUDED
#define MITKMESHMAPPER2D_H_HEADER_INCLUDED

#include "mitkCommon.h"
#include "MitkMapperExtExports.h"
#include "mitkGLMapper.h"

namespace mitk {

class BaseRenderer;
class Mesh;

/**
 * \brief OpenGL-based mapper to display a mesh in a 2D window
 *
 * \todo implement for AbstractTransformGeometry.
 * \ingroup Mapper
 */
class MITKMAPPEREXT_EXPORT MeshMapper2D : public GLMapper
{
public:
  mitkClassMacro(MeshMapper2D, GLMapper);

  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

  /** @brief Get the Mesh to map */
  const mitk::Mesh *GetInput(void);

  virtual void Paint( mitk::BaseRenderer *renderer ) override;

protected:
  MeshMapper2D();

  virtual ~MeshMapper2D();
};

} // namespace mitk



#endif /* MITKMESHMapper2D_H_HEADER_INCLUDED */
