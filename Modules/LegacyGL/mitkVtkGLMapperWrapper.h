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


#ifndef MITKVtkGLMapperWrapper_H_HEADER
#define MITKVtkGLMapperWrapper_H_HEADER


#include <MitkLegacyGLExports.h>
#include "mitkVtkMapper.h"
#include "mitkBaseRenderer.h"
#include "mitkLocalStorageHandler.h"
#include <vtkSmartPointer.h>
#include "mitkGLMapper.h"

class vtkGLMapperProp;

namespace mitk {

  /**
  * @brief Vtk-based 2D mapper for PointSet
  */
  class MITKLEGACYGL_EXPORT VtkGLMapperWrapper : public VtkMapper
  {
  public:
    mitkClassMacro(VtkGLMapperWrapper, VtkMapper);

    mitkNewMacro1Param(Self,GLMapper::Pointer)

    itkCloneMacro(Self)

    /** \brief returns the a prop assembly */
    virtual vtkProp* GetVtkProp(mitk::BaseRenderer* renderer) override;

    virtual void GenerateDataForRenderer(mitk::BaseRenderer* renderer) override;

    /** \brief Internal class holding the mapper, actor, etc. for each of the 3 2D render windows */
    class LocalStorage : public mitk::Mapper::BaseLocalStorage
    {
    public:
      /* constructor */
      LocalStorage();

      /* destructor */
      ~LocalStorage();
      vtkSmartPointer<vtkGLMapperProp> m_GLMapperProp;
    };

    virtual void ApplyColorAndOpacityProperties(mitk::BaseRenderer* renderer, vtkActor * actor) override;

    void MitkRender(mitk::BaseRenderer* renderer, mitk::VtkPropRenderer::RenderType type) override;

    virtual void Update(BaseRenderer *renderer) override;

    virtual void SetDataNode(DataNode* node) override;

    virtual DataNode* GetDataNode() const override;

    /** \brief The LocalStorageHandler holds all (three) LocalStorages for the three 2D render windows. */
    mitk::LocalStorageHandler<LocalStorage> m_LSH;

  protected:


    GLMapper::Pointer m_MitkGLMapper;
    /* constructor */
    VtkGLMapperWrapper(GLMapper::Pointer mitkGLMapper);

    /* destructor */
    virtual ~VtkGLMapperWrapper();
    void Enable2DOpenGL(mitk::BaseRenderer *renderer);
    void Disable2DOpenGL();
  };

} // namespace mitk

#endif /* MITKVtkGLMapperWrapper_H_HEADER_INCLUDED_C1902626 */
