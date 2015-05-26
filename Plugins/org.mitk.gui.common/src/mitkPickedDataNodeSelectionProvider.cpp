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

#include "mitkPickedDataNodeSelectionProvider.h"

#include "internal/org_mitk_gui_common_Activator.h"

#include <mitkDataNodePickingEventObserver.h>
#include <usModuleRegistry.h>
#include <usModule.h>

namespace mitk {


    PickedDataNodeSelectionProvider::PickedDataNodeSelectionProvider()
        : m_InteractionEventObserverServiceTracker(us::ModuleRegistry::GetModule(1)->GetModuleContext(), us::LDAPFilter("(name=DataNodePicker)"))
        , m_DataNodePickingEventObserver(NULL)
    {
        m_InteractionEventObserverServiceTracker.Open();
        ConnectPickedDataNodeSelectionProviderEvents();
    }

    PickedDataNodeSelectionProvider::~PickedDataNodeSelectionProvider()
    {
        DisconnectPickedDataNodeSelectionProviderEvents();
    }

    void PickedDataNodeSelectionProvider::AddSelectionChangedListener(berry::ISelectionChangedListener* listener)
    {
        selectionEvents.AddListener(listener);
    }

    void PickedDataNodeSelectionProvider::RemoveSelectionChangedListener(berry::ISelectionChangedListener* listener)
    {
        selectionEvents.RemoveListener(listener);
    }

    berry::ISelection::ConstPointer PickedDataNodeSelectionProvider::GetSelection() const
    {
        return berry::ISelection::ConstPointer(new mitk::DataNodeSelection(m_CurrentSelection));
    }

    void PickedDataNodeSelectionProvider::SetSelection(const berry::ISelection::ConstPointer& selection)
    {
        mitk::DataNodeSelection::ConstPointer dataNodeSelection = selection.Cast<const mitk::DataNodeSelection>();
        if (dataNodeSelection) {
            std::list<mitk::DataNode::Pointer> selectedNodes = dataNodeSelection->GetSelectedDataNodes();

            m_CurrentSelection.clear();
            std::copy(selectedNodes.begin(), selectedNodes.end(), std::back_inserter(m_CurrentSelection));

            FireSelectionChanged();
        }
    }

    void PickedDataNodeSelectionProvider::FireSelectionChanged()
    {
        berry::SelectionChangedEvent::Pointer event(new berry::SelectionChangedEvent(
            berry::ISelectionProvider::Pointer(this), GetSelection()));
        selectionEvents.selectionChanged(event);
    }

    void PickedDataNodeSelectionProvider::ConnectPickedDataNodeSelectionProviderEvents()
    {
        DisconnectPickedDataNodeSelectionProviderEvents();

        mitk::InteractionEventObserver* obsService = m_InteractionEventObserverServiceTracker.GetService();
        m_DataNodePickingEventObserver = dynamic_cast<mitk::DataNodePickingEventObserver*>(obsService);

        if (m_DataNodePickingEventObserver == NULL)
        {
            return;
        }


        m_DataNodePickingEventObserver->SingleNodePickEvent.AddListener(mitk::MessageDelegate1 < PickedDataNodeSelectionProvider, mitk::DataNode* >
            (this, &PickedDataNodeSelectionProvider::SetSelectionToSingleNode));

        m_DataNodePickingEventObserver->AddNodePickEvent.AddListener(mitk::MessageDelegate1 < PickedDataNodeSelectionProvider, mitk::DataNode* >
            (this, &PickedDataNodeSelectionProvider::AddNodeToSelection));

        m_DataNodePickingEventObserver->ToggleNodePickEvent.AddListener(mitk::MessageDelegate1 < PickedDataNodeSelectionProvider, mitk::DataNode* >
            (this, &PickedDataNodeSelectionProvider::ToggleNodeSelection));
    }

    void PickedDataNodeSelectionProvider::DisconnectPickedDataNodeSelectionProviderEvents()
    {
        if (!m_DataNodePickingEventObserver) {
            return;
        }

        m_DataNodePickingEventObserver->SingleNodePickEvent.RemoveListener(mitk::MessageDelegate1 < PickedDataNodeSelectionProvider, mitk::DataNode* >
            (this, &PickedDataNodeSelectionProvider::SetSelectionToSingleNode));

        m_DataNodePickingEventObserver->AddNodePickEvent.RemoveListener(mitk::MessageDelegate1 < PickedDataNodeSelectionProvider, mitk::DataNode* >
            (this, &PickedDataNodeSelectionProvider::AddNodeToSelection));

        m_DataNodePickingEventObserver->ToggleNodePickEvent.RemoveListener(mitk::MessageDelegate1 < PickedDataNodeSelectionProvider, mitk::DataNode* >
            (this, &PickedDataNodeSelectionProvider::ToggleNodeSelection));

        m_DataNodePickingEventObserver = NULL;
    }

    void PickedDataNodeSelectionProvider::SetSelectionToSingleNode(mitk::DataNode* node)
    {
        m_CurrentSelection.clear();
        if (node) {
            m_CurrentSelection.push_back(node);
        }

        FireSelectionChanged();
    }

    void PickedDataNodeSelectionProvider::AddNodeToSelection(mitk::DataNode* node)
    {
        if (std::find(m_CurrentSelection.begin(), m_CurrentSelection.end(), mitk::DataNode::Pointer(node)) == m_CurrentSelection.end()) {
            m_CurrentSelection.push_back(node);
            FireSelectionChanged();
        }
    }

    void PickedDataNodeSelectionProvider::ToggleNodeSelection(mitk::DataNode* node)
    {
        std::vector<mitk::DataNode::Pointer>::iterator iter = std::find(m_CurrentSelection.begin(), m_CurrentSelection.end(), mitk::DataNode::Pointer(node));
        if (iter == m_CurrentSelection.end()) {
            m_CurrentSelection.push_back(node);
        }
        else {
            m_CurrentSelection.erase(iter);
        }
        FireSelectionChanged();
    }
} // namespace mitk
