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

#ifndef DataNodePickingEventObserver_h
#define DataNodePickingEventObserver_h

#include <MitkCoreExports.h>
#include "mitkPickingEventObserver.h"

namespace mitk
{
    /**
    * \class DataNodePickingEventObserver
    */

    class MITKCORE_EXPORT DataNodePickingEventObserver: public PickingEventObserver
    {
    public:
        DataNodePickingEventObserver();
        virtual ~DataNodePickingEventObserver();

        mitk::Message1<mitk::DataNode*> SingleNodePickEvent;
        mitk::Message1<mitk::DataNode*> AddNodePickEvent;
        mitk::Message1<mitk::DataNode*> ToggleNodePickEvent;

        static void SetEnabled(bool enabled);
        static bool IsEnabled();
    protected:
        virtual void HandlePickOneEvent(InteractionEvent* interactionEvent) override;
        virtual void HandlePickAddEvent(InteractionEvent* interactionEvent) override;
        virtual void HandlePickToggleEvent(InteractionEvent* interactionEvent) override;

        mitk::DataNode* GetPickedDataNode(InteractionEvent* interactionEvent);

        static bool m_Enabled;
    };

} /* namespace mitk */

#endif /* DataNodePickingEventObserver_h */
