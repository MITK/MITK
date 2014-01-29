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

#include <MitkExports.h>
#include "mitkInteractionEventObserver.h"
#include "iostream"

namespace mitk
{
/**
   *\class EventRecorder
   *@brief Observer that enables recoding of all user interaction with the render windows and storing it in an XML file.
   *
   * @ingroup Interaction
   **/
class MITK_CORE_EXPORT EventRecorder: public InteractionEventObserver
{
public:
  EventRecorder();
  ~EventRecorder();

  /**
     * By this function the Observer gets notified about new events.
     */
  virtual void Notify(InteractionEvent* interactionEvent, bool);

  /**
     * @brief SetEventIgnoreList Optional. Provide a list of strings that describe which events are to be ignored
     */
  void SetEventIgnoreList(std::vector<std::string> list);

  void StartRecording();
  void StopRecording();

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
