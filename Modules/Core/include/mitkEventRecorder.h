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

#ifndef mitkEventRecorder_h
#define mitkEventRecorder_h

#include <MitkCoreExports.h>
#include "mitkInteractionEventObserver.h"
#include "iostream"

namespace mitk
{
/**
   *\class EventRecorder
   *@brief Observer that enables recoding of all user interaction with the render windows and storing it in an XML file.
   *
   * @ingroup Interaction
   *
   * XML file will look like
   *
   *  <?xml version="1.0"?>
   *  <interactions>
   *   <config>
   *    <renderer RendererName="stdmulti.widget1" ViewDirection="0"/>
   *    <renderer RendererName="stdmulti.widget2" ViewDirection="1"/>
   *    <renderer RendererName="stdmulti.widget3" ViewDirection="2"/>
   *   </config>
   *   <events>
   *    <event_variant class="MOUSEMOVEEVENT" >
   *     <attribute name="PositionOnScreen" value="491,388"/>
   *     <attribute name="PositionInWorld" value="128,235.771,124.816"/>
   *     <attribute name="RendererName" value="stdmulti.widget2"/>
   *    </event_variant>
   *   </events>
   *  </interactions>
   **/
class MITKCORE_EXPORT EventRecorder: public InteractionEventObserver
{
public:
  EventRecorder();
  ~EventRecorder();

  /**
     * By this function the Observer gets notified about new events.
     */
  virtual void Notify(InteractionEvent* interactionEvent, bool) override;

  /**
     * @brief SetEventIgnoreList Optional. Provide a list of strings that describe which events are to be ignored
     */
  void SetEventIgnoreList(std::vector<std::string> list);

  void StartRecording();
  void StopRecording();

  bool IsActive(){ return m_Active; }

  void SetOutputFile(std::string filename)
  {
    m_FileName = filename;
  }

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
