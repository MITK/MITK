/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkToolManager_h
#define mitkToolManager_h

#include "mitkDataNode.h"
#include "mitkDataStorage.h"
#include "mitkTool.h"
#include "mitkWeakPointer.h"
#include <MitkSegmentationExports.h>

#pragma GCC visibility push(default)
#include <itkEventObject.h>
#pragma GCC visibility pop

#include <vector>

namespace mitk
{
  class Image;
  class PlaneGeometry;

  /**
    \brief Manages and coordinates instances of mitk::Tool.

    \sa QmitkToolSelectionBox
    \sa Tool
    \sa QmitkSegmentationView

    \ingroup Interaction
    \ingroup ToolManagerEtAl

    There is a separate page describing the general design of QmitkSegmentationView: \ref QmitkSegmentationTechnicalPage

    This class creates and manages several instances of mitk::Tool.

    \li ToolManager creates instances of mitk::Tool by asking the itk::ObjectFactory to list all known implementations
    of mitk::Tool.
        As a result, one has to implement both a subclass of mitk::Tool and a matching subclass of
    itk::ObjectFactoryBase that is registered
        to the top-level itk::ObjectFactory. For an example, see mitkContourToolFactory.h. (this limitiation of
    one-class-one-factory is due
        to the implementation of itk::ObjectFactory).
        In MITK, the right place to register the factories to itk::ObjectFactory is the mitk::QMCoreObjectFactory or
    mitk::SBCoreObjectFactory.

     \li ToolManager knows a set of "reference" DataNodes and a set of "working" DataNodes. The first application are
    segmentation tools, where the
        reference is the original image and the working data the (kind of) binary segmentation. However, ToolManager is
    implemented more generally, so that
        there could be other tools that work, e.g., with surfaces.


    \li There is a set of events that are sent by ToolManager. At the moment these are TODO update documentation:
        - mitk::ToolReferenceDataChangedEvent whenever somebody calls SetReferenceData. Most of the time this actually
    means that the data has changed, but
          there might be cases where the same data is passed to SetReferenceData a second time, so don't rely on the
    assumption that something actually changed.
        - mitk::ToolSelectedEvent is sent when a (truly) different tool was activated. In reaction to this event you can
    ask for the active Tool using
          GetActiveTool or GetActiveToolID (where nullptr or -1 indicate that NO tool is active at the moment).

    Design descisions:

    \li Not a singleton, because there could be two functionalities using tools, each one with different
    reference/working data.

    $Author$
  */
  class MITKSEGMENTATION_EXPORT ToolManager : public itk::Object
  {
  public:
    typedef std::vector<Tool::Pointer> ToolVectorType;
    typedef std::vector<Tool::ConstPointer> ToolVectorTypeConst;
    typedef std::vector<DataNode *> DataVectorType; // has to be observed for delete events!
    typedef std::map<DataNode *, unsigned long> NodeTagMapType;

    Message<> NodePropertiesChanged;
    Message<> NewNodesGenerated;
    Message1<DataVectorType *> NewNodeObjectsGenerated;

    Message<> ActiveToolChanged;
    Message<> ReferenceDataChanged;
    Message<> WorkingDataChanged;
    Message<> RoiDataChanged;
    Message<> SelectedTimePointChanged;

    Message1<std::string> ToolErrorMessage;
    Message1<std::string> GeneralToolMessage;

    mitkClassMacroItkParent(ToolManager, itk::Object);
    mitkNewMacro1Param(ToolManager, DataStorage *);

    /**
      \brief Gives you a list of all tools.
      This is const on purpose.
     */
    const ToolVectorTypeConst GetTools();

    int GetToolID(const Tool *tool);

