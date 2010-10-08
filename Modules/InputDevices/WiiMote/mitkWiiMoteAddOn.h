#ifndef MITK_WIIMOTEADDON_H_
#define MITK_WIIMOTEADDON_H_

#include <mitkEventMapperAddon.h>
#include <mitkInputDevicesExports.h>

#include "mitkWiiMoteIREvent.h"
#include "mitkWiiMoteButtonEvent.h"
#include "mitkWiiMoteCalibrationEvent.h"

// used for Point2D
#include "mitkVector.h"

namespace mitk {
  
  class WiiMoteThread;

  /**
  * Documentation in the interface. Some code snippets used here are from the demo <br>
  * application from gl.tter and therefore are under the same license as <code> wiimote.cpp </code>.
  *
  * @brief EventMapper addon for a Wiimote
  * @ingroup Interaction
  */
  class mitkInputDevices_EXPORT WiiMoteAddOn : public EventMapperAddOn
  {

  public:

    WiiMoteAddOn();
    ~WiiMoteAddOn();

    // Singleton
    static WiiMoteAddOn* GetInstance();

    /**
    * @see mitk::EventMapperAddOn::ForwardEvent(const mitk::StateEvent* e)
    */
    void ForwardEvent(const mitk::StateEvent* e);

    /**
    * Starts all available Wiimotes.
    */
    void ActivateWiiMotes();

    /**
    * Disconnects all Wiimotes and stops the thread.
    */
    void DeactivateWiiMotes();

    /**
    * Creates suitable events, when the Wiimote sends IR data <br>
    * and forwards it for further processing.
    */
    void WiiMoteInput(const itk::EventObject& e);

    /**
    * Creates suitable events, when the Wiimote sends button events <br>
    * and forwards it for further processing.
    */
    void WiiMoteButtonPressed(const itk::EventObject& e);

   /**
    * Creates suitable events, when the Wiimote sends IR data <br>
    * to configure the sensitivity and forwards it for further processing.
    */
    void WiiMoteCalibrationInput(const itk::EventObject& e);

  protected:

  private:

    WiiMoteThread* m_WiiMoteThread;

  }; // end class WiiMoteAddOn
} // end namespace mitk

#endif // MITK_WIIMOTEADDON_H_ 
