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

//VTK
#include <vtkSmartPointer.h>
class vtkPropAssembly;
class vtkCutter;
class vtkPlane;
class vtkLookupTable;

namespace mitk {

  class Surface;

  /**
  * @brief Vtk-based mapper for cutting 2D slices out of Surfaces.
  * @ingroup Mapper
  */
  class MITK_CORE_EXPORT SurfaceVtkMapper2D : public VtkMapper
  {
  public:
    mitkClassMacro(SurfaceVtkMapper2D, VtkMapper);

    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    virtual const mitk::Surface* GetInput() const;

    /** \brief returns the prop assembly */
    virtual vtkProp* GetVtkProp(mitk::BaseRenderer* renderer);

    /** \brief set the default properties for this mapper */
    static void SetDefaultProperties(mitk::DataNode* node, mitk::BaseRenderer* renderer = NULL, bool overwrite = false);

    /** \brief Internal class holding the mapper, actor, etc. for each of the 3 2D render windows */
    class LocalStorage : public mitk::Mapper::BaseLocalStorage
    {
    public:
      /** \brief Timestamp of last update of stored data. */
      itk::TimeStamp m_LastUpdateTime;
      /**
       * @brief m_PropAssembly Contains all vtkProps for the final rendering.
       */
      vtkSmartPointer<vtkPropAssembly> m_PropAssembly;
      /**
       * \brief Actor of a 2D render window.
      */
      vtkSmartPointer<vtkActor> m_Actor;
      /**
       * @brief m_Mapper VTK mapper for all types of 2D polydata e.g. werewolves.
       */
      vtkSmartPointer<vtkPolyDataMapper> m_Mapper;
      /**
       * @brief m_Cutter Filter to cut out the 2D slice.
       */
      vtkSmartPointer<vtkCutter> m_Cutter;
      /**
       * @brief m_CuttingPlane The plane where to cut off the 2D slice.
       */
      vtkSmartPointer<vtkPlane> m_CuttingPlane;

      /** \brief Default constructor of the local storage. */
      LocalStorage();
      /** \brief Default deconstructor of the local storage. */
      ~LocalStorage();
    };

    /** \brief The LocalStorageHandler holds all (three) LocalStorages for the three 2D render windows. */
    mitk::LocalStorageHandler<LocalStorage> m_LSH;


  protected:
    /**
     * @brief SurfaceVtkMapper2D default constructor.
     */
    SurfaceVtkMapper2D();

    /**
     * @brief ~SurfaceVtkMapper2D default destructor.
     */
    virtual ~SurfaceVtkMapper2D();

    /**
     * @brief GenerateDataForRenderer produces all the data.
     * @param renderer The respective renderer of the mitkRenderWindow.
     */
    virtual void GenerateDataForRenderer(mitk::BaseRenderer* renderer);

    /**
     * @brief ResetMapper Called in mitk::Mapper::Update to hide objects.
     * If TimeSlicedGeometry or time step is not valid, reset the mapper.
     * so that nothing is displayed e.g. toggle visiblity of the propassembly.
     *
     * @param renderer The respective renderer of the mitkRenderWindow.
     */
    virtual void ResetMapper( BaseRenderer* renderer );

    /**
     * @brief ApplyAllProperties Pass all the properties to VTK.
     * @param renderer The respective renderer of the mitkRenderWindow.
     */
    void ApplyAllProperties( BaseRenderer* renderer);

    /**
     * @brief Update Check if data should be generated.
     * @param renderer The respective renderer of the mitkRenderWindow.
     */
    void Update(BaseRenderer* renderer);

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
