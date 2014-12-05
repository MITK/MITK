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

#ifndef mitkToolManager_h_Included
#define mitkToolManager_h_Included

#include "mitkTool.h"
#include <MitkSegmentationExports.h>
#include "mitkDataNode.h"
#include "mitkDataStorage.h"
#include "mitkWeakPointer.h"

#pragma GCC visibility push(default)
#include <itkEventObject.h>
#pragma GCC visibility pop

#include <vector>
#include <map>

#include "usServiceRegistration.h"


namespace mitk
{

class Image;
class PlaneGeometry;

/**
  \brief Manages and coordinates instances of mitk::Tool.

  \sa QmitkToolSelectionBox
  \sa QmitkToolReferenceDataSelectionBox
  \sa QmitkToolWorkingDataSelectionBox
  \sa Tool
  \sa QmitkSegmentationView

  \ingroup Interaction
  \ingroup ToolManagerEtAl

  There is a separate page describing the general design of QmitkSegmentationView: \ref QmitkSegmentationTechnicalPage

  This class creates and manages several instances of mitk::Tool.

  \li ToolManager creates instances of mitk::Tool by asking the itk::ObjectFactory to list all known implementations of mitk::Tool.
      As a result, one has to implement both a subclass of mitk::Tool and a matching subclass of itk::ObjectFactoryBase that is registered
      to the top-level itk::ObjectFactory. For an example, see mitkContourToolFactory.h. (this limitiation of one-class-one-factory is due
      to the implementation of itk::ObjectFactory).
      In MITK, the right place to register the factories to itk::ObjectFactory is the mitk::QMCoreObjectFactory or mitk::SBCoreObjectFactory.

   \li One (and only one - or none at all) of the registered tools can be activated using ActivateTool. This tool is registered to mitk::GlobalInteraction
      as a listener and will receive all mouse clicks and keyboard strokes that get into the MITK event mechanism. Tools are automatically
      unregistered from GlobalInteraction when no clients are registered to ToolManager (see RegisterClient()).

   \li ToolManager knows a set of "reference" DataNodes and a set of "working" DataNodes. The first application are segmentation tools, where the
      reference is the original image and the working data the (kind of) binary segmentation. However, ToolManager is implemented more generally, so that
      there could be other tools that work, e.g., with surfaces.

  \li Any "user/client" of ToolManager, i.e. every functionality that wants to use a tool, should call RegisterClient when the tools should be active.
      ToolManager keeps track of how many clients want it to be used, and when this count reaches zero, it unregistes the active Tool from GlobalInteraction.
      In "normal" settings, the functionality does not need to care about that if it uses a QmitkToolSelectionBox, which does exactly that when it is
      enabled/disabled.

  \li There is a set of events that are sent by ToolManager. At the moment these are TODO update documentation:
      - mitk::ToolReferenceDataChangedEvent whenever somebody calls SetReferenceData. Most of the time this actually means that the data has changed, but
        there might be cases where the same data is passed to SetReferenceData a second time, so don't rely on the assumption that something actually changed.
      - mitk::ToolSelectedEvent is sent when a (truly) different tool was activated. In reaction to this event you can ask for the active Tool using
        GetActiveTool or GetActiveToolID (where NULL or -1 indicate that NO tool is active at the moment).

  Design descisions:

  \li Not a singleton, because there could be two functionalities using tools, each one with different reference/working data.

  $Author$
*/
class MitkSegmentation_EXPORT ToolManager : public itk::Object
{
  public:

    typedef std::vector<Tool::Pointer>         ToolVectorType;
    typedef std::vector<Tool::ConstPointer>    ToolVectorTypeConst;
    typedef std::vector<DataNode*> DataVectorType; // has to be observed for delete events!
    typedef std::map<DataNode*, unsigned long> NodeTagMapType;

    Message<> NodePropertiesChanged;
    Message<> NewNodesGenerated;
    Message1<DataVectorType*> NewNodeObjectsGenerated;

    Message<> ActiveToolChanged;
    Message<> ReferenceDataChanged;
    Message<> WorkingDataChanged;
    Message<> RoiDataChanged;

    Message1<std::string> ToolErrorMessage;
    Message1<std::string> GeneralToolMessage;

    mitkClassMacro(ToolManager, itk::Object);
    mitkNewMacro1Param(ToolManager, DataStorage*);

    /**
      \brief Gives you a list of all tools.
      This is const on purpose.
     */
    const ToolVectorTypeConst GetTools();

    int GetToolID( const Tool* tool );

    /*
      \param id The tool of interest.
      Counting starts with 0.
    */
    Tool* GetToolById(int id);

    /**
      \param id The tool to activate. Provide -1 for disabling any tools.
      Counting starts with 0.
      Registeres a listner for NodeRemoved event at DataStorage (see mitk::ToolManager::OnNodeRemoved).
    */
    bool ActivateTool(int id);

