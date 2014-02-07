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

#ifndef mitkInteractionTestHelper_h
#define mitkInteractionTestHelper_h

#include <vtkSmartPointer.h>
#include <mitkRenderWindow.h>
#include <mitkDataNodeFactory.h>
#include <mitkDataStorage.h>
#include <mitkXML2EventParser.h>

#include <MitkTestingHelperExports.h>

class vtkRenderWindow;
class vtkRenderer;

namespace mitk
{

/** @brief Creates everything needed to load and playback interaction events.
 *
  * The interaction is loaded from an xml file and the event are created. This file is
  * usually a recorded user interaction with the GUI. This can be done with InteractionEventRecorder
  * plugin. Also all necessary objects to handle interaction events are generated.
  * The user of this class is responsible to add the data object to interact with to the data storage
  * of InteractionTestHelper. And must also make sure that a proper data interactor is associated with the data object.
  *
  * To test PointSet interaction for instance make sure you have a PointSet node and a PointSetDataInteractor.
  * Then just add the node to the storage of the your InteractionTestHelper by calling InteractionTestHelper::AddNodeToStorage.
  * Use InteractionTestHelper::PlaybackInteraction to execute.
  *
  * \sa XML2EventParser
  * \sa EventFactory
  * \sa EventRecorder
*/
class MITK_TESTINGHELPER_EXPORT InteractionTestHelper
{

public:
  /** @param interactionFilePath Path to xml file containing interaction events.
    **/
  InteractionTestHelper(std::string interactionFilePath);

  /** Default destructor */
  ~InteractionTestHelper();

  /** @brief Getter for the vtkRenderer.
    **/
  BaseRenderer* GetRenderer();

  /** @brief Getter for the vtkRenderWindow which should be used to call vtkRegressionTestImage.
    **/
  RenderWindow* GetRenderWindow();

  /**
   * @brief SetStopRenderWindow Convenience method to make the renderwindow hold after rendering. Usefull for debugging.
   * @param flag Flag indicating whether the renderwindow should automatically close (false, default) or stay open (true). Usefull for debugging.
   */
  void SetAutomaticallyCloseRenderWindow(bool automaticallyCloseRenderWindow);


  /** @brief Set the view direction of the renderwindow (e.g. sagittal, coronal, axial)
    **/
  void SetViewDirection(mitk::SliceNavigationController::ViewDirection viewDirection);


//  /** @brief Render everything into an mitkRenderWindow. Call SetViewDirection() and SetProperty() before this method.
//    **/
//  void Render();

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
   * @brief PlaybackInteraction plays loaded interaction by passing events to the dispatcher.
   */
  void PlaybackInteraction();

protected:
  /**
     * @brief Initialize Internal method to initialize the renderwindow and set the datastorage.
     * @param width Height of renderwindow.
     * @param height Width of renderwindow.
     * @param interactionFilePath path to xml file containing interaction events.
     */
  void Initialize(int width, int height, std::string interactionFilePath);


  /** @brief This method tries to load the given file into a member datastorage, in order to render it.
        @param fileName The filename of the file to be loaded (including path).
    **/
  void AddToStorage(const std::string& filename);

  mitk::XML2EventParser::EventContainerType m_Events; //<< List with loaded interaction events

  mitk::RenderWindow::Pointer m_RenderWindow; //<< Contains the mitkRenderWindow into which the test renders the data
  mitk::DataStorage::Pointer m_DataStorage; //<< Contains the mitkDataStorage which contains the data to be rendered
  bool m_AutomaticallyCloseRenderWindow; //<< Flag indicating whether the renderwindow should automatically close (true, default) or stay open (false). Usefull for debugging.

};
}//namespace mitk
#endif
