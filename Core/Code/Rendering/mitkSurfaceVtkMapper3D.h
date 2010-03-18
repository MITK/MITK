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
#include <vtkOpenGLPolyDataMapper.h>
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
  *   - \b "color": (ColorProperty) Diffuse color of the surface object (this property will be read when material.diffuseColor is not defined)
  *   - \b "Opacity": (FloatProperty) Opacity of the surface object
  *   - \b "material.ambientColor": (ColorProperty) Ambient color  of the surface object
  *   - \b "material.ambientCoefficient": (  FloatProperty) Ambient coefficient of the surface object
  *   - \b "material.diffuseColor": ( ColorProperty) Diffuse color of the surface object
  *   - \b "material.diffuseCoefficient": (FloatProperty) Diffuse coefficient of the surface object
  *   - \b "material.specularColor": (ColorProperty) Specular Color of the surface object
  *   - \b "material.specularCoefficient": (FloatProperty) Specular coefficient of the surface object
  *   - \b "material.specularPower": (FloatProperty) Specular power of the surface object
  *   - \b "material.interpolation": (VtkInterpolationProperty) Interpolation
  *   - \b "material.representation": (VtkRepresentationProperty*) Representation
  *   - \b "material.wireframeLineWidth": (FloatProperty) Width in pixels of the lines drawn.
  *   - \b "scalar visibility": (BoolProperty) If the scarlars of the surface are visible
  
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

  static void SetDefaultProperties(mitk::DataNode* node, mitk::BaseRenderer* renderer = NULL, bool overwrite = false);

protected:
  SurfaceVtkMapper3D();

  virtual ~SurfaceVtkMapper3D();

  virtual void GenerateData(mitk::BaseRenderer* renderer);
  
  virtual void ResetMapper( mitk::BaseRenderer* renderer );

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
        m_VtkPolyDataMapper = vtkOpenGLPolyDataMapper::New();
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
  
  static void ApplyMitkPropertiesToVtkProperty(mitk::DataNode *node, vtkProperty* property, mitk::BaseRenderer* renderer);
  static void SetDefaultPropertiesForVtkProperty(mitk::DataNode* node, mitk::BaseRenderer* renderer, bool overwrite);
};

} // namespace mitk

#endif /* MITKSURFACEDATAVTKMAPPER3D_H_HEADER_INCLUDED_C1907273 */
