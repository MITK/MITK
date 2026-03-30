/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPointSetVtkMapper3D_h
#define mitkPointSetVtkMapper3D_h

#include <mitkBaseRenderer.h>
#include <mitkVtkMapper.h>
#include <MitkCoreExports.h>
#include <vtkSmartPointer.h>

class vtkActor;
class vtkCellArray;
class vtkPropAssembly;
class vtkAppendPolyData;
class vtkPolyData;
class vtkTubeFilter;
class vtkPolyDataMapper;
class vtkTransformPolyDataFilter;

namespace mitk
{
  class PointSet;

  /**
  * \brief Vtk-based 3D mapper for PointSet.
  *
  * Due to the need of different colors for selected
  * and unselected points and the facts, that we also have a contour and
  * labels for the points, the vtk structure is build up the following way:
  *
  * We have two AppendPolyData, one selected, and one unselected and one
  * for a contour between the points. Each one is connected to an own
  * PolyDataMapper and an Actor. The different color for the unselected and
  * selected state and for the contour is read from properties.
  *
  * "unselectedcolor", "selectedcolor" and "contourcolor" are the strings,
  * that are looked for. Point labels are added besides the selected or the
  * deselected points.
  *
  * Then the three Actors are combined inside a vtkPropAssembly and this
  * object is returned in GetProp() and so hooked up into the rendering
  * pipeline.
  *
  * Properties that can be set for point sets and influence the PointSetVTKMapper3D are:
  *
  *   - \b "color": (ColorProperty*) Color of the point set
  *   - \b "Opacity": (FloatProperty) Opacity of the point set
  *   - \b "show contour": (BoolProperty) If the contour of the points are visible
  *   - \b "contourSizeProp": (FloatProperty) Contour size of the points
  *
  * The default properties are:
  *
  *   - \b "line width": (IntProperty 2)
  *   - \b "pointsize": (FloatProperty 1.0)
  *   - \b "selectedcolor": (ColorProperty (1.0f, 0.0f, 0.0f)) red
  *   - \b "color": (ColorProperty (1.0f, 1.0f, 0.0f)) yellow
  *   - \b "show contour": (BoolProperty false)
  *   - \b "contourcolor": (ColorProperty (1.0f, 0.0f, 0.0f))
  *   - \b "contoursize": (FloatProperty 0.5)
  *   - \b "close contour": (BoolProperty false)
  *   - \b "show points": (BoolProperty true)
  *   - \b "updateDataOnRender": (BoolProperty true)
  *
  * Other properties looked for are:
  *
  *   - \b "show contour": if set to on, lines between the points are shown
  *   - \b "close contour": if set to on, the open strip is closed (first point
  *       connected with last point)
  *   - \b "pointsize": size of the points mapped (diameter of a sphere, in world coordinates!)
  *   - \b "label": text of the Points to show besides points
  *   - \b "contoursize": size of the contour drawn between the points
  *       (if not set, the pointsize is taken)
  *
  * \sa PointSetVtkMapper2D
  * \sa PointSet
  * \ingroup Mapper
  */
  class MITKCORE_EXPORT PointSetVtkMapper3D : public VtkMapper
  {
  public:
    mitkClassMacro(PointSetVtkMapper3D, VtkMapper);

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self);

      /** \brief Returns the PointSet input data object of this mapper.
       *
       * \return The associated PointSet, or nullptr if no data is set.
       */
      virtual const mitk::PointSet *GetInput();

    /** \brief Returns the vtkPropAssembly containing all VTK actors for the given renderer.
     *
     * Overwritten from VtkMapper to return a vtkPropAssembly which is much faster
     * than a vtkAssembly.
     *
     * \param renderer the renderer for which to retrieve the VTK prop.
     * \return The vtkPropAssembly that aggregates all rendering actors.
     */
    vtkProp *GetVtkProp(mitk::BaseRenderer *renderer) override;

