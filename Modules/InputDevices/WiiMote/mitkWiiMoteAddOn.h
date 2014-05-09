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
#ifndef MITK_WIIMOTEADDON_H_
#define MITK_WIIMOTEADDON_H_

// export macro
#include <MitkWiiMoteExports.h>

// mitk
#include "mitkNumericTypes.h" // used for Point2D
#include <mitkEventMapperAddon.h>

namespace mitk {

  class WiiMoteThread;

  /**
  * Documentation in the interface. Some code snippets used here are from the demo <br>
  * application from gl.tter and therefore are under the same license as <code> wiimote.cpp </code>.
  *
  * @brief EventMapper addon for a Wiimote
  * @ingroup Interaction
  */
  class mitkWiiMote_EXPORT WiiMoteAddOn : public EventMapperAddOn
  {

  public:

    WiiMoteAddOn();
    ~WiiMoteAddOn();

    // Singleton
    static WiiMoteAddOn* GetInstance();

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
    * for movement and forwards it for further processing.
    */
    void WiiMoteInput(const itk::EventObject& e);

    /**
    * Creates suitable events, when the Wiimote sends button events <br>
    * to trigger an action and forwards it for further processing.
    */
    void WiiMoteButtonPressed(const itk::EventObject& e);

    /**
    * Creates suitable events, when the Wiimote sends button events <br>
    * to trigger an action and forwards it for further processing.
    */
    void WiiMoteButtonReleased(const itk::EventObject& e);

   /**
    * Creates suitable events, when the Wiimote sends IR data <br>
    * to configure the sensitivity and forwards it for further processing.
    */
    void WiiMoteCalibrationInput(const itk::EventObject& e);

    /**
    * @see mitk::WiiMoteThread::SetWiiMoteSurfaceIModus(bool activated)
    */
    void SetWiiMoteSurfaceIModus(bool activated);

    /**
    * Creates suitable events, regardless what type of data the <br>
    * Wiimote sends for a surface interaction. Then forwards the <br>
    * transformed data for further processing.
    *
    */
    void WiiMoteSurfaceInteractionInput(const itk::EventObject& e);

  protected:

  private:

    WiiMoteThread* m_WiiMoteThread;

  }; // end class WiiMoteAddOn
} // end namespace mitk

#endif // MITK_WIIMOTEADDON_H_
