/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkEventRecorder_h
#define mitkEventRecorder_h

#include "iostream"
#include "mitkInteractionEventObserver.h"
#include <MitkCoreExports.h>

namespace mitk
{
  /**
     *\class EventRecorder
     *@brief Observer that enables recoding of all user interaction with the render windows and storing it in an XML
     *file.
     *
     * @ingroup Interaction
     *
     * XML file will look like:
     *
     * \code{.unparsed}
     * <?xml version="1.0"?>
     * <interactions>
     *   <config>
     *     <renderer RendererName="stdmulti.widget0" ViewDirection="0"/>
     *     <renderer RendererName="stdmulti.widget1" ViewDirection="1"/>
     *     <renderer RendererName="stdmulti.widget2" ViewDirection="2"/>
     *   </config>
     *   <events>
     *     <event_variant class="MOUSEMOVEEVENT" >
     *       <attribute name="PositionOnScreen" value="491,388"/>
     *       <attribute name="PositionInWorld" value="128,235.771,124.816"/>
     *       <attribute name="RendererName" value="stdmulti.widget1"/>
     *     </event_variant>
     *   </events>
     * </interactions>
     * \endcode
     **/
  class MITKCORE_EXPORT EventRecorder : public InteractionEventObserver
  {
  public:
    EventRecorder();
    ~EventRecorder() override;

    /**
       * By this function the Observer gets notified about new events.
       */
    void Notify(InteractionEvent *interactionEvent, bool) override;

    /**
       * @brief SetEventIgnoreList Optional. Provide a list of strings that describe which events are to be ignored
       */
    void SetEventIgnoreList(std::vector<std::string> list);

    void StartRecording();
    void StopRecording();

    bool IsActive() { return m_Active; }
    void SetOutputFile(std::string filename) { m_FileName = filename; }
  private:
    /**
     * @brief m_IgnoreList lists the names of events that are dropped
     */
    std::vector<std::string> m_IgnoreList;

    /**
     * @brief m_Active determindes if events are caught and written to file
     */
    bool m_Active;
    std::string m_FileName;

    std::ofstream m_FileStream;
  };
}
#endif
