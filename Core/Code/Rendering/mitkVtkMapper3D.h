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


#ifndef BASEVTKMAPPER3D_H_HEADER_INCLUDED
#define BASEVTKMAPPER3D_H_HEADER_INCLUDED

#include <MitkExports.h>
#include "mitkMapper.h"
#include "mitkMapper3D.h"
#include "mitkBaseRenderer.h"
#include "vtkMapper.h"

class vtkProp;
class vtkProp3D;
class vtkActor;
class vtkProp3DCollection;

namespace mitk {

//##Documentation
//## @brief Base class of all vtk-based 3D-Mappers
//##
//## GetProp() returns m_Prop3D, which should be
//## initialized by sub-classes (e.g., by setting
//## it to an vtkActor).
//## @ingroup Mapper
class MITK_CORE_EXPORT VtkMapper3D : public Mapper3D
{
public:
  mitkClassMacro(VtkMapper3D, Mapper3D);

  virtual vtkProp *GetVtkProp(mitk::BaseRenderer *renderer) = 0;

  static void SetVtkMapperImmediateModeRendering(vtkMapper *mapper);

  virtual void MitkRenderOpaqueGeometry(mitk::BaseRenderer* renderer);
  virtual void MitkRenderTranslucentGeometry(mitk::BaseRenderer* renderer);
  virtual void MitkRenderOverlay(mitk::BaseRenderer* renderer);


  virtual void MitkRenderVolumetricGeometry(mitk::BaseRenderer* renderer);

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


  /** \brief Returns true if this mapper owns the specified vtkProp for
   * the given BaseRenderer.
   *
   * Note: returns false by default; should be implemented for VTK-based
   * Mapper subclasses. */
  virtual bool HasVtkProp( const vtkProp *prop, BaseRenderer *renderer );

  /**
   * \brief Returns whether this is an vtk-based mapper
   */
  virtual bool IsVtkBased() const { return true; }

protected:
  VtkMapper3D();

  virtual ~VtkMapper3D();

  /** Checks whether the specified property is a AnnotationProperty and if yes,
  * adds it to m_LabelActorCollection (internal method). */
 // virtual void CheckForAnnotationProperty( mitk::BaseProperty *property, BaseRenderer *renderer );

public:

  itkGetObjectMacro(Geometry,Geometry3D);
  itkSetObjectMacro(Geometry,Geometry3D);

protected:
  Geometry3D::Pointer m_Geometry;
  LevelWindow m_LevelWindow;

  //vtkProp3D *m_Prop3D;
  //vtkProp3DCollection *m_LabelActorCollection;
};

} // namespace mitk

#endif /* BASEVTKMAPPER3D_H_HEADER_INCLUDED */
