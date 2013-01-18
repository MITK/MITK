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

// change number
#ifndef VTKMAPPER_H_HEADER_INCLUDED_C1C5453B
#define VTKMAPPER_H_HEADER_INCLUDED_C1C5453B

#include <MitkExports.h>
#include "mitkMapper.h"
#include "mitkBaseRenderer.h"
#include "mitkDataNode.h"
//#include "mitkProperties.h"
//#include "mitkAnnotationProperty.h"
#include "mitkVtkPropRenderer.h"

#include <vtkProp3D.h>
//#include <vtkLODProp3D.h>
#include <vtkActor.h>
#include <vtkProperty.h>
#include <vtkLinearTransform.h>
#include <vtkMapper.h>
#include <vtkPropAssembly.h>
//#include <vtkFollower.h>
//#include <vtkVectorText.h>
#include <vtkPolyDataMapper.h>
#include <vtkProp3DCollection.h>

class vtkProp;
class vtkProp3D;
class vtkActor;
class vtkProp3DCollection;

namespace mitk {

//##Documentation
//## @brief Base class of all Mappers for 2D display
//##
//## @ingroup Mapper
class MITK_CORE_EXPORT VtkMapper : public Mapper
{
  public:
    mitkClassMacro(VtkMapper,Mapper);

    virtual vtkProp* GetVtkProp(mitk::BaseRenderer* renderer) = 0;

    static void SetVtkMapperImmediateModeRendering(vtkMapper *mapper);

     /**
     * \brief Returns whether this is an vtk-based mapper
     */
    virtual bool IsVtkBased() const { return true; }

     /**
     * \brief Returns whether this mapper allows picking in the renderwindow
     */
    virtual bool IsPickable() const { return true; }

    void MitkRender(mitk::BaseRenderer* renderer, mitk::VtkPropRenderer::RenderType type);

      // virtual or not???
    virtual void MitkRenderOverlay(BaseRenderer* renderer);
    virtual void MitkRenderOpaqueGeometry(BaseRenderer* renderer);
    virtual void MitkRenderTranslucentGeometry(BaseRenderer* renderer);
    virtual void MitkRenderVolumetricGeometry(BaseRenderer* renderer);

    /** \brief Returns true if this mapper owns the specified vtkProp for
    * the given BaseRenderer.
    *
    * Note: returns false by default; should be implemented for VTK-based
    * Mapper subclasses. */
    virtual bool HasVtkProp( const vtkProp *prop, BaseRenderer *renderer );

    //##Documentation
    //## @brief Set the vtkTransform of the m_Prop3D for
    //## the current time step of \a renderer
    //##
    //## Called by mitk::VtkPropRenderer::Update before rendering
    //##
    virtual void UpdateVtkTransform(mitk::BaseRenderer *renderer);

    //##Documentation
    //## @brief Apply color and opacity read from the PropertyList
    virtual void ApplyProperties(vtkActor* actor, mitk::BaseRenderer* renderer);

    /**
    * \brief Release vtk-based graphics resources. Must be overwritten in
    * subclasses if vtkProps additional to m_Prop3D are used.
    */
    virtual void ReleaseGraphicsResources(vtkWindow *renWin);

  protected:

    VtkMapper();

    virtual ~VtkMapper();

};
} // namespace mitk
#endif /* VTKMAPPER_H_HEADER_INCLUDED_C1C5453B */