    /** \brief Intentionally empty; transform is handled internally.
     *
     * \param renderer the renderer whose transform would be updated.
     */
    void UpdateVtkTransform(mitk::BaseRenderer *renderer) override;

    /** \brief Sets default properties for point set visualization in 3D.
     *
     * \param node the data node to configure.
     * \param renderer the renderer for which properties should be set, or nullptr for global defaults.
     * \param overwrite if true, existing properties will be overwritten.
     */
    static void SetDefaultProperties(mitk::DataNode *node, mitk::BaseRenderer *renderer = nullptr, bool overwrite = false);

    /**
    * \deprecated Use ReleaseGraphicsResources(mitk::BaseRenderer* renderer) instead.
    * \deprecatedSince{2013_12}
    */
    DEPRECATED(void ReleaseGraphicsResources(vtkWindow *renWin));

    /** \brief Releases VTK graphics resources associated with the given renderer.
     *
     * \param renderer the renderer whose resources should be released.
     */
    void ReleaseGraphicsResources(mitk::BaseRenderer *renderer) override;

    LocalStorageHandler<BaseLocalStorage> m_LSH;

  protected:
    /** \brief Constructor. */
    PointSetVtkMapper3D();

    /** \brief Destructor. */
    ~PointSetVtkMapper3D() override;

    /** \brief Generates the VTK rendering data for the given renderer.
     *
     * \param renderer the renderer for which data is generated.
     */
    void GenerateDataForRenderer(mitk::BaseRenderer *renderer) override;

    /** \brief Resets the mapper by hiding the prop assembly.
     *
     * \param renderer the renderer for which the mapper is reset.
     */
    void ResetMapper(BaseRenderer *renderer) override;

    /** \brief Applies color and opacity properties to the given actor.
     *
     * \param renderer the renderer from which to read properties.
     * \param actor the VTK actor to apply properties to.
     */
    virtual void ApplyAllProperties(mitk::BaseRenderer *renderer, vtkActor *actor);

    /** \brief Creates a tube-based contour connecting the given points.
     *
     * \param points the VTK points to connect.
     * \param connections the VTK cell array defining point connectivity.
     */
    virtual void CreateContour(vtkPoints *points, vtkCellArray *connections);

    /** \brief Creates the VTK render objects (spheres, cubes, etc.) for each point. */
    virtual void CreateVTKRenderObjects();

    /// All point positions, already in world coordinates
    vtkSmartPointer<vtkPoints> m_WorldPositions;
    /// All connections between two points (used for contour drawing)
    vtkSmartPointer<vtkCellArray> m_PointConnections;

    vtkSmartPointer<vtkAppendPolyData> m_vtkSelectedPointList;
    vtkSmartPointer<vtkAppendPolyData> m_vtkUnselectedPointList;

    vtkSmartPointer<vtkPoints> m_VtkPoints;
    vtkSmartPointer<vtkCellArray> m_VtkPointConnections;

    vtkSmartPointer<vtkTransformPolyDataFilter> m_VtkPointsTransformer;

    vtkSmartPointer<vtkPolyDataMapper> m_VtkSelectedPolyDataMapper;
    vtkSmartPointer<vtkPolyDataMapper> m_VtkUnselectedPolyDataMapper;

    vtkSmartPointer<vtkActor> m_SelectedActor;
    vtkSmartPointer<vtkActor> m_UnselectedActor;
    vtkSmartPointer<vtkActor> m_ContourActor;

    vtkSmartPointer<vtkPropAssembly> m_PointsAssembly;

    // help for contour between points
    vtkSmartPointer<vtkAppendPolyData> m_vtkTextList;

    // variables to be able to log, how many inputs have been added to PolyDatas
    unsigned int m_NumberOfSelectedAdded;
    unsigned int m_NumberOfUnselectedAdded;

    // variables to check if an update of the vtk objects is needed
    ScalarType m_PointSize;
    ScalarType m_ContourRadius;
  };

} // namespace mitk

#endif
