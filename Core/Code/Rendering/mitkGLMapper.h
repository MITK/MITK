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


#ifndef MITKGLMAPPER_H_HEADER_INCLUDED_C197C872
#define MITKGLMAPPER_H_HEADER_INCLUDED_C197C872

#include <MitkExports.h>
#include "mitkMapper.h"
#include "mitkBaseRenderer.h"
#include "mitkVtkPropRenderer.h"

namespace mitk {

//##Documentation
//## @brief Base class of all OpenGL-based Mappers
//##
//## Those must implement the abstract method Paint(BaseRenderer).
//## @ingroup Mapper
class MITK_CORE_EXPORT GLMapper : public Mapper
{
  public:
    //##Documentation
    //## @brief Do the painting into the @a renderer
    virtual void Paint(mitk::BaseRenderer *renderer) = 0;

    //##Documentation
    //## @brief Apply color and opacity read from the PropertyList
    virtual void ApplyProperties(mitk::BaseRenderer* renderer);

    // todo: virtual or not?
    //##
    //## @brief calls the paint method
    void MitkRender(mitk::BaseRenderer* renderer, mitk::VtkPropRenderer::RenderType type);

   //##
   //## @brief Returns whether this is an vtk-based mapper
   virtual bool IsVtkBased() const { return false; }

   //##
   //## @brief Returns whether this mapper allows picking in the renderwindow
   virtual bool IsPickable() const { return false; }

  protected:

    GLMapper();
    virtual ~GLMapper();
};

} // namespace mitk



#endif /* MITKGLMAPPER2D_H_HEADER_INCLUDED_C197C872 */
