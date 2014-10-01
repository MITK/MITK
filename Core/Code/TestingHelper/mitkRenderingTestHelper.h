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

#ifndef mitkRenderingTestHelper_h
#define mitkRenderingTestHelper_h

#include <vtkSmartPointer.h>
#include <mitkRenderWindow.h>
#include <mitkDataStorage.h>

#include <MitkTestingHelperExports.h>

class vtkRenderWindow;
class vtkRenderer;

namespace mitk
{
class MITK_TESTINGHELPER_EXPORT RenderingTestHelper
{
public:
  /** @brief Generate a rendering test helper object including a render window of the size width * height (in pixel).
      @param argc Number of parameters. (here: Images) "Usage: [filename1 filenam2 -V referenceScreenshot
        (optional -T /directory/to/save/differenceImage)]
      @param argv Given parameters. If no data is inserted via commandline, you can add data
      later via AddNodeToDataStorage().
      @param renderingMode Enable Standard, Multisample or DepthPeeling
    **/
  RenderingTestHelper(int width, int height, int argc, char *argv[], mitk::BaseRenderer::RenderingMode::Type renderingMode = mitk::BaseRenderer::RenderingMode::Standard);

  /** @brief Generate a rendering test helper object including a render window of the size width * height (in pixel).*/
  RenderingTestHelper(int width, int height, mitk::BaseRenderer::RenderingMode::Type renderingMode = mitk::BaseRenderer::RenderingMode::Standard);

  /** Default destructor */
  ~RenderingTestHelper();

  /** @brief Getter for the vtkRenderer.
    **/
  vtkRenderer* GetVtkRenderer();

  /** @brief Getter for the vtkRenderWindow which should be used to call vtkRegressionTestImage.
    **/
  vtkRenderWindow* GetVtkRenderWindow();

  /** @brief Method can be used to save a screenshot (e.g. reference screenshot as a .png file.
        @param fileName The filename of the new screenshot (including path).
    **/
  void SaveAsPNG(std::string fileName);

  /**
   * @brief SetStopRenderWindow Convenience method to make the renderwindow hold after rendering. Usefull for debugging.
   * @param flag Flag indicating whether the renderwindow should automatically close (false, default) or stay open (true). Usefull for debugging.
   */
  void SetAutomaticallyCloseRenderWindow(bool automaticallyCloseRenderWindow);

  /** @brief This method set the property of the member datastorage
        @param property Set a property for each image in the datastorage m_DataStorage. If you want
        to set the property for a single data node, use GetDataStorage() and set the property
        yourself for the destinct node.
    **/
  void SetImageProperty(const char *propertyKey, mitk::BaseProperty *property);

  /** @brief Set the view direction of the renderwindow (e.g. sagittal, coronal, axial)
    **/
  void SetViewDirection(mitk::SliceNavigationController::ViewDirection viewDirection);

  /** @brief Reorient the slice (e.g. rotation and translation like the swivel mode).
    **/
  void ReorientSlices(mitk::Point3D origin, mitk::Vector3D rotation);

  /** @brief Render everything into an mitkRenderWindow. Call SetViewDirection() and SetProperty() before this method.
    **/
  void Render();

  /** @brief Returns the datastorage, in order to modify the data inside a rendering test.
    **/
  mitk::DataStorage::Pointer GetDataStorage();

  /**
     * @brief SetMapperID Change between Standard2D and 3D mappers.
     * @param id Enum mitk::BaseRenderer::StandardMapperSlot which defines the mapper.
     */
  void SetMapperID(mitk::BaseRenderer::StandardMapperSlot id);

  /**
     * @brief AddNodeToStorage Add a node to the datastorage and perform a reinit which is necessary for rendering.
     * @param node The data you want to add.
     */
  void AddNodeToStorage(mitk::DataNode::Pointer node);

  /**
     * @brief SetMapperIDToRender3D Convenience method to render in a 3D renderwindow.
     * @warning Does not add helper objects like the image planes to render images in 3D.
     */
  void SetMapperIDToRender3D();

  /**
     * @brief SetMapperIDToRender2D Convenience method to render in a 2D renderwindow.
     */
  void SetMapperIDToRender2D();

  /**
     * @brief SaveReferenceScreenShot Convenience method to save a reference screen shot.
     * @param fileName Path/to/save/the/png/file.
     */
  void SaveReferenceScreenShot(std::string fileName);

  /**
   * @brief CompareRenderWindowAgainstReference Convenience method to compare the image rendered in the internal renderwindow against a reference screen shot.
   *
    Usage of vtkTesting::Test:
    vtkTesting::Test( argc, argv, vtkRenderWindow, threshold )
    Set a vtkRenderWindow containing the desired scene. This is automatically rendered.
    vtkTesting::Test() automatically searches in argc and argv[]
    for a path a valid image with -V. If the test failed with the
    first image (foo.png) it checks if there are images of the form
    foo_N.png (where N=1,2,3...) and compare against them. This allows for multiple
    valid images.
   * @param argc Number of arguments.
   * @param argv Arguments must(!) contain the term "-V Path/To/Valid/Image.png"
   * @param threshold Allowed difference between two images. Default = 10.0 and was taken from VTK.
   * @return True if the images are equal regarding the threshold. False in all other cases.
   */
  bool CompareRenderWindowAgainstReference(int argc, char *argv[], double threshold = 10.0);

  /** @brief Returns true if the opengl context is compatible for advanced vtk effects **/
  bool IsAdvancedOpenGL();

protected:
  /**
     * @brief Initialize Internal method to initialize the renderwindow and set the datastorage.
     * @param width Height of renderwindow.
     * @param height Width of renderwindow.
     * @param renderingMode Enable Standard, Multisampling or Depthpeeling
     */
  void Initialize(int width, int height, mitk::BaseRenderer::RenderingMode::Type renderingMode = mitk::BaseRenderer::RenderingMode::Standard);

  /** @brief Prints the opengl information, e.g. version, vendor and extensions,
     *         This function can only be called after an opengl context is active.
     *         It only prints the context after the vtkRenderwindow is fully initialized.
     **/
  void PrintGLInfo();

  /** @brief This method tries to load the given file into a member datastorage, in order to render it.
        @param fileName The filename of the file to be loaded (including path).
    **/
  void AddToStorage(const std::string& filename);

  /** @brief This method tries to parse the given argv for files (e.g. images) and load them into a member datastorage, in order to render it.
        @param argc Number of parameters.
        @param argv Given parameters.
    **/
  void SetInputFileNames(int argc, char *argv[]);

  mitk::RenderWindow::Pointer m_RenderWindow; //<< Contains the mitkRenderWindow into which the test renders the data
  mitk::DataStorage::Pointer m_DataStorage; //<< Contains the mitkDataStorage which contains the data to be rendered
  bool m_AutomaticallyCloseRenderWindow; //<< Flag indicating whether the renderwindow should automatically close (true, default) or stay open (false). Usefull for debugging.
};
}//namespace mitk
#endif