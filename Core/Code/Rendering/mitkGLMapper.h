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


/** \brief Base class of all OpenGL-based mappers.
*
* Those must implement the abstract method Paint(BaseRenderer), which is called by
* MitkRender(...).
* \ingroup Mapper
*/
class MITK_CORE_EXPORT GLMapper : public Mapper
{
  public:
    /** \brief Do the painting into the \a renderer */
    virtual void Paint(mitk::BaseRenderer *renderer) = 0;

    /** \brief Apply color and opacity read from the PropertyList */
    virtual void ApplyProperties(mitk::BaseRenderer* renderer);

    // todo: virtual or not?
    /** \brief Checks visibility and calls the paint method
    *
    * Note: The enumeration is disregarded, since OpenGL rendering only needs a
    * single render pass.
    */
    void MitkRender(mitk::BaseRenderer* renderer, mitk::VtkPropRenderer::RenderType type);

   /** \brief Returns  whether this is a vtk-based mapper
   *  \return false, since all mappers deriving from this class are OpenGL mappers
   *  @deprecated All mappers of superclass VTKMapper are vtk based, use a dynamic_cast instead
   */
   DEPRECATED( virtual bool IsVtkBased() const );

   /** \brief Returns whether this mapper allows picking in the renderwindow
   virtual bool IsPickable() const { return false; }*/

  protected:

    GLMapper();
    virtual ~GLMapper();
};

} // namespace mitk



#endif /* MITKGLMAPPER2D_H_HEADER_INCLUDED_C197C872 */
