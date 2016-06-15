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

#ifndef mitkTool_h_Included
#define mitkTool_h_Included

#include "mitkCommon.h"
#include <MitkSegmentationExports.h>
#include "mitkToolEvents.h"
#include "itkObjectFactoryBase.h"
#include "itkVersion.h"
#include "mitkToolFactoryMacro.h"
#include "mitkMessage.h"
#include "mitkDataNode.h"
#include "mitkNodePredicateProperty.h"
#include "mitkNodePredicateDataType.h"
#include "mitkNodePredicateDimension.h"
#include "mitkNodePredicateAnd.h"
#include "mitkNodePredicateOr.h"
#include "mitkNodePredicateNot.h"

#include <iostream>
#include <map>
#include <string>

#include <itkObject.h>

#include "usServiceRegistration.h"

#include "mitkInteractionEventObserver.h"
#include "mitkEventStateMachine.h"
#include <mitkLabel.h>


namespace us {
class ModuleResource;
}

namespace mitk
{

  class ToolManager;

  /**
  \brief Base class of all tools used by mitk::ToolManager.

  \sa ToolManager
  \sa SegTool2D

  \ingroup Interaction
  \ingroup ToolManagerEtAl

  There is a separate page describing the \ref QmitkInteractiveSegmentationTechnicalPage.

  Every tool is a mitk::StateMachine, which can follow any transition pattern that it likes. One important thing to know is, that
  every derived tool should always call SuperClass::Deactivated() at the end of its own implementation of Deactivated, because mitk::Tool
  resets the StateMachine in this method. Only if you are very sure that you covered all possible things that might happen to your
  own tool, you should consider not to reset the StateMachine from time to time.

  To learn about the MITK implementation of state machines in general, have a look at \ref InteractionPage.

  To derive a non-abstract tool, you inherit from mitk::Tool (or some other base class further down the inheritance tree), and in your
  own parameterless constructor (that is called from the itkFactorylessNewMacro that you use) you pass a StateMachine pattern name to the superclass.
  Names for valid patterns can be found in StateMachine.xml (which might be enhanced by you).

  You have to implement at least GetXPM() and GetName() to provide some identification.

  Each Tool knows its ToolManager, which can provide the data that the tool should work on.

  \warning Only to be instantiated by mitk::ToolManager (because SetToolManager has to be called). All other uses are unsupported.

  $Author$
  */
  class MITKSEGMENTATION_EXPORT Tool : public EventStateMachine
                                 , public InteractionEventObserver
  {
  public:

    typedef mitk::Label::PixelType DefaultSegmentationDataType;

    /**
    * \brief To let GUI process new events (e.g. qApp->processEvents() )
    */
    Message<> GUIProcessEventsMessage;

    /**
    * \brief To send error messages (to be shown by some GUI)
    */
    Message1<std::string> ErrorMessage;

    /**
    * \brief To send whether the tool is busy (to be shown by some GUI)
    */
    Message1<bool> CurrentlyBusy;

    /**
    * \brief To send general messages (to be shown by some GUI)
    */
    Message1<std::string> GeneralMessage;

    mitkClassMacro(Tool, EventStateMachine);

    // no New(), there should only be subclasses

    /**
    \brief Returns an icon in the XPM format.

    This icon has to fit into some kind of button in most applications, so make it smaller than 25x25 pixels.

    XPM is e.g. supported by The Gimp. But if you open any XPM file in your text editor, you will see that you could also "draw" it with an editor.
    */
    virtual const char** GetXPM() const = 0;

    /**
     * \brief Returns the path of an icon.
     *
     * This icon is preferred to the XPM icon.
     */
    virtual std::string GetIconPath() const { return ""; }

    /**
     * \brief Returns the path of a cursor icon.
     *
     */
    virtual us::ModuleResource GetCursorIconResource() const;

    /**
     * @brief Returns the tool button icon of the tool wrapped by a usModuleResource
     * @return a valid ModuleResource or an invalid if this function
     *         is not reimplemented
     */
    virtual us::ModuleResource GetIconResource() const;

    /**
    \brief Returns the name of this tool. Make it short!

    This name has to fit into some kind of button in most applications, so take some time to think of a good name!
    */
    virtual const char* GetName() const = 0;

    /**
    \brief Name of a group.

    You can group several tools by assigning a group name. Graphical tool selectors might use this information to group tools. (What other reason could there be?)
    */
    virtual const char* GetGroup() const;

    virtual void InitializeStateMachine();

    /**
    * \brief Interface for GUI creation.
    *
    * This is the basic interface for creation of a GUI object belonging to one tool.
    *
    * Tools that support a GUI (e.g. for display/editing of parameters) should follow some rules:
    *
    *  - A Tool and its GUI are two separate classes
    *  - There may be several instances of a GUI at the same time.
    *  - mitk::Tool is toolkit (Qt, wxWidgets, etc.) independent, the GUI part is of course dependent
    *  - The GUI part inherits both from itk::Object and some GUI toolkit class
    *  - The GUI class name HAS to be constructed like "toolkitPrefix" tool->GetClassName() + "toolkitPostfix", e.g. MyTool -> wxMyToolGUI
    *  - For each supported toolkit there is a base class for tool GUIs, which contains some convenience methods
    *  - Tools notify the GUI about changes using ITK events. The GUI must observe interesting events.
    *  - The GUI base class may convert all ITK events to the GUI toolkit's favoured messaging system (Qt -> signals)
    *  - Calling methods of a tool by its GUI is done directly.
    *    In some cases GUIs don't want to be notified by the tool when they cause a change in a tool.
    *    There is a macro CALL_WITHOUT_NOTICE(method()), which will temporarily disable all notifications during a method call.
    */
    virtual itk::Object::Pointer GetGUI(const std::string& toolkitPrefix, const std::string& toolkitPostfix);

    virtual NodePredicateBase::ConstPointer GetReferenceDataPreference() const;
    virtual NodePredicateBase::ConstPointer GetWorkingDataPreference() const;

    DataNode::Pointer CreateEmptySegmentationNode( Image* original, const std::string& organName, const mitk::Color& color );
    DataNode::Pointer CreateSegmentationNode(      Image* image,    const std::string& organName, const mitk::Color& color );

    virtual bool CanHandle(BaseData* referenceData) const;

  protected:

    friend class ToolManager;

    virtual void SetToolManager(ToolManager*);

    void ConnectActionsAndFunctions() override;

    /**
    \brief Called when the tool gets activated.

    Derived tools should call their parents implementation at the beginning of the overriding function.
    */
    virtual void Activated();

    /**
    \brief Called when the tool gets deactivated.

    Derived tools should call their parents implementation at the end of the overriding function.
    */
    virtual void Deactivated();

    /**
    \brief Let subclasses change their event configuration.
    */
    std::string m_EventConfig;

    Tool(); // purposely hidden
    Tool( const char*); // purposely hidden
    virtual ~Tool();

    virtual void Notify( InteractionEvent* interactionEvent,bool isHandled ) override;

    bool FilterEvents(InteractionEvent* , DataNode* ) override;

    ToolManager* m_ToolManager;

  private:

    // for reference data
    NodePredicateDataType::Pointer m_PredicateImages;
    NodePredicateDimension::Pointer m_PredicateDim3;
    NodePredicateDimension::Pointer m_PredicateDim4;
    NodePredicateOr::Pointer m_PredicateDimension;
    NodePredicateAnd::Pointer m_PredicateImage3D;

    NodePredicateProperty::Pointer m_PredicateBinary;
    NodePredicateNot::Pointer m_PredicateNotBinary;

    NodePredicateProperty::Pointer m_PredicateSegmentation;
    NodePredicateNot::Pointer m_PredicateNotSegmentation;

    NodePredicateProperty::Pointer m_PredicateHelper;
    NodePredicateNot::Pointer m_PredicateNotHelper;

    NodePredicateAnd::Pointer m_PredicateImageColorful;

    NodePredicateAnd::Pointer m_PredicateImageColorfulNotHelper;

    NodePredicateAnd::Pointer m_PredicateReference;

    // for working data
    NodePredicateAnd::Pointer m_IsSegmentationPredicate;

    std::string m_InteractorType;

    std::map<us::ServiceReferenceU, EventConfig> m_DisplayInteractorConfigs;

  };

} // namespace

#endif

