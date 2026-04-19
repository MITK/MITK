/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkUnstructuredGridVtkMapper3D_h
#define mitkUnstructuredGridVtkMapper3D_h

#include <MitkMapperExtExports.h>
#include <mitkBaseRenderer.h>
#include <mitkCommon.h>
#include <mitkUnstructuredGrid.h>
#include <mitkVtkMapper.h>

#include <vtkActor.h>
#include <vtkAssembly.h>
#include <vtkDataSetTriangleFilter.h>
#include <vtkVolume.h>

#include <vtkUnstructuredGridMapper.h>
#include <vtkUnstructuredGridVolumeRayCastMapper.h>

#include <vtkProjectedTetrahedraMapper.h>
#include <vtkUnstructuredGridVolumeZSweepMapper.h>

namespace mitk
{
  /** \brief VTK-based mapper for rendering mitk::UnstructuredGrid in 3D.
   *
   * Supports multiple rendering modes including surface, wireframe, and volume
   * rendering (via ray casting, projected tetrahedra, or z-sweep mappers).
   * The rendering mode and visual properties are controlled through DataNode properties.
   *
   * \sa UnstructuredGridMapper2D, UnstructuredGrid, VtkMapper
   * \ingroup Mapper
   */
  class MITKMAPPEREXT_EXPORT UnstructuredGridVtkMapper3D : public VtkMapper
  {
  public:
    mitkClassMacro(UnstructuredGridVtkMapper3D, VtkMapper);

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self);

    /** \brief Get the UnstructuredGrid input data.
     *
     * \return The UnstructuredGrid associated with this mapper's data node.
     */
    virtual const mitk::UnstructuredGrid *GetInput();

    /** \brief Get the VTK prop assembly for 3D rendering.
     *
     * \param[in] renderer The renderer context.
     * \return The VTK prop assembly containing the grid actors and volume.
     */
    vtkProp *GetVtkProp(mitk::BaseRenderer *renderer) override;

    /** \brief Set default rendering properties for unstructured grid 3D visualization.
     *
     * \param[in] node      The data node to configure.
     * \param[in] renderer  The renderer context, or \c nullptr for all renderers.
     * \param[in] overwrite If \c true, overwrite existing properties.
     */
    static void SetDefaultProperties(mitk::DataNode *node, mitk::BaseRenderer *renderer = nullptr, bool overwrite = false);

    LocalStorageHandler<BaseLocalStorage> m_LSH;

  protected:
    UnstructuredGridVtkMapper3D();

    ~UnstructuredGridVtkMapper3D() override;

    void GenerateDataForRenderer(mitk::BaseRenderer *renderer) override;
    void ResetMapper(BaseRenderer * /*renderer*/) override;

    void SetProperties(mitk::BaseRenderer *renderer);

    /** \brief Apply visual properties to this mapper's actors. */
    void ApplyProperties(mitk::BaseRenderer *renderer);

    vtkAssembly *m_Assembly;
    vtkActor *m_Actor;
    vtkActor *m_ActorWireframe;
    vtkVolume *m_Volume;

    vtkDataSetTriangleFilter *m_VtkTriangleFilter;

    vtkUnstructuredGridMapper *m_VtkDataSetMapper;
    vtkUnstructuredGridMapper *m_VtkDataSetMapper2;

    vtkUnstructuredGridVolumeRayCastMapper *m_VtkVolumeRayCastMapper;
    vtkProjectedTetrahedraMapper *m_VtkPTMapper;
    vtkUnstructuredGridVolumeZSweepMapper *m_VtkVolumeZSweepMapper;
  };

} // namespace mitk

#endif
