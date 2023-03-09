/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkEnhancedPointSetVtkMapper3D_h
#define mitkEnhancedPointSetVtkMapper3D_h

#include "MitkMapperExtExports.h"
#include "mitkBaseRenderer.h"
#include "mitkCommon.h"
#include "mitkPointSet.h"
#include "mitkVector.h"
#include "mitkVtkMapper.h"

class vtkActor;
class vtkAssembly;
class vtkSphereSource;
class vtkCubeSource;
class vtkConeSource;
class vtkCylinderSource;
class vtkTubeFilter;
class vtkProp;

namespace mitk
{
  /**
  * \brief Alternative Vtk-based 3D mapper for mitk::PointSet
  *
  * This class renders mitk::PointSet objects in 3D views. It resembles the
  * standard mitk::PointSetVtkMapper3D, but is designed to enable single
  * points to be rendered with individual appearances.
  *
  * Instead of assembling one vtkPolyData object containing all points,
  * a list of VTK source objects (spheres, cubes, cones, ...) is maintained.
  * Therefore, the application can change the appearance and/or type of a
  * specific point at runtime, without having to rebuild the
  *
  * You should use this class instead of the standard mapper if you
  *
  * - change the PointSet very often (by adding or removing points)
  * - need different representations for points (+++)
  * - want to change the point representation frequently (+++)
  *
  * Note: the class is still in experimental stage, and the points above
  * marked with (+++) are not yet working correctly. Also, drawing lines
  * between points (contour mode) is not yet supported. The class will be
  * extended so that point representations are stored in a lookup table,
  * which is indexed by point data from the rendered PointSet.
  *
  * \warning This mapper requires the PointData container to be the same size
  *       as the point container.
  *
  * \sa PointSetVtkMapper3D
  */
  class MITKMAPPEREXT_EXPORT EnhancedPointSetVtkMapper3D : public VtkMapper
  {
  public:
    mitkClassMacro(EnhancedPointSetVtkMapper3D, VtkMapper);

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self);

      virtual const mitk::PointSet *GetInput();

    vtkProp *GetVtkProp(mitk::BaseRenderer *renderer) override;

    void UpdateVtkTransform(mitk::BaseRenderer *renderer) override;

    static void SetDefaultProperties(mitk::DataNode *node, mitk::BaseRenderer *renderer = nullptr, bool overwrite = false);

    LocalStorageHandler<BaseLocalStorage> m_LSH;

  protected:
    EnhancedPointSetVtkMapper3D();

    ~EnhancedPointSetVtkMapper3D() override;

    void RemoveEntryFromSourceMaps(mitk::PointSet::PointIdentifier pointID);
    void DeleteVtkObject(vtkObject *o); // functor for stl_each in destructor

    // update all vtk sources, mappers, actors with current data and properties
    void UpdateVtkObjects();

    void GenerateDataForRenderer(mitk::BaseRenderer *renderer) override;
    void ApplyColorAndOpacityProperties(mitk::BaseRenderer *renderer, vtkActor *actor) override;

    typedef mitk::PointSet::PointIdentifier PointIdentifier;
    typedef std::map<PointIdentifier, vtkSphereSource *> SphereSourceMap;
    typedef std::map<PointIdentifier, vtkCubeSource *> CubeSourceMap;
    typedef std::map<PointIdentifier, vtkConeSource *> ConeSourceMap;
    typedef std::map<PointIdentifier, vtkCylinderSource *> CylinderSourceMap;

    typedef std::pair<vtkActor *, mitk::PointSpecificationType> ActorAndPointType;
    typedef std::map<PointIdentifier, ActorAndPointType> ActorMap;

    SphereSourceMap m_SphereSources;     // stores all sphere sources
    CubeSourceMap m_CubeSources;         // stores all cube sources
    ConeSourceMap m_ConeSources;         // stores all cone sources
    CylinderSourceMap m_CylinderSources; // stores all cylinder sources
    ActorMap m_PointActors; // stores an actor for each point(referenced by its ID) and the currently used pointspec =
                            // which source type is generating the polydata

    vtkActor *m_Contour;
    vtkTubeFilter *m_ContourSource;

    vtkAssembly *m_PropAssembly; // this contains everything, this will be returned by GetVtkProp()
  };
} // namespace mitk

#endif
