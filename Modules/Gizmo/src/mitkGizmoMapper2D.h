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

#ifndef mitkGizmoMapper2D_h
#define mitkGizmoMapper2D_h

#include <mitkVtkMapper.h>

#include <vtkPainterPolyDataMapper.h>

namespace mitk
{

// forward declaration
class Gizmo;

//! 2D Mapper for mitk::Gimzo
class GizmoMapper2D : public mitk::VtkMapper
{
public:

    mitkClassMacro(GizmoMapper2D, VtkMapper);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    //! Provides given node with a set of default properties.
    //!
    //! \param node The DataNode to decorate with properties.
    //! \param renderer When not nullptr, generate specific properties for given renderer
    //! \param overwrite Whether already existing properties shall be overwritten.
    static void SetDefaultProperties(mitk::DataNode* node,
                                     mitk::BaseRenderer* renderer = nullptr,
                                     bool overwrite = false);

    //! Return the vtkProp that represents the "rendering result".
    vtkProp* GetVtkProp(mitk::BaseRenderer* renderer) override
    {
        return m_LSH.GetLocalStorage(renderer)->m_Actor;
    }

    //! "Resets" the mapper, setting its result to invisible.
    void ResetMapper(mitk::BaseRenderer* renderer) override;

    vtkPolyData* GetVtkPolyData(mitk::BaseRenderer* renderer);

private:

    //! Provide the mapping input as a specific RawMesh_C
    const Gizmo* GetInput();

    //! Update the vtkProp, i.e. the contours of a slice through our RawMesh_C.
    virtual void GenerateDataForRenderer(mitk::BaseRenderer* renderer) override;

    //! (RenderWindow) Instance specific data.
    class LocalStorage : public Mapper::BaseLocalStorage
    {
    public:

        //! The overall rendering result.
        vtkSmartPointer<vtkActor> m_Actor;

        //! The mapper of the resulting vtkPolyData
        //! (3D polygons for 3D mapper, 2D contours for 2D mapper)
        vtkSmartPointer<vtkPainterPolyDataMapper> m_VtkPolyDataMapper;

        //! Last time this storage has been updated.
        itk::TimeStamp m_LastUpdateTime;

        LocalStorage();
    };

    //! (RenderWindow) Instance specific data.
    LocalStorageHandler<LocalStorage> m_LSH;
};

} // namespace

#endif
