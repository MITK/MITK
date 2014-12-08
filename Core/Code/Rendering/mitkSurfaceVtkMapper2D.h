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


#ifndef mitkSurfaceVtkMapper2D_h
#define mitkSurfaceVtkMapper2D_h


#include <MitkCoreExports.h>
#include "mitkVtkMapper.h"
#include "mitkBaseRenderer.h"
#include "mitkLocalStorageHandler.h"
#include <vtkSmartPointer.h>
#include <mitkSurface.h>

//class vtkActor;
class vtkPropAssembly;
class vtkCutter;
class vtkPlane;
class vtkLookupTable;
class vtkLinearTransform;
class vtkPKdTree;
class vtkStripper;


namespace mitk {

  class Surface;

  /**
  * @brief Vtk-based 2D mapper for Surfaces
  * @ingroup Mapper
  */
  class MITK_CORE_EXPORT SurfaceVtkMapper2D : public VtkMapper
  {
  public:
    mitkClassMacro(SurfaceVtkMapper2D, VtkMapper);

    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    virtual const mitk::Surface* GetInput() const;

    /** \brief returns the a prop assembly */
    virtual vtkProp* GetVtkProp(mitk::BaseRenderer* renderer);

    /** \brief set the default properties for this mapper */
    static void SetDefaultProperties(mitk::DataNode* node, mitk::BaseRenderer* renderer = NULL, bool overwrite = false);

    /** \brief Internal class holding the mapper, actor, etc. for each of the 3 2D render windows */
    class LocalStorage : public mitk::Mapper::BaseLocalStorage
    {
    public:
      /** \brief Timestamp of last update of stored data. */
      itk::TimeStamp m_LastUpdateTime;

      vtkSmartPointer<vtkPropAssembly> m_PropAssembly;
      vtkSmartPointer<vtkActor> m_Actor;
      vtkSmartPointer<vtkPolyDataMapper> m_Mapper;
      /** \brief Actor of a 2D render window. */
      vtkSmartPointer<vtkPolyData> m_PolyData;
      vtkSmartPointer<vtkCutter> m_Cutter;
      vtkSmartPointer<vtkPlane> m_CuttingPlane;

      /** \brief Default constructor of the local storage. */
      LocalStorage();
      /** \brief Default deconstructor of the local storage. */
      ~LocalStorage();
    };

    /** \brief The LocalStorageHandler holds all (three) LocalStorages for the three 2D render windows. */
    mitk::LocalStorageHandler<LocalStorage> m_LSH;


  protected:

    /* constructor */
    SurfaceVtkMapper2D();

    /* destructor */
    virtual ~SurfaceVtkMapper2D();

    /* \brief Applies the color and opacity properties and calls CreateVTKRenderObjects */
    virtual void GenerateDataForRenderer(mitk::BaseRenderer* renderer);
    /* \brief Called in mitk::Mapper::Update
    * If TimeSlicedGeometry or time step is not valid of point set: reset mapper so that nothing is
    * displayed e.g. toggle visiblity of the propassembly */
    virtual void ResetMapper( BaseRenderer* renderer );

    void ApplyAllProperties(mitk::BaseRenderer *renderer);

    void Update(mitk::BaseRenderer *renderer);

    int m_LineWidth;

    bool m_DrawNormals;

    float m_FrontSideColor[4];
    float m_BackSideColor[4];
    float m_LineColor[4];
    float m_FrontNormalLengthInPixels;
    float m_BackNormalLengthInPixels;
  };


} // namespace mitk

#endif /* mitkSurfaceVtkMapper2D_h */
