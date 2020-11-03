/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITKVtkGLMapperWrapper_H_HEADER
#define MITKVtkGLMapperWrapper_H_HEADER

#include "mitkBaseRenderer.h"
#include "mitkGLMapper.h"
#include "mitkLocalStorageHandler.h"
#include "mitkVtkMapper.h"
#include <MitkLegacyGLExports.h>
#include <vtkSmartPointer.h>

class vtkGLMapperProp;

namespace mitk
{
  /**
  * @brief Vtk-based 2D mapper for PointSet
  */
  class MITKLEGACYGL_EXPORT VtkGLMapperWrapper : public VtkMapper
  {
  public:
    mitkClassMacro(VtkGLMapperWrapper, VtkMapper);

    mitkNewMacro1Param(Self, GLMapper::Pointer);

    itkCloneMacro(Self);

    /** \brief returns the a prop assembly */
    vtkProp *GetVtkProp(mitk::BaseRenderer *renderer) override;

    void GenerateDataForRenderer(mitk::BaseRenderer *renderer) override;

    /** \brief Internal class holding the mapper, actor, etc. for each of the 3 2D render windows */
    class LocalStorage : public mitk::Mapper::BaseLocalStorage
    {
    public:
      /* constructor */
      LocalStorage();

      /* destructor */
      ~LocalStorage() override;
      vtkSmartPointer<vtkGLMapperProp> m_GLMapperProp;
    };

    void ApplyColorAndOpacityProperties(mitk::BaseRenderer *renderer, vtkActor *actor) override;

    void MitkRender(mitk::BaseRenderer *renderer, mitk::VtkPropRenderer::RenderType type) override;

    void Update(BaseRenderer *renderer) override;

    void SetDataNode(DataNode *node) override;

    DataNode *GetDataNode() const override;

    /** \brief The LocalStorageHandler holds all (three) LocalStorages for the three 2D render windows. */
    mitk::LocalStorageHandler<LocalStorage> m_LSH;

  protected:
    GLMapper::Pointer m_MitkGLMapper;
    /* constructor */
    VtkGLMapperWrapper(GLMapper::Pointer mitkGLMapper);

    /* destructor */
    ~VtkGLMapperWrapper() override;
    void Enable2DOpenGL(mitk::BaseRenderer *);
    void Disable2DOpenGL();
  };

} // namespace mitk

#endif /* MITKVtkGLMapperWrapper_H_HEADER_INCLUDED_C1902626 */
