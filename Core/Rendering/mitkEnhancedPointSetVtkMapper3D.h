/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef MITKEnhancedPointSetVtkMapper3D_H_HEADER_INCLUDED_C1907273
#define MITKEnhancedPointSetVtkMapper3D_H_HEADER_INCLUDED_C1907273

#include "mitkCommon.h"
#include "mitkBaseVtkMapper3D.h"
#include "mitkBaseRenderer.h"
#include "mitkPointSet.h"
#include "mitkVector.h"

class vtkActor;
class vtkPropAssembly;
class vtkSphereSource;
class vtkCubeSource;
class vtkConeSource;
class vtkCylinderSource;
class vtkTubeFilter;
class vtkProp;


namespace mitk {

  /**
  * @brief Vtk-based mapper for PointSet
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
  * "unselectedcolor", "selectedcolor" and "contourcolor" are the properties, 
  * that are looked for. Labels are added besides the selected or the 
  * deselected points. Specify the text in a "label" property.
  *
  * Then the three Actors are combined inside a vtkPropAssembly and this 
  * object is returned in GetProp() and so hooked up into the rendering 
  * pipeline. Other properties looked for are:
  *
  *   - \b "show contour": if set to on, lines between the points are shown
  *   - \b "close contour": if set to on, the open strip is closed (first point 
  *       connected with last point)
  *   - \b "pointsize": size of the points mapped
  *   - \b "show label": show text stored in "label" 
  *   - \b "label": text of the Points to show besides points
  *   - \b "opacity": transparency of the points and contour
  *   - \b "contoursize": size of the contour drawn between the points 
  *       (if not set, the pointsize is taken)
  * @ingroup Mapper
  */
  class MITK_CORE_EXPORT EnhancedPointSetVtkMapper3D : public BaseVtkMapper3D
  {
  public:
    mitkClassMacro(EnhancedPointSetVtkMapper3D, BaseVtkMapper3D);

    itkNewMacro(Self);

    virtual const mitk::PointSet* GetInput();

    //overwritten from BaseVtkMapper3D to be able to return a 
    //m_PointsAssembly which is much faster than a vtkAssembly
    //virtual vtkProp* GetProp();
    virtual void UpdateVtkTransform();

    static void SetDefaultProperties(mitk::DataTreeNode* node, mitk::BaseRenderer* renderer = NULL, bool overwrite = false);

    void ReleaseGraphicsResources(vtkWindow *renWin);

  protected:
    EnhancedPointSetVtkMapper3D();

    virtual ~EnhancedPointSetVtkMapper3D();

    void RemoveEntryFromSourceMaps( mitk::PointSet::PointIdentifier pointID );
    void DeleteVtkObject(vtkObject* o);  // functor for stl_each in destructor

    void UpdateVtkObjects();  // update all vtk sources, mappers, actors with current data and properties

    virtual void GenerateData();
    virtual void GenerateData(mitk::BaseRenderer* renderer);
    virtual void ApplyProperties(mitk::BaseRenderer* renderer);
    virtual void CreateContour(mitk::BaseRenderer* renderer);

    typedef mitk::PointSet::PointIdentifier PointIdentifier;
    typedef std::map<PointIdentifier, vtkSphereSource*> SphereSourceMap;
    typedef std::map<PointIdentifier, vtkCubeSource*> CubeSourceMap;
    typedef std::map<PointIdentifier, vtkConeSource*> ConeSourceMap;
    typedef std::map<PointIdentifier, vtkCylinderSource*> CylinderSourceMap;

    typedef std::pair<vtkActor*, mitk::PointSpecificationType> ActorAndPointType;
    typedef std::map<PointIdentifier, ActorAndPointType> ActorMap;

    SphereSourceMap m_SphereSources;      // stores all sphere sources
    CubeSourceMap m_CubeSources;          // stores all cube sources
    ConeSourceMap m_ConeSources;          // stores all cone sources
    CylinderSourceMap m_CylinderSources;  // stores all cylinder sources
    ActorMap m_PointActors; // stores an actor for each point(referenced by its ID) and the currently used pointspec = which source type is generating the polydata

    vtkActor* m_Contour;
    vtkTubeFilter* m_ContourSource;

    vtkPropAssembly* m_PropAssembly;  // this contains everything, this will be returned by GetProp()
  };
} // namespace mitk

#endif /* MITKEnhancedPointSetVtkMapper3D_H_HEADER_INCLUDED_C1907273 */