    template <class T>
    int GetToolIdByToolType()
    {
      int id = 0;
      for ( ToolVectorType::iterator iter = m_Tools.begin();
            iter != m_Tools.end();
            ++iter, ++id )
      {
        if ( dynamic_cast<T*>(iter->GetPointer()) )
        {
          return id;
        }
      }
      return -1;
    }

    /**
      \return -1 for "No tool is active"
    */
    int GetActiveToolID();

    /**
      \return NULL for "No tool is active"
    */
    Tool* GetActiveTool();

    /*
      \brief Set a list of data/images as reference objects.
    */
    void SetReferenceData(DataVectorType);

    /*
      \brief Set single data item/image as reference object.
    */
    void SetReferenceData(DataNode*);

    /*
      \brief Set a list of data/images as working objects.
    */
    void SetWorkingData(DataVectorType);

    /*
      \brief Set single data item/image as working object.
    */
    void SetWorkingData(DataNode*);

    /*
      \brief Set a list of data/images as roi objects.
    */
    void SetRoiData(DataVectorType);

    /*
      \brief Set a single data item/image as roi object.
    */
    void SetRoiData(DataNode*);

    /**
      * If set to true the mitk::GlobalInteraction just informs
      * the active tool about new mitk::StateEvent but no other
      * Interactor or Statemachine
      */
    void ActivateExclusiveStateEventPolicy(bool);

    /*
      \brief Get the list of reference data.
    */
    DataVectorType GetReferenceData();

    /*
      \brief Get the current reference data.
      \warning If there is a list of items, this method will only return the first list item.
    */
    DataNode* GetReferenceData(int);

    /*
      \brief Get the list of working data.
    */
    DataVectorType GetWorkingData();

    /*
      \brief Get the current working data.
      \warning If there is a list of items, this method will only return the first list item.
    */
    DataNode* GetWorkingData(int);

    /*
     \brief Get the current roi data
     */
    DataVectorType GetRoiData();

    /*
     \brief Get the roi data at position idx
     */
    DataNode* GetRoiData(int idx);

    DataStorage* GetDataStorage();
    void SetDataStorage(DataStorage& storage);

    /*
      \brief Tell that someone is using tools.
      GUI elements should call this when they become active. This method increases an internal "client count". Tools
      are only registered to GlobalInteraction when this count is greater than 0. This is useful to automatically deactivate
      tools when you hide their GUI elements.
     */
    void RegisterClient();

    /*
      \brief Tell that someone is NOT using tools.
      GUI elements should call this when they become active. This method increases an internal "client count". Tools
      are only registered to GlobalInteraction when this count is greater than 0. This is useful to automatically deactivate
      tools when you hide their GUI elements.
     */
    void UnregisterClient();

    /** \brief Initialize all classes derived from mitk::Tool by itkObjectFactoy */
    void InitializeTools();

    void OnOneOfTheReferenceDataDeletedConst(const itk::Object* caller, const itk::EventObject& e);
    void OnOneOfTheReferenceDataDeleted           (itk::Object* caller, const itk::EventObject& e);

    void OnOneOfTheWorkingDataDeletedConst(const itk::Object* caller, const itk::EventObject& e);
    void OnOneOfTheWorkingDataDeleted           (itk::Object* caller, const itk::EventObject& e);

    void OnOneOfTheRoiDataDeletedConst(const itk::Object* caller, const itk::EventObject& e);
    void OnOneOfTheRoiDataDeleted           (itk::Object* caller, const itk::EventObject& e);

    /*
     \brief Connected to tool's messages

     This method just resends error messages coming from any of the tools. This way clients (GUIs) only have to observe one message.
     */
    void OnToolErrorMessage(std::string s);
    void OnGeneralToolMessage(std::string s);

  protected:

    /**
      You may specify a list of tool "groups" that should be available for this ToolManager. Every Tool can report its group
      as a string. This constructor will try to find the tool's group inside the supplied string. If there is a match,
      the tool is accepted. Effectively, you can provide a human readable list like "default, lymphnodevolumetry, oldERISstuff".
    */
    ToolManager(DataStorage* storage); // purposely hidden
    virtual ~ToolManager();

    ToolVectorType m_Tools;

    Tool* m_ActiveTool;
    int m_ActiveToolID;
    us::ServiceRegistration<InteractionEventObserver> m_ActiveToolRegistration;

    DataVectorType m_ReferenceData;
    NodeTagMapType  m_ReferenceDataObserverTags;

    DataVectorType m_WorkingData;
    NodeTagMapType  m_WorkingDataObserverTags;

    DataVectorType m_RoiData;
    NodeTagMapType m_RoiDataObserverTags;

    int m_RegisteredClients;

    WeakPointer<DataStorage> m_DataStorage;

    bool m_ExclusiveStateEventPolicy;

    /// \brief Callback for NodeRemove events
    void OnNodeRemoved(const mitk::DataNode* node);

  private:

    std::map<us::ServiceReferenceU, EventConfig> m_DisplayInteractorConfigs;
};

} // namespace

#endif

