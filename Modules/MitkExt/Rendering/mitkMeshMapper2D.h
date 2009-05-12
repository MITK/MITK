/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef MITKMESHMAPPER2D_H_HEADER_INCLUDED
#define MITKMESHMAPPER2D_H_HEADER_INCLUDED

#include "mitkCommon.h"
#include "mitkGLMapper2D.h"

namespace mitk {

class BaseRenderer;
class Mesh;

/**
 * \brief OpenGL-based mapper to display a mesh in a 2D window
 * 
 * \todo implement for AbstractTransformGeometry.
 * \ingroup Mapper
 */
class MITK_CORE_EXPORT MeshMapper2D : public GLMapper2D
{
public:
  mitkClassMacro(MeshMapper2D, Mapper2D);

  itkNewMacro(Self);

  /** @brief Get the Mesh to map */
  const mitk::Mesh *GetInput(void);

  virtual void Paint( mitk::BaseRenderer *renderer );

protected:
  MeshMapper2D();

  virtual ~MeshMapper2D();
};

} // namespace mitk



#endif /* MITKMESHMapper2D_H_HEADER_INCLUDED */
