/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


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
  berryObjectMacro(QmitkFunctionalitySelectionProvider);

  QmitkFunctionalitySelectionProvider(QmitkFunctionality* _Functionality);

  ~QmitkFunctionalitySelectionProvider() override;

  //# ISelectionProvider methods
  ///
  /// \see ISelectionProvider::AddSelectionChangedListener()
  ///
  void AddSelectionChangedListener(berry::ISelectionChangedListener* listener) override;
  ///
  /// \see ISelectionProvider::GetSelection()
  ///
  berry::ISelection::ConstPointer GetSelection() const override;
  ///
  /// \see ISelectionProvider::RemoveSelectionChangedListener()
  ///
  void RemoveSelectionChangedListener(berry::ISelectionChangedListener* listener) override;
  ///
  /// \see ISelectionProvider::SetSelection()
  ///
  void SetSelection(const berry::ISelection::ConstPointer& selection) override;
  ///
  /// Sends the nodes as selected to the workbench
  ///
  void FireNodesSelected(const std::vector<mitk::DataNode::Pointer>& nodes );

protected:

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
