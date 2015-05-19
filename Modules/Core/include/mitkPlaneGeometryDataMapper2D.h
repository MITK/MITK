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

#ifndef mitkPlaneGeometryDataMapper2D_h
#define mitkPlaneGeometryDataMapper2D_h

#include "mitkVtkMapper.h"
#include "mitkBaseRenderer.h"
#include <MitkCoreExports.h>
#include <mitkWeakPointer.h>
#include <vtkSmartPointer.h>

class vtkActor2D;
class vtkPropAssembly;
class vtkFloatArray;
class vtkCellArray;
class vtkPolyDataMapper2D;

namespace mitk {

/**
  * @brief Vtk-based 2D mapper for rendering a crosshair with the plane geometry.
  *
  * This mapper uses the mitkPlaneGeometryData from the three helper objects in
  * the StdMultiWidget to render a crosshair in all 2D render windows. The crosshair
  * is assembled as lines and rendered with a vtkPolyDataMapper. The mapper
  * requires multiple plane geometry to compute the correct crosshair position.
  * The mapper offers the following properties:
  * \b Crosshair.Line width: The thickness of the crosshair.
  * \b Crosshair.Gap Size: The gap between the lines in pixels.
  * \b Crosshair.Orientation Decoration: Adds a PlaneOrientationProperty, which
  * indicates the direction of the plane normal. See mitkPlaneOrientationProperty.
  *
  * @ingroup Mapper
  */
class MITKCORE_EXPORT PlaneGeometryDataMapper2D : public VtkMapper
{
public:
  mitkClassMacro(PlaneGeometryDataMapper2D, VtkMapper);

  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

  virtual const mitk::PlaneGeometryData* GetInput() const;

  /** \brief returns the a prop assembly */
  virtual vtkProp* GetVtkProp(mitk::BaseRenderer* renderer) override;

  /** Applies properties specific to this mapper */
  virtual void ApplyAllProperties( BaseRenderer *renderer );

  virtual void UpdateVtkTransform(mitk::BaseRenderer *renderer) override;

  /** \brief set the default properties for this mapper */
  static void SetDefaultProperties(mitk::DataNode* node, mitk::BaseRenderer* renderer = NULL, bool overwrite = false);

  /** \brief Internal class holding the mapper, actor, etc. for each of the 3 2D render windows */
  class LocalStorage : public mitk::Mapper::BaseLocalStorage
  {

  public:

    /* constructor */
    LocalStorage();

    /* destructor */
    ~LocalStorage();

    // actor
    vtkSmartPointer<vtkActor2D> m_CrosshairActor;
    vtkSmartPointer<vtkActor2D> m_CrosshairHelperLineActor;
    vtkSmartPointer<vtkPropAssembly> m_CrosshairAssembly;
    vtkSmartPointer<vtkPolyDataMapper2D> m_HelperLinesmapper;
    vtkSmartPointer<vtkPolyDataMapper2D> m_Mapper;
  };

  /** \brief The LocalStorageHandler holds all (three) LocalStorages for the three 2D render windows. */
  mitk::LocalStorageHandler<LocalStorage> m_LSH;

protected:

  /* constructor */
  PlaneGeometryDataMapper2D();

  /* destructor */
  virtual ~PlaneGeometryDataMapper2D();

  /* \brief Applies the color and opacity properties and calls CreateVTKRenderObjects */
  virtual void GenerateDataForRenderer(mitk::BaseRenderer* renderer) override;

  void CreateVtkCrosshair(BaseRenderer *renderer);

  /**
    * \brief Returns the thick slice mode for the given datanode.
    *
    * This method returns the value of the 'reslice.thickslices' property for
    * the given datanode.
    *   '0': thick slice mode disabled
    *   '1': thick slice mode enabled
    *
    * The variable 'thickSlicesNum' contains the value of the 'reslice.thickslices.num'
    * property that defines how many slices are shown at once.
    */
  int DetermineThickSliceMode( DataNode * dn, int &thickSlicesNum );

  void DrawLine(Point3D p0, Point3D p1,
                vtkCellArray *lines,
                vtkPoints *points);

  // member variables holding the current value of the properties used in this mapper
  typedef std::vector<DataNode*> NodesVectorType;
  NodesVectorType m_OtherPlaneGeometries;

  typedef std::set<Self*> AllInstancesContainer;
  static AllInstancesContainer s_AllInstances;

  bool m_RenderOrientationArrows;
  bool m_ArrowOrientationPositive;
  mitk::ScalarType m_DepthValue;

  void ApplyColorAndOpacityProperties2D(BaseRenderer *renderer, vtkActor2D *actor);
};
} // namespace mitk
#endif /* mitkPlaneGeometryDataMapper2D_h */
