/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkCrosshairVtkMapper2D_h
#define mitkCrosshairVtkMapper2D_h

#include <MitkCoreExports.h>

#include <mitkBaseRenderer.h>
#include <mitkCrosshairData.h>
#include <mitkVtkMapper.h>

#include <vtkSmartPointer.h>

class vtkActor2D;
class vtkCellArray;
class vtkPropAssembly;
class vtkPolyDataMapper2D;

namespace mitk
{
  /**
    * @brief Vtk-based 2D mapper for rendering a crosshair using vtk mapper.
    *
    *        CrosshairData is used to retrieve the selected position, which is stored
    *        in the processed data node.
    *        The crosshair is created by drawing lines through the selected position,
    *        adding an additional gap in the center of the crosshair and creating vtk poly
    *        data from the created points and lines.
    */
  class MITKCORE_EXPORT CrosshairVtkMapper2D : public VtkMapper
  {
  public:

    mitkClassMacro(CrosshairVtkMapper2D, VtkMapper);
    itkFactorylessNewMacro(Self);

    const CrosshairData* GetInput() const;

    /** \brief Checks whether this mapper needs to update itself and generate data. */
    void Update(mitk::BaseRenderer* renderer) override;

    vtkProp* GetVtkProp(BaseRenderer* renderer) override;

    static void SetDefaultProperties(DataNode* node, BaseRenderer* renderer = nullptr, bool overwrite = false);

  protected:

    CrosshairVtkMapper2D();
    ~CrosshairVtkMapper2D() override;

    /** \brief Internal class holding the mapper, actor, etc. for each of the 2D render windows */
    class LocalStorage : public Mapper::BaseLocalStorage
    {
    public:

      LocalStorage();
      ~LocalStorage() override;

      // actor
      vtkSmartPointer<vtkActor2D> m_CrosshairActor;
      vtkSmartPointer<vtkPolyDataMapper2D> m_Mapper;
      vtkSmartPointer<vtkPropAssembly> m_CrosshairAssembly;
    };

    /** \brief The LocalStorageHandler holds all LocalStorages for the 2D render windows. */
    LocalStorageHandler<LocalStorage> m_LSH;

    /* \brief Applies the color and opacity properties and calls CreateVTKRenderObjects */
    void GenerateDataForRenderer(BaseRenderer *renderer) override;

    void CreateVtkCrosshair(BaseRenderer *renderer);

    void DrawLine(Point3D p0, Point3D p1, vtkCellArray* lines, vtkPoints* points);

    void ApplyAllProperties(BaseRenderer* renderer);
    void ApplyColorAndOpacityProperties2D(BaseRenderer* renderer, vtkActor2D* actor);

    const int defaultGapSize;
  };

}

#endif
