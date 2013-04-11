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


#ifndef MITKPointSetVtkMAPPER2D_H_HEADER_INCLUDED_C1902626
#define MITKPointSetVtkMAPPER2D_H_HEADER_INCLUDED_C1902626

#include <MitkExports.h>
#include "mitkVtkMapper.h"
#include "mitkBaseRenderer.h"
#include <vtkSmartPointer.h>
#include <vtkAppendPolyData.h>
#include <vtkGlyph3D.h>
#include <vtkGlyphSource2D.h>
#include <vtkFloatArray.h>

class vtkActor;
class vtkPropAssembly;
class vtkAppendPolyData;
class vtkPolyData;
class vtkTubeFilter;
class vtkPolyDataMapper;
class vtkGlyphSource2D;
class vtkSphereSource;
class vtkGlyph3D;
class vtkFloatArray;

namespace mitk {

  class PointSet;
  //TODO -- change commentar!

  /**
  * @brief Vtk-based mapper for PointSet
  *
  * Due to the need of different colors for selected
  * and unselected points and the facts, that we also have a contour and
  * labels for the points, the vtk structure is build up the following way:
  *
  * We have two AppendPolyData, one selected, and one unselected and one
  * for a contour between the points. Each one is connected to an own
  * PolyDaraMapper and an Actor. The different color for the unselected and
  * selected state and for the contour is read from properties.
  *
  * "unselectedcolor", "selectedcolor" and "contourcolor" are the strings,
  * that are looked for. Pointlabels are added besides the selected or the
  * deselected points.
  *
  * Then the three Actors are combined inside a vtkPropAssembly and this
  * object is returned in GetProp() and so hooked up into the rendering
  * pipeline.

  * Properties that can be set for point sets and influence the PointSetVTKMapper3D are:
  *

  *   - \b "color": (ColorProperty*) Color of the point set
  *   - \b "Opacity": (FloatProperty) Opacity of the point set
  *   - \b "show contour": (BoolProperty) If the contour of the points are visible
  *   - \b "contourSizeProp":(FloatProperty) Contour size of the points


  The default properties are:

  *   - \b "line width": (IntProperty::New(2), renderer, overwrite )
  *   - \b "pointsize": (FloatProperty::New(1.0), renderer, overwrite)
  *   - \b "selectedcolor": (ColorProperty::New(1.0f, 0.0f, 0.0f), renderer, overwrite)  //red
  *   - \b "color": (ColorProperty::New(1.0f, 1.0f, 0.0f), renderer, overwrite)  //yellow
  *   - \b "show contour": (BoolProperty::New(false), renderer, overwrite )
  *   - \b "contourcolor": (ColorProperty::New(1.0f, 0.0f, 0.0f), renderer, overwrite)
  *   - \b "contoursize": (FloatProperty::New(0.5), renderer, overwrite )
  *   - \b "close contour": (BoolProperty::New(false), renderer, overwrite )
  *   - \b "show points": (BoolProperty::New(true), renderer, overwrite )
  *   - \b "updateDataOnRender": (BoolProperty::New(true), renderer, overwrite )



  *Other properties looked for are:
  *
  *   - \b "show contour": if set to on, lines between the points are shown
  *   - \b "close contour": if set to on, the open strip is closed (first point
  *       connected with last point)
  *   - \b "pointsize": size of the points mapped
  *   - \b "label": text of the Points to show besides points
  *   - \b "contoursize": size of the contour drawn between the points
  *       (if not set, the pointsize is taken)
  *
  * @ingroup Mapper
  */
  class MITK_CORE_EXPORT PointSetVtkMapper2D : public VtkMapper
  {
  public:
    mitkClassMacro(PointSetVtkMapper2D, VtkMapper);

    itkNewMacro(Self);

    //TODO: change to GetInputConnection()
    virtual const mitk::PointSet* GetInput();

    //overwritten from VtkMapper3D to be able to return a
    //m_PropAssemblies which is much faster than a vtkAssembly
    virtual vtkProp* GetVtkProp(mitk::BaseRenderer* renderer);

    static void SetDefaultProperties(mitk::DataNode* node, mitk::BaseRenderer* renderer = NULL, bool overwrite = false);

    //void ReleaseGraphicsResources(vtkWindow *renWin);


    //enum Glyph2DType{Vertex, Dash, Cross, ThickCross, Triangle, Square, Circle, Diamond, Arrow, ThickArrow, HookedArrow};



    /** \brief Internal class holding the mapper, actor, etc. for each of the 3 2D render windows */
    class LocalStorage : public mitk::Mapper::BaseLocalStorage
    {
    public:

      // use std::vector because of the three 2D render windows
      vtkSmartPointer<vtkPolyDataMapper> m_VtkUnselectedPolyDataMappers;
      vtkSmartPointer<vtkPolyDataMapper> m_VtkSelectedPolyDataMappers;
      vtkSmartPointer<vtkPolyDataMapper> m_VtkContourPolyDataMappers;

