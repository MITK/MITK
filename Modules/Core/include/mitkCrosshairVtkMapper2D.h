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
   * \brief VTK-based 2D mapper for rendering a crosshair.
   *
   * CrosshairData is used to retrieve the selected position, which is stored
   * in the processed data node. The crosshair is created by drawing lines
   * through the selected position, adding an additional gap in the center
   * of the crosshair, and creating VTK poly data from the created points
   * and lines.
   *
   * \sa CrosshairData
   * \sa CrosshairManager
   */
  class MITKCORE_EXPORT CrosshairVtkMapper2D : public VtkMapper
  {
  public:

    mitkClassMacro(CrosshairVtkMapper2D, VtkMapper);
    itkFactorylessNewMacro(Self);

    /**
     * \brief Get the input CrosshairData from the data node.
     *
     * \return The CrosshairData associated with this mapper's data node.
     */
    const CrosshairData* GetInput() const;

    /**
     * \brief Check whether this mapper needs to update itself and generate data.
     *
     * Skips rendering if the data node is not visible in the given renderer
     * or if the data is not of type CrosshairData.
     *
     * \param[in] renderer  The renderer to update for.
     */
    void Update(mitk::BaseRenderer* renderer) override;

    /**
     * \brief Get the VTK prop assembly for the given renderer.
     *
     * \param[in] renderer  The renderer whose local storage provides the prop.
     * \return The vtkPropAssembly containing the crosshair actor.
     */
    vtkProp* GetVtkProp(BaseRenderer* renderer) override;

    /**
     * \brief Set default properties for the crosshair data node.
     *
     * Sets "Line width" and "Crosshair.Gap Size" properties.
     *
     * \param[in] node       The data node to set properties on.
     * \param[in] renderer   The renderer for renderer-specific properties. May be nullptr.
     * \param[in] overwrite  If true, existing properties are overwritten.
     */
    static void SetDefaultProperties(DataNode* node, BaseRenderer* renderer = nullptr, bool overwrite = false);

  protected:

    /** \brief Constructor. Initializes the default gap size. */
    CrosshairVtkMapper2D();

    /** \brief Destructor. */
    ~CrosshairVtkMapper2D() override;

    /**
     * \brief Internal class holding the mapper, actor, etc. for each of the 2D render windows.
     */
    class LocalStorage : public Mapper::BaseLocalStorage
    {
    public:

      /** \brief Constructor. Creates the VTK actor, mapper, and assembly. */
      LocalStorage();

      /** \brief Destructor. */
      ~LocalStorage() override;

      vtkSmartPointer<vtkActor2D> m_CrosshairActor;       ///< \brief The crosshair line actor.
      vtkSmartPointer<vtkPolyDataMapper2D> m_Mapper;       ///< \brief The poly data mapper.
      vtkSmartPointer<vtkPropAssembly> m_CrosshairAssembly; ///< \brief The prop assembly grouping actors.
    };

    /** \brief The LocalStorageHandler holds all LocalStorages for the 2D render windows. */
    LocalStorageHandler<LocalStorage> m_LSH;

    /**
     * \brief Apply color and opacity properties and create the VTK crosshair geometry.
     *
     * \param[in] renderer  The renderer to generate data for.
     */
    void GenerateDataForRenderer(BaseRenderer *renderer) override;

    /**
     * \brief Create the VTK crosshair poly data for the given renderer.
     *
     * Converts the 3D crosshair position to 2D display coordinates,
     * then draws four line segments with a central gap.
     *
     * \param[in] renderer  The renderer providing coordinate transformations.
     */
    void CreateVtkCrosshair(BaseRenderer *renderer);

    /**
     * \brief Draw a line between two 3D points and add it to the given VTK structures.
     *
     * \param[in]     p0      The start point.
     * \param[in]     p1      The end point.
     * \param[in,out] lines   The cell array to add the line to.
     * \param[in,out] points  The points array to add the endpoints to.
     */
    void DrawLine(Point3D p0, Point3D p1, vtkCellArray* lines, vtkPoints* points);

    /**
     * \brief Apply all visual properties (color, opacity, line width) to the crosshair actor.
     *
     * \param[in] renderer  The renderer providing property context.
     */
    void ApplyAllProperties(BaseRenderer* renderer);

    /**
     * \brief Apply color and opacity properties to a 2D actor.
     *
     * \param[in]     renderer  The renderer providing property context.
     * \param[in,out] actor     The vtkActor2D to apply properties to.
     */
    void ApplyColorAndOpacityProperties2D(BaseRenderer* renderer, vtkActor2D* actor);

    const int defaultGapSize;  ///< \brief The default gap size in pixels for the crosshair center.
  };

}

#endif
