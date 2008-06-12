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


#ifndef BASEVTKMAPPER3D_H_HEADER_INCLUDED
#define BASEVTKMAPPER3D_H_HEADER_INCLUDED

#include "mitkCommon.h"
#include "mitkMapper.h"
#include "mitkMapper3D.h"
#include "mitkBaseRenderer.h"

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
class MITK_CORE_EXPORT BaseVtkMapper3D : public Mapper3D
{
public:
  mitkClassMacro(BaseVtkMapper3D, Mapper3D);

  virtual vtkProp *GetProp();
  virtual vtkProp *GetLabelProp();

  void MitkRenderOpaqueGeometry(mitk::BaseRenderer* renderer);
  void MitkRenderTranslucentGeometry(mitk::BaseRenderer* renderer);
  void MitkRenderOverlay(mitk::BaseRenderer* renderer);

  //##Documentation
  //## @brief Set the vtkTransform of the m_Prop3D for
  //## the current time step of \a renderer
  //##
  //## Called by mitk::VtkPropRenderer::Update before rendering
  //##
  virtual void UpdateVtkTransform();

  //##Documentation
  //## @brief Apply color and opacity read from the PropertyList
  virtual void ApplyProperties(vtkActor* actor, mitk::BaseRenderer* renderer);

protected:
  BaseVtkMapper3D();

  virtual ~BaseVtkMapper3D();

public:

  itkGetObjectMacro(Geometry,Geometry3D);
  itkSetObjectMacro(Geometry,Geometry3D);

protected:
  Geometry3D::Pointer m_Geometry;
  LevelWindow m_LevelWindow;

  vtkProp3D *m_Prop3D;
  vtkProp3DCollection *m_LabelActorCollection;
};

} // namespace mitk

#endif /* BASEVTKMAPPER3D_H_HEADER_INCLUDED */
