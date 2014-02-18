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

#include <mitkRenderWindow.h>
#include <mitkDataNodeFactory.h>
#include <mitkDataStorage.h>
#include <mitkXML2EventParser.h>
#include <mitkMouseModeSwitcher.h>

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

  InteractionTestHelper(const std::string &interactionXmlFilePath);

  ~InteractionTestHelper();

  /** @brief Returns the datastorage, in order to modify the data inside a rendering test.
    **/
  mitk::DataStorage::Pointer GetDataStorage();

  /**
     * @brief AddNodeToStorage Add a node to the datastorage and perform a reinit which is necessary for rendering.
     * @param node The data you want to add.
     */
  void AddNodeToStorage(mitk::DataNode::Pointer node);

  /**
   * @brief PlaybackInteraction playback loaded interaction by passing events to the dispatcher.
   */
  void PlaybackInteraction();

  /**
   * @brief SetTimeStep Sets timesteps of all SliceNavigationControllers to given timestep.
   * @param newTimeStep new timestep
   *
   * Does the same as using ImageNavigators Time slider. Use this if your data was modified in a timestep other than 0.
   */
  void SetTimeStep(int newTimeStep);

  typedef std::vector<mitk::RenderWindow::Pointer> RenderWindowListType;

protected:
  /**
     * @brief Initialize Internal method to initialize the renderwindow and set the datastorage.
     * @throws mitk::Exception if interaction xml file can not be loaded.
     */
  void Initialize(const std::string &interactionXmlFilePath);

  /**
   * @brief LoadInteraction loads events from xml file.
   * @param interactionXmlPath path to xml file with interaction events.
   */
  void LoadInteraction();


  mitk::XML2EventParser::EventContainerType m_Events; // List with loaded interaction events

  std::string m_InteractionFilePath;

  RenderWindowListType m_RenderWindowList;
  mitk::RenderWindow::Pointer m_RenderWindowAxial;
  mitk::RenderWindow::Pointer m_RenderWindowSagittal;
  mitk::RenderWindow::Pointer m_RenderWindowFrontal;
  mitk::DataStorage::Pointer m_DataStorage;
  mitk::MouseModeSwitcher::Pointer m_MouseModeSwitcher;

};
}//namespace mitk
#endif
