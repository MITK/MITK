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


#ifndef MITKVOLUMEDATAVTKMAPPER3D_H_HEADER_INCLUDED
#define MITKVOLUMEDATAVTKMAPPER3D_H_HEADER_INCLUDED

#include <MitkCoreExports.h>
#include "mitkVtkMapper.h"
#include "mitkBaseRenderer.h"
#include "mitkImage.h"
#include <vtkVolumeProperty.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkPlane.h>
#include <vtkImplicitPlaneWidget.h>
#include <vtkImageMask.h>

#include <vector>
#include <set>

class vtkAssembly;
class vtkVolumeRayCastMapper;
class vtkFixedPointVolumeRayCastMapper;
class vtkVolumeTextureMapper2D;
class vtkVolumeMapper;
class vtkVolume;
class vtkObject;
class vtkImageShiftScale;
class vtkImageChangeInformation;
class vtkLODProp3D;
class vtkImageResample;
class vtkCubeSource;
class vtkPolyDataMapper;
class vtkActor;


namespace mitk {

  /************************************************************************/
  /* Properties that influence the mapper are:
  *
  * - \b "level window": for the level window of the volume data
  * - \b "LookupTable" : for the lookup table of the volume data
  * - \b "TransferFunction" (mitk::TransferFunctionProperty): for the used transfer function of the volume data
  ************************************************************************/

//##Documentation
//## @brief Vtk-based mapper for VolumeData
//##
//## @ingroup Mapper
class MITKCORE_EXPORT VolumeDataVtkMapper3D : public VtkMapper
{
public:
  mitkClassMacro(VolumeDataVtkMapper3D, VtkMapper);

  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

  virtual const mitk::Image* GetInput();

  virtual void EnableMask();
  virtual void DisableMask();
  Image::Pointer GetMask();
  bool SetMask(const Image* mask);
  virtual void UpdateMask();

  static void SetDefaultProperties(mitk::DataNode* node, mitk::BaseRenderer* renderer = NULL, bool overwrite = false);

  /** Returns true if this Mapper currently allows for Level-of-Detail rendering.
   * This reflects whether this Mapper currently invokes StartEvent, EndEvent, and
   * ProgressEvent on BaseRenderer. */
  virtual bool IsLODEnabled( BaseRenderer *renderer = NULL ) const;

  virtual vtkProp *GetVtkProp(mitk::BaseRenderer *renderer);

protected:


  VolumeDataVtkMapper3D();


  virtual ~VolumeDataVtkMapper3D();

  virtual void GenerateDataForRenderer(mitk::BaseRenderer* renderer);

  void CreateDefaultTransferFunctions();

  void UpdateTransferFunctions( mitk::BaseRenderer *renderer );

  void SetPreferences();

  void SetClippingPlane(vtkRenderWindowInteractor* interactor);
  void DelClippingPlane();

  vtkImageShiftScale* m_ImageCast;
  vtkImageChangeInformation* m_UnitSpacingImageFilter;
  vtkVolumeProperty* m_VolumePropertyLow;
  vtkVolumeProperty* m_VolumePropertyMed;
  vtkVolumeProperty* m_VolumePropertyHigh;
  vtkVolumeTextureMapper2D* m_T2DMapper;
  vtkVolumeRayCastMapper* m_HiResMapper;
  vtkImageResample* m_Resampler;

  vtkLODProp3D* m_VolumeLOD;

  vtkCubeSource *m_BoundingBox;
  vtkPolyDataMapper *m_BoundingBoxMapper;
  vtkActor *m_BoundingBoxActor;

  vtkAssembly *m_Prop3DAssembly;

  vtkPlane* m_ClippingPlane;
  vtkImplicitPlaneWidget* m_PlaneWidget;

  vtkImageData *m_Mask;
  vtkImageMask *m_ImageMaskFilter;

  vtkPiecewiseFunction *m_DefaultOpacityTransferFunction;
  vtkPiecewiseFunction *m_DefaultGradientTransferFunction;
  vtkColorTransferFunction *m_DefaultColorTransferFunction;

  int m_LowResID;
  int m_MedResID;
  int m_HiResID;
  bool m_PlaneSet;
  double m_PlaneNormalA;
  double m_PlaneNormalB;
  double m_PlaneNormalC;

  std::set< vtkRenderWindow * > m_RenderWindowInitialized;

};

} // namespace mitk

#endif /* MITKVOLUMEDATAVTKMAPPER3D_H_HEADER_INCLUDED */
