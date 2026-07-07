/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkEventRecorder_h
#define mitkEventRecorder_h

#include <iostream>
#include <mitkInteractionEventObserver.h>
#include <MitkCoreExports.h>

namespace mitk
{
  /**
     * \class EventRecorder
     * \brief Observer that enables recording of all user interaction with the render windows and storing it in an XML
     * file.
     *
     * \ingroup Interaction
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
     *
     * \sa InteractionEventObserver
     * \sa EventFactory
     **/
  class MITKCORE_EXPORT EventRecorder : public InteractionEventObserver
  {
  public:
    /** \brief Constructor. */
    EventRecorder();

    /** \brief Destructor. Closes any open file stream. */
    ~EventRecorder() override;

    /**
     * \brief Callback that receives new interaction events.
     *
     * This function is called by the event dispatching system to notify the recorder
     * about new events. The event is serialized to XML and written to the output file.
     *
     * \param interactionEvent The interaction event to record.
     */
    void Notify(InteractionEvent *interactionEvent, bool) override;

    /**
     * \brief Set a list of event names to ignore during recording.
     *
     * Events whose class name matches any entry in the list will not be recorded.
     *
     * \param list Vector of event class names to ignore.
     */
    void SetEventIgnoreList(std::vector<std::string> list);

    /**
     * \brief Start recording events to the output file.
     *
     * \pre The output file must be set via SetOutputFile() before calling this method.
     */
    void StartRecording();

    /**
     * \brief Stop recording and close the output file.
     */
    void StopRecording();

    /**
     * \brief Check whether the recorder is currently active.
     *
     * \return \c true if recording is in progress, \c false otherwise.
     */
    bool IsActive() { return m_Active; }

    /**
     * \brief Set the output file path for recording.
     *
     * \param filename Path to the XML output file.
     */
    void SetOutputFile(std::string filename) { m_FileName = filename; }
  private:
    /**
     * \brief Lists the names of events that are dropped during recording.
     */
    std::vector<std::string> m_IgnoreList;

    /**
     * \brief Determines if events are caught and written to file.
     */
    bool m_Active;
    std::string m_FileName;

    std::ofstream m_FileStream;
  };
}
#endif
