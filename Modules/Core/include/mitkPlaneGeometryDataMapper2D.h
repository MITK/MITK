/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPlaneGeometryDataMapper2D_h
#define mitkPlaneGeometryDataMapper2D_h

#include <mitkBaseRenderer.h>
#include <mitkVtkMapper.h>
#include <MitkCoreExports.h>
#include <vtkSmartPointer.h>

class vtkActor2D;
class vtkPropAssembly;
class vtkCellArray;
class vtkPolyDataMapper2D;

namespace mitk
{
  /**
    * \brief VTK-based 2D mapper for rendering a crosshair with the plane geometry.
    *
    * This mapper uses the mitkPlaneGeometryData from the three helper objects in
    * the StdMultiWidget to render a crosshair in all 2D render windows. The crosshair
    * is assembled as lines and rendered with a vtkPolyDataMapper. The mapper
    * requires multiple plane geometry to compute the correct crosshair position.
    * The plane bounds are computed using either ReferenceGeometry if it is present or
    * the plane geometry itself otherwise.
    * The mapper offers the following properties:
    * \b Crosshair.Line width: The thickness of the crosshair.
    * \b Crosshair.Gap Size: The gap between the lines in pixels.
    * \b Crosshair.Orientation Decoration: Adds a PlaneOrientationProperty, which
    * indicates the direction of the plane normal. See mitkPlaneOrientationProperty.
    *
    * \ingroup Mapper
    * \sa PlaneGeometryData
    * \sa PlaneOrientationProperty
    */
  class MITKCORE_EXPORT PlaneGeometryDataMapper2D : public VtkMapper
  {
  public:
    mitkClassMacro(PlaneGeometryDataMapper2D, VtkMapper);

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self);

      /** \brief Get the PlaneGeometryData input of this mapper. */
      virtual const mitk::PlaneGeometryData *GetInput() const;

    /**
     * \brief Get the VTK prop assembly for the given renderer.
     * \param renderer The renderer for which to return the VTK prop.
     * \return The assembled vtkProp containing the crosshair visualization.
     */
    vtkProp *GetVtkProp(mitk::BaseRenderer *renderer) override;

    /**
     * \brief Apply all visual properties specific to this mapper.
     * \param renderer The renderer whose properties to apply.
     */
    virtual void ApplyAllProperties(BaseRenderer *renderer);

    /**
     * \brief Set the default properties for this mapper on the given node.
     * \param node The data node on which to set default properties.
     * \param renderer The renderer context. If nullptr, properties apply to all renderers.
     * \param overwrite If true, existing properties are overwritten.
     */
    static void SetDefaultProperties(mitk::DataNode *node, mitk::BaseRenderer *renderer = nullptr, bool overwrite = false);

    /** \brief Internal class holding the mapper, actor, etc. for each of the 3 2D render windows */
    class LocalStorage : public mitk::Mapper::BaseLocalStorage
    {
    public:
      /* constructor */
      LocalStorage();

      /* destructor */
      ~LocalStorage() override;

      // actor
      vtkSmartPointer<vtkActor2D> m_CrosshairActor;
      vtkSmartPointer<vtkActor2D> m_CrosshairHelperLineActor;
      vtkSmartPointer<vtkActor2D> m_ArrowActor;
      vtkSmartPointer<vtkPolyDataMapper2D> m_HelperLinesmapper;
      vtkSmartPointer<vtkPolyDataMapper2D> m_Arrowmapper;
      vtkSmartPointer<vtkPolyDataMapper2D> m_Mapper;
      vtkSmartPointer<vtkPropAssembly> m_CrosshairAssembly;
    };

    /** \brief The LocalStorageHandler holds all (three) LocalStorages for the three 2D render windows. */
    mitk::LocalStorageHandler<LocalStorage> m_LSH;

  protected:
    /* constructor */
    PlaneGeometryDataMapper2D();

    /* destructor */
    ~PlaneGeometryDataMapper2D() override;

    /** \brief Apply color and opacity properties, then create VTK render objects. */
    void GenerateDataForRenderer(mitk::BaseRenderer *renderer) override;

    /** \brief Create the VTK crosshair representation for the given renderer. */
    void CreateVtkCrosshair(BaseRenderer *renderer);

    /** \brief Test if a point lies within the given PlaneGeometry bounds. */
    static bool TestPointInPlaneGeometry(const PlaneGeometry *planeGeometry, const Point3D &point);

    /** \brief Test if a point lies within the given reference geometry bounds. */
    static bool TestPointInReferenceGeometry(const BaseGeometry *referenceGeometry, const Point3D &point);

    /** \brief Clip a crosshair line segment to the bounds of the given PlaneGeometry. */
    static bool CutCrossLineWithPlaneGeometry(const PlaneGeometry *planeGeometry, Line3D &crossLine);

    /** \brief Clip a crosshair line segment to the bounds of the given reference geometry. */
    static bool CutCrossLineWithReferenceGeometry(const BaseGeometry *referenceGeometry, Line3D &crossLine);

    /** \brief Add a line segment between two 3D points to VTK data structures. */
    void DrawLine(Point3D p0, Point3D p1, vtkCellArray *lines, vtkPoints *points);

    // member variables holding the current value of the properties used in this mapper
    typedef std::vector<DataNode *> NodesVectorType;
    NodesVectorType m_OtherPlaneGeometries;

    typedef std::set<Self *> AllInstancesContainer;
    static AllInstancesContainer s_AllInstances;

    bool m_RenderOrientationArrows;
    bool m_ArrowOrientationPositive;

    void ApplyColorAndOpacityProperties2D(BaseRenderer *renderer, vtkActor2D *actor);
    void DrawOrientationArrow(vtkSmartPointer<vtkCellArray> triangles,
                              vtkSmartPointer<vtkPoints> triPoints,
                              double triangleSizeMM,
                              Vector3D &orthogonalVector,
                              Point3D &point1,
                              Point3D &point2);
  };
} // namespace mitk
#endif
