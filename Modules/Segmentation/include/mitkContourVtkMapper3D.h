/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkContourVtkMapper3D_h
#define mitkContourVtkMapper3D_h

#include <mitkVtkMapper.h>
#include <MitkSegmentationExports.h>

#include <vtkPolyData.h>
#include <vtkSmartPointer.h>

class vtkPolyDataMapper;
class vtkAppendPolyData;
class vtkActor;
class vtkTubeFilter;

namespace mitk
{
  class BaseRenderer;
  class Contour;

  /**
   * \brief VTK-based mapper for rendering a mitk::Contour in 3D.
   * \ingroup Mapper
   * \sa Contour
   */
  class MITKSEGMENTATION_EXPORT ContourVtkMapper3D : public VtkMapper
  {
  public:
    mitkClassMacro(ContourVtkMapper3D, VtkMapper);

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self);

      virtual const mitk::Contour *GetInput();

    vtkProp *GetVtkProp(mitk::BaseRenderer *renderer) override;

  protected:
    ContourVtkMapper3D();
    ~ContourVtkMapper3D() override;

    void GenerateDataForRenderer(mitk::BaseRenderer *renderer) override;

    vtkSmartPointer<vtkPolyDataMapper> m_VtkPolyDataMapper;
    vtkSmartPointer<vtkTubeFilter> m_TubeFilter;

    vtkSmartPointer<vtkAppendPolyData> m_VtkPointList;
    vtkSmartPointer<vtkPolyData> m_Contour;
    vtkSmartPointer<vtkActor> m_Actor;
  };

} // namespace mitk

#endif
