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


#ifndef MITKSURFACEDATAVTKMAPPER3D_H_HEADER_INCLUDED_C1907273
#define MITKSURFACEDATAVTKMAPPER3D_H_HEADER_INCLUDED_C1907273

#include "mitkCommon.h"
#include "mitkBaseVtkMapper3D.h"
#include "mitkSurface.h"
#include "mitkBaseRenderer.h"

#include <vtkActor.h>
#include <vtkPainterPolyDataMapper.h>
#include <vtkPolyDataMapper.h>
#include <vtkPolyDataNormals.h>
#include <vtkPlaneCollection.h>

namespace mitk {


//##Documentation
//## @brief Vtk-based mapper for Surface
//##
//## @ingroup Mapper



  /**
  * @brief Vtk-based mapper for Surface
  * 

  * Properties that can be set for surfaces and influence the surfaceVTKMapper3D are:
  *
  *   - \b "material": (MaterialProperty) Material of the surface object. The material properties are:

  *   - \b "color": (ColorProperty) Color of the surface object
  *   - \b "AmbientColor": (AmbientColor) Ambient color  of the surface object
  *   - \b "Ambient": (  FloatProperty) Ambient coefficient of the surface object
  *   - \b "DiffuseColor": ( DiffuseColor) Diffuse color of the surface object
  *   - \b "Diffuse": (FloatProperty) Diffuse coefficient of the surface object
  *   - \b "SpecularColor": (SpecularColor) Specular Color of the surface object
  *   - \b "Specular": (FloatProperty) Specular coefficient of the surface object
  *   - \b "SpecularPower": (FloatProperty) Specular power of the surface object
  *   - \b "Opacity": (FloatProperty) Opacity of the surface object
  *   - \b "Interpolation": (VtkInterpolationProperty) Interpolation
  *   - \b "Representation": (VtkRepresentationProperty*) Representation
  *   - \b "LineWidth": (IntProperty) Width in pixels of the lines drawn.
  *   - \b ""scalar visibility": (BoolProperty) If the scarlars of the surface are visible
  
  The default properties are:
  *   - \b "wireframe line width": (FloatProperty::New(1.0))
  *   - \b ""material": (MaterialProperty)
  *   - \b ""scalar visibility": (BoolProperty)
  *   - \b "color mode": (BoolProperty)
  *   - \b "representation": (VtkRepresentationProperty)
  *   - \b "interpolation"(VtkInterpolationProperty)
  *   - \b "scalar mode": (VtkScalarModeProperty)

  * Properties to look for are:
  *
  *   - \b "scalar visibility": if set to on, scalars assigned to the data are shown
  *        Turn this on if using a lookup table.
  *   - \b "ScalarsRangeMinimum": Optional. Can be used to store the scalar min, e.g.
  *         for the level window settings.
  *   - \b "ScalarsRangeMaximum": Optional. See above.
  *
  * There might be still some other, deprecated properties. These will not be documented anymore.
  * Please check the source if you really need them.
  *
  * @ingroup Mapper
  */

class MITK_CORE_EXPORT SurfaceVtkMapper3D : public BaseVtkMapper3D
{
public:

  mitkClassMacro(SurfaceVtkMapper3D, BaseVtkMapper3D);

  itkNewMacro(Self);

  itkSetMacro(GenerateNormals, bool);

  itkGetMacro(GenerateNormals, bool);

  //enable ImmediateModeRendering for vtkMapping
  //yet to solve bug 1398
  void SetImmediateModeRenderingOn(int on = 1);

  itkGetMacro(ImmediateModeRenderingOn, int);

  virtual const mitk::Surface* GetInput();

  virtual vtkProp *GetVtkProp(mitk::BaseRenderer *renderer);

  virtual void ApplyProperties(vtkActor* actor, mitk::BaseRenderer* renderer);

  static void SetDefaultProperties(mitk::DataTreeNode* node, mitk::BaseRenderer* renderer = NULL, bool overwrite = false);

protected:
  SurfaceVtkMapper3D();

  virtual ~SurfaceVtkMapper3D();

  virtual void GenerateData(mitk::BaseRenderer* renderer);

  /** Checks whether the specified property is a ClippingProperty and if yes,
   * adds it to m_ClippingPlaneCollection (internal method). */
  virtual void CheckForClippingProperty( mitk::BaseRenderer* renderer, mitk::BaseProperty *property );

  bool m_GenerateNormals;

  //enable ImmediateModeRendering for the vtkMapper
  int m_ImmediateModeRenderingOn;
  
public:
    
  class LocalStorage : public mitk::Mapper::BaseLocalStorage
  {
    public:

      vtkActor* m_Actor;
      vtkPolyDataMapper *m_VtkPolyDataMapper;
      vtkPolyDataNormals *m_VtkPolyDataNormals;
      vtkPlaneCollection *m_ClippingPlaneCollection;
      
      itk::TimeStamp m_ShaderTimestampUpdate;

      LocalStorage()
      {
        m_VtkPolyDataMapper = vtkPainterPolyDataMapper::New();
        m_VtkPolyDataNormals = vtkPolyDataNormals::New();
        m_Actor = vtkActor::New();
        m_ClippingPlaneCollection = vtkPlaneCollection::New();

        m_Actor->SetMapper(m_VtkPolyDataMapper);
      }
      
      ~LocalStorage()
      {
        m_VtkPolyDataMapper->Delete();
        m_VtkPolyDataNormals->Delete();
        m_Actor->Delete();                                  
        m_ClippingPlaneCollection->Delete();
      }
  };  
    
  mitk::Mapper::LocalStorageHandler<LocalStorage> m_LSH;  
  
};

} // namespace mitk

#endif /* MITKSURFACEDATAVTKMAPPER3D_H_HEADER_INCLUDED_C1907273 */
