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


#ifndef MITKGLMAPPER2D_H_HEADER_INCLUDED_C197C872
#define MITKGLMAPPER2D_H_HEADER_INCLUDED_C197C872

#include <MitkExports.h>
#include "mitkMapper2D.h"
#include "mitkBaseRenderer.h"

namespace mitk {

//##Documentation
//## @brief Base class of all OpenGL-based 2D-Mappers
//##
//## Those must implement the abstract method Paint(BaseRenderer).
//## @ingroup Mapper
class MITK_CORE_EXPORT GLMapper2D : public Mapper2D
{
  public:
    //##Documentation
    //## @brief Do the painting into the @a renderer
    virtual void Paint(mitk::BaseRenderer *renderer) = 0;

    //##Documentation
    //## @brief Apply color and opacity read from the PropertyList
    virtual void ApplyProperties(mitk::BaseRenderer* renderer);

    void MitkRenderOpaqueGeometry(mitk::BaseRenderer* renderer);
    void MitkRenderTranslucentGeometry(mitk::BaseRenderer* renderer);
    void MitkRenderOverlay(mitk::BaseRenderer* renderer);
    void MitkRenderVolumetricGeometry(mitk::BaseRenderer* renderer);

  /**
   * \brief Returns whether this is an vtk-based mapper
   */
  virtual bool IsVtkBased() const { return false; }



  protected:
    GLMapper2D();
    virtual ~GLMapper2D();
};

} // namespace mitk



#endif /* MITKGLMAPPER2D_H_HEADER_INCLUDED_C197C872 */


