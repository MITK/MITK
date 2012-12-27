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


#ifndef QMITKFUNCTIONALITYUTIL_H
#define QMITKFUNCTIONALITYUTIL_H

#include <berryISelectionProvider.h>

class QmitkFunctionality;

#include <mitkDataNode.h>
#include <mitkDataNodeSelection.h>

///
/// Internal class for selection providing
///
class QmitkFunctionalitySelectionProvider: public berry::ISelectionProvider
{

public:

  ///
  /// Creates smartpointer typedefs
  ///
  berryObjectMacro(QmitkFunctionalitySelectionProvider)
  ///
  /// Create a selection provider for the given _Functionality
  ///
  berryNewMacro1Param(QmitkFunctionalitySelectionProvider, QmitkFunctionality*)

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
  virtual void SetSelection(berry::ISelection::ConstPointer selection);
  ///
  /// Sends the nodes as selected to the workbench
  ///
  void FireNodesSelected( std::vector<mitk::DataNode::Pointer> nodes );

protected:

  ///
  /// nothing to do here
  ///
  QmitkFunctionalitySelectionProvider(QmitkFunctionality* _Functionality);
  ///
  /// nothing to do here
  ///
  virtual ~QmitkFunctionalitySelectionProvider();
  ///
  /// the functionality parent
  ///
  QmitkFunctionality* m_Functionality;
  ///
  /// Holds the current selection (selection made by m_Functionality !!!)
  ///
  mitk::DataNodeSelection::ConstPointer m_CurrentSelection;
  ///
  /// The selection events other parts can listen too
  ///
  berry::ISelectionChangedListener::Events m_SelectionEvents;
};

#endif // QMITKFUNCTIONALITYUTIL_H
