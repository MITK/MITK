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
class vtkAssembly;
class vtkCutter;
class vtkPlane;
class vtkLookupTable;
class vtkGlyph3D;
class vtkArrowSource;
class vtkReverseSense;
class vtkSphereTree;

namespace mitk {

class Surface;

/**
  * @brief Vtk-based mapper for cutting 2D slices out of Surfaces.
  *
  * The mapper uses a vtkCutter filter to cut out slices (contours) of the 3D
  * volume and render these slices as vtkPolyData. The data is transformed
  * according to its geometry before cutting, to support the geometry concept
  * of MITK.
  *
  * Properties:
  * \b Surface.2D.Line Width: Thickness of the rendered lines in 2D.
  * \b Surface.2D.Normals.Draw Normals: enables drawing of normals as 3D arrows
  * in the 2D render window. The normals are created with a vtkGlyph3D from
  * the vtkPolyData.
  * \b Surface.2D.Normals.Draw Inverse Normals: same as normals, but in the
  * other direction. The inverse normals are computed with a vtkReverseSense
  * filter.
  * \b Surface.2D.Normals.(Inverse) Normals Color: Color of the (inverse) normals.
  * \b Surface.2D.Normals.(Inverse) Normals Scale Factor: Regulates the size of the normals.
  *
  * @ingroup Mapper
  */
class MITKCORE_EXPORT SurfaceVtkMapper2D : public VtkMapper
{
public:
  mitkClassMacro(SurfaceVtkMapper2D, VtkMapper);

  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

  virtual const mitk::Surface* GetInput() const;

  /** \brief returns the prop assembly */
  virtual vtkProp* GetVtkProp(mitk::BaseRenderer* renderer) override;

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
       *
       * Consists of 3 actors:
       * The surface cut (the slice from the 3D surface).
       * The normals and the inverse normals.
       */
    vtkSmartPointer<vtkAssembly> m_PropAssembly;

    /**
     * @brief m_Actor actor for the surface cut.
     */
    vtkSmartPointer<vtkActor> m_Actor;
    /**
       * @brief m_Mapper VTK mapper for all types of 2D polydata e.g. werewolves.
       */
    vtkSmartPointer<vtkOpenGLPolyDataMapper> m_Mapper;

    vtkSmartPointer<vtkSphereTree> m_SphereTree;
    /**
       * @brief m_CuttingPlane The plane where to cut off the 2D slice.
       */
    vtkSmartPointer<vtkPlane> m_CuttingPlane;

    vtkSmartPointer<vtkPolyData> m_EmptyPolyData;

    /** \brief Default constructor of the local storage. */
    LocalStorage();
    /** \brief Default deconstructor of the local storage. */
    ~LocalStorage();
  };

  /** \brief The LocalStorageHandler holds all (three) LocalStorages for the three 2D render windows. */
  mitk::LocalStorageHandler<LocalStorage> m_LSH;

  /**
   * @brief UpdateVtkTransform Overwrite the method of the base class.
   *
   * The base class transforms the actor according to the respective
   * geometry which is correct for most cases. This mapper, however,
   * uses a vtkCutter to cut out a contour. To cut out the correct
   * contour, the data has to be transformed beforehand. Else the
   * current plane geometry will point the cutter to en empty location
   * (if the surface does have a geometry, which is a rather rare case).
   */
  void UpdateVtkTransform(mitk::BaseRenderer* /*renderer*/) override
  {
  }

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
  virtual void GenerateDataForRenderer(mitk::BaseRenderer* renderer) override;

  /**
     * @brief ResetMapper Called in mitk::Mapper::Update to hide objects.
     * If TimeSlicedGeometry or time step is not valid, reset the mapper.
     * so that nothing is displayed e.g. toggle visiblity of the propassembly.
     *
     * @param renderer The respective renderer of the mitkRenderWindow.
     */
  virtual void ResetMapper( BaseRenderer* renderer ) override;

  /**
     * @brief ApplyAllProperties Pass all the properties to VTK.
     * @param renderer The respective renderer of the mitkRenderWindow.
     */
  void ApplyAllProperties( BaseRenderer* renderer);

  /**
     * @brief Update Check if data should be generated.
     * @param renderer The respective renderer of the mitkRenderWindow.
     */
  void Update(BaseRenderer* renderer) override;
};
} // namespace mitk
#endif /* mitkSurfaceVtkMapper2D_h */
