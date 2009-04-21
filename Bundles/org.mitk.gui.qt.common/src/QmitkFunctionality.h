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

#include <cherryQtViewPart.h>
#include <cherryIWorkbenchPartReference.h>
#include <cherryIPartListener.h>

#include <mitkDataStorage.h>

#include "mitkQtCommonDll.h"

//!mm-added
class QmitkStdMultiWidget;
class QScrollArea;

class MITK_QT_COMMON QmitkFunctionality : public cherry::QtViewPart, virtual public cherry::IPartListener
{

public:
  cherryObjectMacro(QmitkFunctionality)

  QmitkFunctionality();
  virtual ~QmitkFunctionality();
  virtual void CreatePartControl(void* parent);

  ///
  /// Called immediately before CreateQtPartControl().
  /// Actions that should be taken after creating the controls are executed here. 
  ///
  void AfterCreateQtPartControl();

  virtual void SetFocus();

  //!mm,add:some functions from QmitkFunctionality
  ///
  /// Called when the part is activated.
  ///
  virtual void Activated();
  ///
  /// Called when the part is deactivated.
  ///
  virtual void Deactivated();
  ///
  /// Called when a StdMultiWidget is available.
  ///
  virtual void StdMultiWidgetAvailable(QmitkStdMultiWidget& stdMultiWidget);
  ///
  /// Called when a StdMultiWidget is available.
  ///
  virtual void StdMultiWidgetClosed(QmitkStdMultiWidget& stdMultiWidget);
  ///
  /// Called when no StdMultiWidget is available.
  ///
  virtual void StdMultiWidgetNotAvailable();
  ///
  /// Called when a DataStorage Add Event was thrown. Sets
  /// m_InDataStorageChanged to true and calls NodeAdded afterwards.
  /// \see m_InDataStorageChanged  
  ///
  void NodeAddedProxy(const mitk::DataTreeNode* node);
  ///
  /// Called when a DataStorage Add Event was thrown. May be reimplemented
  /// by deriving classes.
  ///
  virtual void NodeAdded(const mitk::DataTreeNode* node);
  ///
  /// Called when a DataStorage remove event was thrown. Sets
  /// m_InDataStorageChanged to true and calls NodeAdded afterwards.
  /// \see m_InDataStorageChanged  
  ///
  void NodeRemovedProxy(const mitk::DataTreeNode* node);
  ///
  /// Called when a DataStorage Remove Event was thrown. May be reimplemented
  /// by deriving classes.
  ///
  virtual void NodeRemoved(const mitk::DataTreeNode* node);
  ///
  /// Called when a DataStorage add *or* remove event was thrown. May be reimplemented
  /// by deriving classes.
  ///
  virtual void DataStorageChanged();

  ///
  /// Outputs an error message to the console and displays a message box containing
  /// the exception description.
  /// @param e the exception which should be handled
  /// @param showDialog controls, whether additionally a message box should be
  ///        displayed to inform the user that something went wrong
  /// 
  void HandleException( std::exception& e, QWidget* parent = NULL, bool showDialog = true ) const;
  void HandleException( const char* str, QWidget* parent = NULL, bool showDialog = true ) const;

  // IPartListener
  virtual void 	PartActivated (cherry::IWorkbenchPartReference::Pointer partRef);
  virtual void 	PartBroughtToTop (cherry::IWorkbenchPartReference::Pointer partRef);
  virtual void 	PartClosed (cherry::IWorkbenchPartReference::Pointer partRef);
  virtual void 	PartDeactivated (cherry::IWorkbenchPartReference::Pointer partRef);
  virtual void 	PartOpened (cherry::IWorkbenchPartReference::Pointer partRef);
  virtual void 	PartHidden (cherry::IWorkbenchPartReference::Pointer partRef);
  virtual void 	PartVisible (cherry::IWorkbenchPartReference::Pointer partRef);
  virtual void 	PartInputChanged (cherry::IWorkbenchPartReference::Pointer partRef);
protected:

  void SetHandleMultipleDataStorages(bool multiple);
  bool HandlesMultipleDataStorages() const;

  mitk::DataStorage::Pointer GetDataStorage() const;
  mitk::DataStorage::Pointer GetDefaultDataStorage() const;

  QmitkStdMultiWidget* GetActiveStdMultiWidget();

  QWidget* m_Parent;
private:
  bool m_HandlesMultipleDataStorages;
  ///
  /// Saves if this class is currently working on DataStorage changes.
  /// This is a protector variable to avoid recursive calls on event listener functions.
  bool m_InDataStorageChanged;
};

#endif /*QMITKFUNCTIONALITY_H_*/
