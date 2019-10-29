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

#ifndef mitkVtkLayerController_h
#define mitkVtkLayerController_h

#include <MitkCoreExports.h>
#include <map>
#include <vector>
#include <vtkSmartPointer.h>

class vtkRenderWindow;
class vtkRenderer;

namespace mitk
{
  /**
   * Manages the VTK layer hierarchy
   * of a vtkRenderWindow.
   * For simple access the layers are divided into three
   * main groups: background, scene and foreground layers.
   * Renderers can be registered via the insert... functions and
   * removed via the RemoveRenderer function.
   */
  class MITKCORE_EXPORT VtkLayerController
  {
  public:
    static VtkLayerController *GetInstance(vtkSmartPointer<vtkRenderWindow> renWin);
    static void AddInstance(vtkSmartPointer<vtkRenderWindow> renWin, vtkSmartPointer<vtkRenderer> mitkSceneRenderer);
    static void RemoveInstance(vtkSmartPointer<vtkRenderWindow> renWin);

    VtkLayerController(vtkSmartPointer<vtkRenderWindow> renderWindow);
    virtual ~VtkLayerController();

    /**
      * Returns the current vtkRenderer of the Scene
      */
    vtkSmartPointer<vtkRenderer> GetSceneRenderer();

    /**
      * Connects a VTK renderer with a vtk renderwindow. The renderer will be rendered in the background.
      * With forceAbsoluteBackground set true a renderer can be placed at the absolute background of the scene.
      * Multiple calls with forceAbsoluteBackground set true will set the latest registered renderer as background.
      */
    void InsertBackgroundRenderer(vtkSmartPointer<vtkRenderer> renderer, bool forceAbsoluteBackground);

    /**
      * Connects a VTK renderer with a vtk renderwindow. The renderer will be rendered in the foreground.
      * With forceAbsoluteBackground set true a renderer can be placed at the absolute foreground of the scene.
      * Multiple calls with forceAbsoluteForeground set true will set the latest registered renderer as foreground.
      */
    void InsertForegroundRenderer(vtkSmartPointer<vtkRenderer> renderer, bool forceAbsoluteForeground);

    /**
      * Connects a VTK renderer with a vtk renderwindow. The renderer will be rendered between background renderers and
      * foreground renderers.
      */
    void InsertSceneRenderer(vtkSmartPointer<vtkRenderer> renderer);

    /**
      * Connects a VtkRenderWindow with the layer controller.
      */
    void SetRenderWindow(vtkSmartPointer<vtkRenderWindow> renwin);

    /**
      * A renderer which has been inserted via a insert... function can be removed from the vtkRenderWindow with
      * RemoveRenderer.
      */
    void RemoveRenderer(vtkSmartPointer<vtkRenderer> renderer);

    /**
      * Returns true if a renderer has been inserted
      */
    bool IsRendererInserted(vtkSmartPointer<vtkRenderer> renderer);

    /**
      * Returns the number of renderers in the renderwindow.
      */
    unsigned int GetNumberOfRenderers();

    void SetEraseForAllRenderers(int i);

  protected:
    vtkSmartPointer<vtkRenderWindow> m_RenderWindow;

  private:
    /**
      * Internally used to sort all registered renderers and to connect the with the vtkRenderWindow.
      * Mention that VTK Version 5 and above is rendering higher numbers in the background and VTK
      * Verison < 5 in the foreground.
      */
    void UpdateLayers();

    // Layer Management
    typedef std::vector<vtkSmartPointer<vtkRenderer>> RendererVectorType;
    RendererVectorType m_BackgroundRenderers;
    RendererVectorType m_SceneRenderers;
    RendererVectorType m_ForegroundRenderers;

    typedef std::map<const vtkSmartPointer<vtkRenderWindow>, mitk::VtkLayerController *> vtkLayerControllerMapType;
    static vtkLayerControllerMapType s_LayerControllerMap;
  };

} // Namespace MITK
#endif /* mitkVtkLayerController_h */