      vtkSmartPointer<vtkPolyData> m_VtkUnselectedPointListPolyData;
      vtkSmartPointer<vtkPolyData> m_VtkSelectedPointListPolyData;
      vtkSmartPointer<vtkPolyData> m_VtkContourPolyData;

      vtkSmartPointer<vtkActor> m_UnselectedActors;
      vtkSmartPointer<vtkActor> m_SelectedActors;
      vtkSmartPointer<vtkActor> m_ContourActors;

      vtkSmartPointer<vtkPropAssembly> m_PropAssemblies;

      vtkSmartPointer<vtkPoints> m_UnselectedPoints;
      vtkSmartPointer<vtkPoints> m_SelectedPoints;
      vtkSmartPointer<vtkPoints> m_ContourPoints;

      vtkSmartPointer<vtkGlyph3D> m_UnselectedGlyph3D;
      vtkSmartPointer<vtkGlyph3D> m_SelectedGlyph3D;

       // Setup scales
     vtkSmartPointer<vtkFloatArray> m_UnselectedScales;
     vtkSmartPointer<vtkFloatArray> m_SelectedScales;

     vtkSmartPointer<vtkGlyphSource2D> m_UnselectedGlyphSource2D;
     vtkSmartPointer<vtkGlyphSource2D> m_SelectedGlyphSource2D;
      //help for contour between points
      // vtkSmartPointer<vtkAppendPolyData> m_vtkTextList;


      LocalStorage()
      {

        // mappers
        m_VtkUnselectedPolyDataMappers = vtkSmartPointer<vtkPolyDataMapper>::New();
        m_VtkSelectedPolyDataMappers = vtkSmartPointer<vtkPolyDataMapper>::New();
        m_VtkContourPolyDataMappers = vtkSmartPointer<vtkPolyDataMapper>::New();

         // Setup scales
         m_UnselectedScales = vtkSmartPointer<vtkFloatArray>::New();
         m_SelectedScales = vtkSmartPointer<vtkFloatArray>::New();


        // polydata
        m_VtkUnselectedPointListPolyData = vtkSmartPointer<vtkPolyData>::New();
        m_VtkSelectedPointListPolyData = vtkSmartPointer <vtkPolyData>::New();
        m_VtkContourPolyData = vtkSmartPointer<vtkPolyData>::New();

        // actors
        m_UnselectedActors = vtkSmartPointer <vtkActor>::New();
        m_SelectedActors = vtkSmartPointer <vtkActor>::New();
        m_ContourActors = vtkSmartPointer <vtkActor>::New();

        // propassembly
        m_PropAssemblies = vtkSmartPointer <vtkPropAssembly>::New();

        // points
        m_UnselectedPoints = vtkSmartPointer<vtkPoints>::New();
        m_SelectedPoints = vtkSmartPointer<vtkPoints>::New();
        m_ContourPoints = vtkSmartPointer<vtkPoints>::New();

        // glyphs
        m_UnselectedGlyph3D = vtkSmartPointer<vtkGlyph3D>::New();
        m_SelectedGlyph3D = vtkSmartPointer<vtkGlyph3D>::New();

        // glyph source (provides the different shapes)
        m_UnselectedGlyphSource2D = vtkSmartPointer<vtkGlyphSource2D>::New();
        m_SelectedGlyphSource2D = vtkSmartPointer<vtkGlyphSource2D>::New();

      }

      ~LocalStorage() {};

    };

    /** \brief The LocalStorageHandler holds all (three) LocalStorages for the three 2D render windows. */
    mitk::Mapper::LocalStorageHandler<LocalStorage> m_LSH;


  protected:
    PointSetVtkMapper2D();

    virtual ~PointSetVtkMapper2D();

    virtual void GenerateDataForRenderer(mitk::BaseRenderer* renderer);
    virtual void ResetMapper( BaseRenderer* renderer );
    virtual void ApplyAllProperties(mitk::BaseRenderer* renderer, vtkActor* actor);
    virtual void CreateVTKRenderObjects(mitk::BaseRenderer* renderer);

    bool m_Polygon;
    bool m_PolygonClosed;
    bool m_ShowPoints;
    bool m_ShowDistances;
    int m_DistancesDecimalDigits;
    bool m_ShowAngles;
    bool m_ShowDistantLines;
    int  m_LineWidth;
    int m_PointLineWidth;
    int m_Point2DSize;
    //Glyph2DType m_GlyphType;
    //mitk::EnumerationProperty::IdType

    int m_IdGlyph;
    bool m_FillGlyphs;

  };


} // namespace mitk

#endif /* MITKPointSetVtkMAPPER2D_H_HEADER_INCLUDED_C1902626 */
