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

#ifndef MITKPICKEDDATANODESELECTIONPROVIDER_H_
#define MITKPICKEDDATANODESELECTIONPROVIDER_H_

#include <org_mitk_gui_common_Export.h>

#include <berryISelectionProvider.h>

#include <mitkDataNodeSelection.h>
#include <mitkInteractionEventObserver.h>

#include <usServiceTracker.h>

namespace mitk {

class DataNodePickingEventObserver;

class MITK_GUI_COMMON_PLUGIN PickedDataNodeSelectionProvider : public berry::ISelectionProvider
{
public:

    berryObjectMacro(PickedDataNodeSelectionProvider);

    PickedDataNodeSelectionProvider();
    virtual ~PickedDataNodeSelectionProvider();

    /**
    * This method will always return a mitk::DataNodeSelection object.
    */
    void AddSelectionChangedListener(berry::ISelectionChangedListener* listener) override;

    void RemoveSelectionChangedListener(berry::ISelectionChangedListener* listener) override;

    berry::ISelection::ConstPointer GetSelection() const override;

    void SetSelection(const berry::ISelection::ConstPointer& selection) override;

protected:
    berry::ISelectionChangedListener::Events selectionEvents;

    virtual void FireSelectionChanged();

    void ConnectPickedDataNodeSelectionProviderEvents();
    void DisconnectPickedDataNodeSelectionProviderEvents();
    void SetSelectionToSingleNode(mitk::DataNode*);
    void AddNodeToSelection(mitk::DataNode*);
    void ToggleNodeSelection(mitk::DataNode*);

private:
    std::vector<mitk::DataNode::Pointer> m_CurrentSelection;
    us::ServiceTracker<mitk::InteractionEventObserver> m_InteractionEventObserverServiceTracker;
    mitk::DataNodePickingEventObserver* m_DataNodePickingEventObserver;
};

} // namespace mitk

#endif /* MITKPICKEDDATANODESELECTIONPROVIDER_H_ */
