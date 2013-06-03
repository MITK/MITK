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
#include "mitkLocalStorageHandler.h"
#include <vtkSmartPointer.h>
#include <vtkAppendPolyData.h>
#include <vtkGlyph3D.h>
#include <vtkGlyphSource2D.h>
#include <vtkFloatArray.h>
#include <vtkCellArray.h>
#include <vtkTextActor.h>

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

  /**
  * @brief Vtk-based 2D mapper for PointSet
  *
  * Due to the need of different colors for selected
  * and unselected points and the facts, that we also have a contour and
  * labels for the points, the vtk structure is build up the following way:
  *
  * We have three PolyData, one selected, and one unselected and one
  * for a contour between the points. Each one is connected to an own
  * PolyDataMapper and an Actor. The different color for the unselected and
  * selected state and for the contour is read from properties.
  *
  * This mapper has several additional functionalities, such as rendering
  * a contour between points, calculating and displaying distances or angles
  * between points.
  *
  * Then the three Actors are combined inside a vtkPropAssembly and this
  * object is returned in GetProp() and so hooked up into the rendering
  * pipeline.

  * Properties that can be set for point sets and influence the PointSetVTKMapper2D are:
  *
  *   - \b "line width": (IntProperty 2)               // line width of the line from one point to another
  *   - \b "point line width": (IntProperty 1)         // line width of the cross marking a point
  *   - \b "point 2D size": (IntProperty 6)            // size of the glyph marking a point
  *   - \b "show contour": (BoolProperty false)        // enable contour rendering between points (lines)
  *   - \b "close contour": (BoolProperty false)       // if enabled, the open strip is closed (first point connected with last point)
  *   - \b "show points": (BoolProperty true)          // show or hide points
  *   - \b "show distances": (BoolProperty false)      // show or hide distance measure
  *   - \b "distance decimal digits": (IntProperty 2)  // set the number of decimal digits to be shown when rendering the distance information
  *   - \b "show angles": (BoolProperty false)         // show or hide angle measurement
  *   - \b "show distant lines": (BoolProperty false)  // show the line between to points from a distant view (equals "always on top" option)
  *   - \b "layer": (IntProperty 1)                    // default is drawing pointset above images (they have a default layer of 0)
  *   - \b "PointSet.2D.shape" (EnumerationProperty Cross)    // provides different shapes marking a point
  *       0 = "None", 1 = "Vertex", 2 = "Dash", 3 = "Cross", 4 = "ThickCross", 5 = "Triangle", 6 = "Square", 7 = "Circle",
  *       8 = "Diamond", 9 = "Arrow", 10 = "ThickArrow", 11 = "HookedArrow", 12 = "Cross"
  *   - \b "PointSet.2D.fill shape": (BoolProperty::New(false))   // fill or do not fill the glyph shape
  *
  *
  * Other Properties used here but not defined in this class:
  *
  *   - \b "selectedcolor": (ColorProperty (1.0f, 0.0f, 0.0f))  // default color of the selected pointset e.g. the current point is red
  *   - \b "contourcolor" : (ColorProperty (1.0f, 0.0f, 0.0f))  // default color for the contour is red
  *   - \b "color": (ColorProperty (1.0f, 1.0f, 0.0f))          // default color of the (unselected) pointset is yellow
  *   - \b "opacity": (FloatProperty 1.0)                       // opacity of point set, contours
  *   - \b "label": (StringProperty NULL)     // a label can be defined for each point, which is rendered in proximity to the point
  *
  * @ingroup Mapper
  */
  class MITK_CORE_EXPORT PointSetVtkMapper2D : public VtkMapper
  {
  public:
    mitkClassMacro(PointSetVtkMapper2D, VtkMapper);

    itkNewMacro(Self);

    virtual const mitk::PointSet* GetInput();

    /** \brief returns the a prop assembly */
    virtual vtkProp* GetVtkProp(mitk::BaseRenderer* renderer);

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

      // points
      vtkSmartPointer<vtkPoints> m_UnselectedPoints;
      vtkSmartPointer<vtkPoints> m_SelectedPoints;
      vtkSmartPointer<vtkPoints> m_ContourPoints;

      // scales
      vtkSmartPointer<vtkFloatArray> m_UnselectedScales;
      vtkSmartPointer<vtkFloatArray> m_SelectedScales;

      // distances
      vtkSmartPointer<vtkFloatArray> m_DistancesBetweenPoints;

      // lines
      vtkSmartPointer<vtkCellArray> m_ContourLines;

      // glyph source (provides different shapes for the points)
      vtkSmartPointer<vtkGlyphSource2D> m_UnselectedGlyphSource2D;
      vtkSmartPointer<vtkGlyphSource2D> m_SelectedGlyphSource2D;

      // glyph
      vtkSmartPointer<vtkGlyph3D> m_UnselectedGlyph3D;
      vtkSmartPointer<vtkGlyph3D> m_SelectedGlyph3D;

      // polydata
      vtkSmartPointer<vtkPolyData> m_VtkUnselectedPointListPolyData;
      vtkSmartPointer<vtkPolyData> m_VtkSelectedPointListPolyData;
      vtkSmartPointer<vtkPolyData> m_VtkContourPolyData;

      // actor
      vtkSmartPointer<vtkActor> m_UnselectedActor;
      vtkSmartPointer<vtkActor> m_SelectedActor;
      vtkSmartPointer<vtkActor> m_ContourActor;
      vtkSmartPointer<vtkTextActor> m_VtkTextActor;

      std::vector < vtkSmartPointer<vtkTextActor> > m_VtkTextLabelActors;
      std::vector < vtkSmartPointer<vtkTextActor> > m_VtkTextDistanceActors;
      std::vector < vtkSmartPointer<vtkTextActor> > m_VtkTextAngleActors;

      // mappers
      vtkSmartPointer<vtkPolyDataMapper> m_VtkUnselectedPolyDataMapper;
      vtkSmartPointer<vtkPolyDataMapper> m_VtkSelectedPolyDataMapper;
      vtkSmartPointer<vtkPolyDataMapper> m_VtkContourPolyDataMapper;

      // propassembly
      vtkSmartPointer<vtkPropAssembly> m_PropAssembly;

    };

    /** \brief The LocalStorageHandler holds all (three) LocalStorages for the three 2D render windows. */
    mitk::LocalStorageHandler<LocalStorage> m_LSH;


  protected:

    /* constructor */
    PointSetVtkMapper2D();

    /* destructor */
    virtual ~PointSetVtkMapper2D();

    /* \brief Applies the color and opacity properties and calls CreateVTKRenderObjects */
    virtual void GenerateDataForRenderer(mitk::BaseRenderer* renderer);
    /* \brief Toggles visiblity of the propassembly */
    virtual void ResetMapper( BaseRenderer* renderer );
    /* \brief Fills the vtk objects, thus it is only called when the point set has been changed.
    * This function iterates over the input point set and uses a specific shape defined in vtk glyph source
    * to mark each point. The glyphs need to be rotated in the 2D- render windows
    * in order to be ortogonal to the view vector. Therefore, the rotation of the current PlaneGeometry is
    * used to determine the appropriate orienation of the glyphs. */
    virtual void CreateVTKRenderObjects(mitk::BaseRenderer* renderer);

    // properties
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
    int m_IdGlyph;
    bool m_FillGlyphs;

  };


} // namespace mitk

#endif /* MITKPointSetVtkMAPPER2D_H_HEADER_INCLUDED_C1902626 */
