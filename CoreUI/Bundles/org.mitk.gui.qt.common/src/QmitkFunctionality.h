/*=========================================================================

 Program:   Medical Imaging & Interaction Toolkit
 Language:  C++
 Date:      $Date$
 Version:   $Revision$

 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/

#ifndef QMITKFUNCTIONALITY_H_
#define QMITKFUNCTIONALITY_H_

#ifdef __MINGW32__
// We need to inlclude winbase.h here in order to declare
// atomic intrinsics like InterlockedIncrement correctly.
// Otherwhise, they would be declared wrong within qatomic_windows.h .
#include <windows.h>
#endif

//# blueberry stuff
#include <berryQtViewPart.h>
#include <berryIWorkbenchPartReference.h>
#include <berryIPreferencesService.h>
#include <berryIBerryPreferences.h>
#include <berryISelectionListener.h>
#include <berryISelectionProvider.h>
#include <berryISelectionChangedListener.h>

//# mitk stuff
#include "mitkQtCommonDll.h"
#include "mitkDataNodeSelection.h"
#include <mitkDataStorage.h>
#include <mitkDataNodeSelection.h>

//# forward declarations
class QmitkStdMultiWidget;
class QScrollArea;

///
/// \class QmitkFunctionality
///
/// \brief The base class of all MITK related blueberry views (~ in the old version of MITK, this was called "Functionality")
///
/// QmitkFunctionality provides several convenience methods that eases the introduction of a new view:
///
/// <ol>
///   <li> Access to the DataStorage (~ the shared data repository)
///   <li> Access to the StdMultiWidget (the 2x2 RenderWindow arrangement)
///   <li> Access to and update notification for the functionality/view preferences
///   <li> Access to and update notification for the current DataNode selection / to DataNode selection events send through the SelectionService
///   <li> Methods to send DataNode selections through the SelectionService
///   <li> Some events for unproblematic inter-View communication (e.g. when to add/remove interactors)
///   <li> Some minor important convenience methods (like changing the mouse cursor/exception handling)
/// </ol>
///
/// Please use the Activated/Deactivated method to add/remove interactors, disabling multiwidget crosshair or anything which may 
/// "affect" other functionalities. For further reading please have a look at QmitkFunctionality::IsExclusiveFunctionality().
///
class MITK_QT_COMMON QmitkFunctionality : public berry::QtViewPart, virtual public berry::ISelectionProvider
{

//# public virtual methods which can be overwritten
public:
  ///
  /// Creates smartpointer typedefs
  ///
  berryObjectMacro(QmitkFunctionality)
  ///
  /// Nothing to do in the standard ctor. <b>Initiliaze your GUI in CreateQtPartControl(QWidget*)</b>
  /// \see berry::QtViewPart::CreateQtPartControl(QWidget*)
  ///
  QmitkFunctionality();
  ///
  /// Disconnects all standard event listeners
  ///
  virtual ~QmitkFunctionality();
  ///
  /// Called, when the WorkbenchPart gets closed for removing event listeners.
  /// Do not unregister your event listener in the destructor as the workbench
  /// gets closed before. If in any event processing function the GUI gets accessed
  /// your app might crash.
  ///
  virtual void ClosePart();
  ///
  /// Called when the selection in the workbench changed
  ///
  virtual void OnSelectionChanged(std::vector<mitk::DataNode*> nodes);
  ///
  /// Called when the preferences object of this view changed.
  /// \see GetPreferences()
  ///
  virtual void OnPreferencesChanged(const berry::IBerryPreferences*);
  ///
  /// Make this view manage multiple DataStorage. If set to true GetDataStorage()
  /// will return the currently active DataStorage (and not the default one).
  /// \see GetDataStorage()
  ///
  void SetHandleMultipleDataStorages(bool multiple);
  ///
  /// \return true if this view handles multiple DataStorages, false otherwise
  ///
  bool HandlesMultipleDataStorages() const;
  ///
  /// Called when a StdMultiWidget is available. Should not be used anymore, see GetActiveStdMultiWidget()
  /// \see GetActiveStdMultiWidget()
  ///
  virtual void StdMultiWidgetAvailable(QmitkStdMultiWidget& stdMultiWidget);
  ///
  /// Called when a StdMultiWidget is available. Should not be used anymore, see GetActiveStdMultiWidget()
  /// \see GetActiveStdMultiWidget()
  ///
  virtual void StdMultiWidgetClosed(QmitkStdMultiWidget& stdMultiWidget);
  ///
  /// Called when no StdMultiWidget is available anymore. Should not be used anymore, see GetActiveStdMultiWidget()
  /// \see GetActiveStdMultiWidget()
  ///
  virtual void StdMultiWidgetNotAvailable();
  ///
  /// Only called when IsExclusiveFunctionality() returns true.
  /// \see IsExclusiveFunctionality()
  ///
  virtual void Activated();
  ///
  /// \return true if this view is currently activated, false otherwise
  ///
  bool IsActivated() const;
  ///
  /// Only called when IsExclusiveFunctionality() returns true.
  /// \see IsExclusiveFunctionality()
  ///
  virtual void Deactivated();
  ///
  /// Some functionalities need to add special interactors, removes the crosshair from the stdmultiwidget, etc.
  /// In this case the functionality has to tidy up when changing to another functionality 
  /// which also wants to do change the "default configuration". In the old Qt3-based
  /// version of MITK two functionalities could never be opened at the same time so that the 
  /// methods Activated() and Deactivated() were the right place for the functionalitites to 
  /// add/remove their interactors, etc. This is still true for the new MITK Workbench,
  /// but as there can be several functionalities visible at the same time the behaviour concerning 
  /// when Activated() and Deactivated() are called:
  ///
  /// 1. Activated() and Deactivated() are only called if IsExclusiveFunctionality() returns true 
  ///
  /// 2. If only one standalone functionality is or becomes visible, Activated() will be called on that functionality
  ///
  /// 3. If two or more standalone functionalities are visible,
  ///    Activated() will be called on the functionality that receives focus, Deactivated() will be called 
  ///    on the one that looses focus, gets hidden or closed
  ///
  ///
  /// As a consequence of 1. if you overwrite IsExclusiveFunctionality() and let it return false, you
  /// signalize the MITK Workbench that this functionality does nothing to the "default configuration"
  /// and can easily be visible while other functionalities are also visible.
  ///
  /// By default the method returns true.
  ///
  /// \return true if this functionality is meant to work as a standalone view, false otherwise
  ///
  virtual bool IsExclusiveFunctionality() const;
  ///
  /// Informs other parts of the workbench that node is selected via the blueberry selection service.
  ///
  void FireNodeSelected(mitk::DataNode::Pointer node);
  ///
  /// Informs other parts of the workbench that the nodes are selected via the blueberry selection service.
  ///
  void FireNodesSelected(std::vector<mitk::DataNode::Pointer> nodes);
  ///
  /// Called when this functionality becomes visible ( no matter what IsExclusiveFunctionality() returns )
  ///
  virtual void Visible();
  ///
  /// \return true if this view is currently visible, false otherwise
  ///
  bool IsVisible() const;
  ///
  /// Called when this functionality is hidden ( no matter what IsExclusiveFunctionality() returns )
  ///
  virtual void Hidden();
//# protected virtual methods which can be overwritten
protected:
  ///
  /// Called when a DataStorage Add event was thrown. May be reimplemented
  /// by deriving classes.
  ///
  virtual void NodeAdded(const mitk::DataNode* node);
  ///
  /// Called when a DataStorage Changed event was thrown. May be reimplemented
  /// by deriving classes.
  ///
  virtual void NodeChanged(const mitk::DataNode* node);
  ///
  /// Called when a DataStorage Remove event was thrown. May be reimplemented
  /// by deriving classes.
  ///
  virtual void NodeRemoved(const mitk::DataNode* node);
  ///
  /// Called when a DataStorage add *or* remove *or* change event was thrown. May be reimplemented
  /// by deriving classes.
  ///
  virtual void DataStorageChanged();
  ///
  /// \return the selection of the currently active part of the workbench or an empty vector
  ///         if nothing is selected
  ///    
  std::vector<mitk::DataNode*> GetCurrentSelection() const;
  ///
  /// Returns the current selection made in the datamanager bundle or an empty vector
  /// if nothing`s selected or if the bundle does not exist
  ///
  std::vector<mitk::DataNode*> GetDataManagerSelection() const;
  ///
  /// Returns the Preferences object for this Functionality.
  /// <b>Important</b>: When refering to this preferences, e.g. in a PreferencePage: The ID
  /// for this preferences object is "/<VIEW-ID>", e.g. "/org.mitk.views.datamanager"
  /// 
  berry::IPreferences::Pointer GetPreferences() const;
  ///
  /// Returns the default <b>or</b> the currently active DataStorage if m_HandlesMultipleDataStorages
  /// is set to true
  /// \see SetHandleMultipleDataStorages(bool)
  /// \see HandlesMultipleDataStorages()
  ///
  mitk::DataStorage::Pointer GetDataStorage() const;
  ///
  /// \return always returns the default DataStorage
  ///
  mitk::DataStorage::Pointer GetDefaultDataStorage() const;
  ///
  /// Returns the default and active StdMultiWidget.
  /// <b>If there is not StdMultiWidget yet a new one is created in this method!</b>
  ///
  QmitkStdMultiWidget* GetActiveStdMultiWidget();
  ///
  /// Outputs an error message to the console and displays a message box containing
  /// the exception description.
  /// \param e the exception which should be handled
  /// \param showDialog controls, whether additionally a message box should be
  ///        displayed to inform the user that something went wrong
  /// 
  void HandleException( std::exception& e, QWidget* parent = NULL, bool showDialog = true ) const;
  ///
  /// Calls HandleException ( std::exception&, QWidget*, bool ) internally
  /// \see HandleException ( std::exception&, QWidget*, bool )
  ///
  void HandleException( const char* str, QWidget* parent = NULL, bool showDialog = true ) const;  
  ///
  /// Convenient method to set and reset a wait cursor ("hourglass")
  /// 
  void WaitCursorOn();
  ///
  /// Convenient method to restore the standard cursor
  ///
  void WaitCursorOff();
  ///
  /// Convenient method to set and reset a busy cursor
  /// 
  void BusyCursorOn();
  ///
  /// Convenient method to restore the standard cursor
  ///
  void BusyCursorOff();
  ///
  /// Convenient method to restore the standard cursor
  ///
  void RestoreOverrideCursor();

//# other public methods which should not be overwritten
public:
  ///
  /// Creates a scroll area for this view and calls CreateQtPartControl then
  ///
  void CreatePartControl(void* parent);
  ///
  /// Called when this view receives the focus. Same as Activated()
  /// \see Activated()
  ///
  void SetFocus();
  ///
  /// Called when a DataStorage Add Event was thrown. Sets
  /// m_InDataStorageChanged to true and calls NodeAdded afterwards.
  /// \see m_InDataStorageChanged  
  ///
  void NodeAddedProxy(const mitk::DataNode* node);
  ///
  /// Called when a DataStorage remove event was thrown. Sets
  /// m_InDataStorageChanged to true and calls NodeRemoved afterwards.
  /// \see m_InDataStorageChanged  
  ///
  void NodeRemovedProxy(const mitk::DataNode* node);
  ///
  /// Called when a DataStorage changed event was thrown. Sets
  /// m_InDataStorageChanged to true and calls NodeChanged afterwards.
  /// \see m_InDataStorageChanged  
  ///
  void NodeChangedProxy(const mitk::DataNode* node);
  ///
  /// Toggles the visible flag m_Visible
  ///
  void SetVisible(bool visible);
  ///
  /// Toggles the activated flag m_Activated
  ///
  void SetActivated(bool activated);

  //# ISelectionProvider methods
  ///
  /// \see ISelectionProvider::AddSelectionChangedListener()
  ///
  virtual void AddSelectionChangedListener(berry::ISelectionChangedListener::Pointer listener);
  ///
  /// \see ISelectionProvider::GetSelection()
  ///
  virtual berry::ISelection::ConstPointer GetSelection() const;
  ///
  /// \see ISelectionProvider::RemoveSelectionChangedListener()
  ///
  virtual void RemoveSelectionChangedListener(berry::ISelectionChangedListener::Pointer listener);
  ///
  /// \see ISelectionProvider::SetSelection()
  ///
  virtual void SetSelection(berry::ISelection::Pointer selection);
  ///
  /// Called, when the WorkbenchPart gets closed for removing event listeners
  /// Internally this method calls ClosePart after it removed the listeners registered
  /// by QmitkFunctionality. By having this proxy method the user does not have to
  /// call QmitkFunctionality::ClosePart() when overwriting ClosePart()
  ///
  void ClosePartProxy();

//# other protected methods which should not be overwritten (or which are deprecated)
protected:
  ///
  /// Called immediately after CreateQtPartControl().
  /// Here standard event listeners for a QmitkFunctionality are registered
  ///
  void AfterCreateQtPartControl();
  ///
  /// code to activate the last visible functionality
  ///
  void ActivateLastVisibleFunctionality();
  /// 
  /// reactions to selection events from data manager (and potential other senders)
  ///
  void BlueBerrySelectionChanged(berry::IWorkbenchPart::Pointer sourcepart, berry::ISelection::ConstPointer selection);
  ///
  /// Converts a mitk::DataNodeSelection to a std::vector<mitk::DataNode*> (possibly empty
  ///
  std::vector<mitk::DataNode*> DataNodeSelectionToVector(mitk::DataNodeSelection::ConstPointer currentSelection) const;
  //# protected fields
protected:
  /// 
  /// helper stuff to observe BlueBerry selections
  ///
  friend struct berry::SelectionChangedAdapter<QmitkFunctionality>;
  ///
  /// Saves the parent of this view (this is the scrollarea created in CreatePartControl(void*)
  /// \see CreatePartControl(void*)
  ///
  QWidget* m_Parent;
  ///
  /// Saves if this view is the currently active one.
  ///
  bool m_Active;
  ///
  /// Saves if this view is visible
  ///
  bool m_Visible;

//# private fields:
private:
  ///
  /// The selection events other parts can listen too (needed for QmitkFunctionality to be a selection provider)
  ///
  berry::ISelectionChangedListener::Events m_SelectionEvents;
  ///
  /// Holds the current selection (selection made by this Functionality !!!)
  ///
  mitk::DataNodeSelection::Pointer m_CurrentSelection;
  ///
  /// object to observe BlueBerry selections 
  ///
  berry::ISelectionListener::Pointer m_BlueBerrySelectionListener;
  ///
  /// Saves if this view handles multiple datastorages
  ///
  bool m_HandlesMultipleDataStorages;
  ///
  /// Saves if this class is currently working on DataStorage changes.
  /// This is a protector variable to avoid recursive calls on event listener functions.
  bool m_InDataStorageChanged;
  ///
  /// saves all visible functionalities
  ///
  std::set<std::string> m_VisibleFunctionalities;
  ///
  /// The Preferences Service to retrieve and store preferences.
  ///
  berry::IPreferencesService::WeakPtr m_PreferencesService;
};

#endif /*QMITKFUNCTIONALITY_H_*/