    /**
      \param id The tool of interest.
      Counting starts with 0.
    */
    Tool *GetToolById(int id);

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
      for (auto iter = m_Tools.begin(); iter != m_Tools.end(); ++iter, ++id)
      {
        if (dynamic_cast<T *>(iter->GetPointer()))
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
      \return nullptr for "No tool is active"
    */
    Tool *GetActiveTool();

    /**
      \brief Set a list of data/images as reference objects.
    */
    void SetReferenceData(DataVectorType);

    /**
      \brief Set single data item/image as reference object.
    */
    void SetReferenceData(DataNode *);

    /**
      \brief Set a list of data/images as working objects.
    */
    void SetWorkingData(DataVectorType);

    /**
      \brief Set single data item/image as working object.
    */
    void SetWorkingData(DataNode *);

    /**
      \brief Set a list of data/images as roi objects.
    */
    void SetRoiData(DataVectorType);

    /**
      \brief Set a single data item/image as roi object.
    */
    void SetRoiData(DataNode *);

    /**
      \brief Get the list of reference data.
    */
    DataVectorType GetReferenceData();

    /**
      \brief Get the current reference data.
      \warning If there is a list of items, this method will only return the first list item.
    */
    DataNode *GetReferenceData(int);

    /**
      \brief Get the list of working data.
    */
    DataVectorType GetWorkingData();

    /**
      \brief Get the current working data.
      \warning If there is a list of items, this method will only return the first list item.
    */
    DataNode *GetWorkingData(unsigned int);

    /**
     \brief Get the current roi data
     */
    DataVectorType GetRoiData();

    /**
     \brief Get the roi data at position idx
     */
    DataNode *GetRoiData(int idx);

    DataStorage::Pointer GetDataStorage() const;
    void SetDataStorage(DataStorage &storage);

    /** Get the current selected time point of the RenderManager
    */
    TimePointType GetCurrentTimePoint() const;

    /**
      \brief Tell that someone is using tools.
      GUI elements should call this when they become active. This method increases an internal "client count".
     */
    void RegisterClient();

    /**
      \brief Tell that someone is NOT using tools.
      GUI elements should call this when they become active. This method increases an internal "client count".
     */
    void UnregisterClient();

    /** \brief Initialize all classes derived from mitk::Tool by itkObjectFactoy */
    void InitializeTools();

    void OnOneOfTheReferenceDataDeletedConst(const itk::Object *caller, const itk::EventObject &e);
    void OnOneOfTheReferenceDataDeleted(itk::Object *caller, const itk::EventObject &e);

    void OnOneOfTheWorkingDataDeletedConst(const itk::Object *caller, const itk::EventObject &e);
    void OnOneOfTheWorkingDataDeleted(itk::Object *caller, const itk::EventObject &e);

    void OnOneOfTheRoiDataDeletedConst(const itk::Object *caller, const itk::EventObject &e);
    void OnOneOfTheRoiDataDeleted(itk::Object *caller, const itk::EventObject &e);

    /**
     \brief Connected to tool's messages

     This method just resends error messages coming from any of the tools. This way clients (GUIs) only have to observe
     one message.
     */
    void OnToolErrorMessage(std::string s);
    void OnGeneralToolMessage(std::string s);

  protected:
    /**
      You may specify a list of tool "groups" that should be available for this ToolManager. Every Tool can report its
      group
      as a string. This constructor will try to find the tool's group inside the supplied string. If there is a match,
      the tool is accepted. Effectively, you can provide a human readable list like "default, lymphnodevolumetry,
      oldERISstuff".
    */
    ToolManager(DataStorage *storage); // purposely hidden
    ~ToolManager() override;

    ToolVectorType m_Tools;

    Tool *m_ActiveTool;
    int m_ActiveToolID;
    us::ServiceRegistration<InteractionEventObserver> m_ActiveToolRegistration;

    DataVectorType m_ReferenceData;
    NodeTagMapType m_ReferenceDataObserverTags;

    DataVectorType m_WorkingData;
    NodeTagMapType m_WorkingDataObserverTags;

    DataVectorType m_RoiData;
    NodeTagMapType m_RoiDataObserverTags;

    int m_RegisteredClients;

    WeakPointer<DataStorage> m_DataStorage;

    /// \brief Callback for NodeRemove events
    void OnNodeRemoved(const mitk::DataNode *node);

    /** Callback for time changed events*/
    void OnTimeChangedConst(const itk::Object* caller, const itk::EventObject& e);
    void OnTimeChanged(itk::Object* caller, const itk::EventObject& e);

    void EnsureTimeObservation();
    void StopTimeObservation();

  private:
    /** Time point of last detected change*/
    TimePointType m_LastTimePoint = 0;
    /** Tag of the observer that listens to time changes*/
    unsigned long m_TimePointObserverTag = 0;
    /** Pointer to the observed time stepper*/
    WeakPointer<SliceNavigationController> m_CurrentTimeNavigationController;
  };

} // namespace

#endif
