/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
   * \brief Manages the VTK layer hierarchy of a vtkRenderWindow.
   *
   * For simple access the layers are divided into three main groups:
   * background, scene, and foreground layers. Renderers can be
   * registered via the Insert... functions and removed via RemoveRenderer().
   */
  class MITKCORE_EXPORT VtkLayerController
  {
  public:
    /**
     * \brief Return the VtkLayerController instance for the given render window.
     *
     * \param[in] renWin The VTK render window.
     * \return The associated VtkLayerController, or nullptr if none exists.
     */
    static VtkLayerController *GetInstance(vtkSmartPointer<vtkRenderWindow> renWin);

    /**
     * \brief Register a new VtkLayerController instance for the given render window.
     *
     * \param[in] renWin The VTK render window.
     * \param[in] mitkSceneRenderer The initial scene renderer for this window.
     */
    static void AddInstance(vtkSmartPointer<vtkRenderWindow> renWin, vtkSmartPointer<vtkRenderer> mitkSceneRenderer);

    /**
     * \brief Remove the VtkLayerController instance for the given render window.
     *
     * \param[in] renWin The VTK render window whose controller should be removed.
     */
    static void RemoveInstance(vtkSmartPointer<vtkRenderWindow> renWin);

    /**
     * \brief Construct a VtkLayerController for the given render window.
     * \param[in] renderWindow The VTK render window to manage.
     */
    VtkLayerController(vtkSmartPointer<vtkRenderWindow> renderWindow);
    virtual ~VtkLayerController();

    /**
     * \brief Return the current scene renderer.
     * \return The VTK renderer used for the scene layer.
     */
    vtkSmartPointer<vtkRenderer> GetSceneRenderer();

    /**
     * \brief Insert a renderer into the background layer.
     *
     * \param[in] renderer The VTK renderer to insert.
     * \param[in] forceAbsoluteBackground If true, place the renderer at the absolute
     *            background. Multiple calls with this set to true will make the latest
     *            renderer the absolute background.
     */
    void InsertBackgroundRenderer(vtkSmartPointer<vtkRenderer> renderer, bool forceAbsoluteBackground);

    /**
     * \brief Insert a renderer into the foreground layer.
     *
     * \param[in] renderer The VTK renderer to insert.
     * \param[in] forceAbsoluteForeground If true, place the renderer at the absolute
     *            foreground. Multiple calls with this set to true will make the latest
     *            renderer the absolute foreground.
     */
    void InsertForegroundRenderer(vtkSmartPointer<vtkRenderer> renderer, bool forceAbsoluteForeground);

    /**
     * \brief Insert a renderer into the scene layer.
     *
     * The renderer will be rendered between background and foreground renderers.
     *
     * \param[in] renderer The VTK renderer to insert.
     */
    void InsertSceneRenderer(vtkSmartPointer<vtkRenderer> renderer);

    /**
     * \brief Set the VTK render window managed by this controller.
     * \param[in] renwin The VTK render window.
     */
    void SetRenderWindow(vtkSmartPointer<vtkRenderWindow> renwin);

    /**
     * \brief Remove a previously inserted renderer from the render window.
     * \param[in] renderer The VTK renderer to remove.
     */
    void RemoveRenderer(vtkSmartPointer<vtkRenderer> renderer);

    /**
     * \brief Check whether a renderer has been inserted.
     *
     * \param[in] renderer The VTK renderer to check.
     * \return True if the renderer is currently registered.
     */
    bool IsRendererInserted(vtkSmartPointer<vtkRenderer> renderer);

    /**
     * \brief Return the total number of renderers in the render window.
     * \return The count of all registered renderers across all layers.
     */
    unsigned int GetNumberOfRenderers();

    /**
     * \brief Set the erase flag for all registered renderers.
     * \param[in] i The erase flag value (non-zero to enable erasing).
     */
    void SetEraseForAllRenderers(int i);

  protected:
    vtkSmartPointer<vtkRenderWindow> m_RenderWindow;

  private:
    /**
      * Internally used to sort all registered renderers and to connect the with the vtkRenderWindow.
      * Mention that VTK Version 5 and above is rendering higher numbers in the background and VTK
      * Version < 5 in the foreground.
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
#endif
