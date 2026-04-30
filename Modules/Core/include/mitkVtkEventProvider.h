/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkVtkEventProvider_h
#define mitkVtkEventProvider_h

#include <mitkRenderWindow.h>
#include <MitkCoreExports.h>

#include <vtkInteractorObserver.h>

namespace mitk
{
  /**
   * \brief Integrates into the VTK event mechanism to generate MITK-specific events.
   *
   * This class is the non-Qt-dependent counterpart to the event handling code
   * in QmitkRenderWindow. It observes VTK interactor events and translates
   * them into MITK interaction events via VtkEventAdapter.
   *
   * \ingroup Renderer
   */
  class MITKCORE_EXPORT vtkEventProvider : public vtkInteractorObserver
  {
  public:
    static vtkEventProvider *New();
    vtkTypeMacro(vtkEventProvider, vtkInteractorObserver);

    /**
     * \brief Enable or disable listening for VTK events.
     *
     * When enabling, registers observers for all interaction event types.
     * When disabling, removes all observers from the interactor.
     *
     * \param[in] enabled Non-zero to enable, zero to disable.
     */
    void SetEnabled(int enabled) override;

    /**
     * \brief Set the VTK render window interactor.
     *
     * If an interactor was previously set, it is disabled first.
     *
     * \param[in] iren The new render window interactor.
     */
    void SetInteractor(vtkRenderWindowInteractor *iren) override;

    /**
     * \brief Set the MITK RenderWindow associated with this event provider.
     * \param[in] renWin The MITK render window to associate.
     */
    virtual void SetMitkRenderWindow(mitk::RenderWindow *renWin);

    /**
     * \brief Return the associated MITK RenderWindow.
     * \return Pointer to the MITK render window.
     */
    mitk::RenderWindow *GetRenderWindow();

  protected:
    vtkEventProvider();
    ~vtkEventProvider() override;

    /**
     * \brief Callback for the VTK observer/command pattern.
     *
     * Translates VTK events into MITK interaction events and forwards
     * them to the associated RenderWindow.
     */
    static void ProcessEvents(vtkObject *object, unsigned long event, void *clientdata, void *calldata);

    mitk::RenderWindow *m_RenderWindow;

    /**
     * \brief Add a VTK event type to the list of observed interaction events.
     * \param[in] ievent The VTK event identifier to observe.
     */
    void AddInteractionEvent(unsigned long ievent);

    /**
     * \brief Remove a VTK event type from the list of observed interaction events.
     * \param[in] ievent The VTK event identifier to stop observing.
     */
    void RemoveInteractionEvent(unsigned long ievent);

    typedef std::vector<unsigned long> InteractionEventsVectorType;
    InteractionEventsVectorType m_InteractionEventsVector;

  private:
    vtkEventProvider(const vtkEventProvider &); // Not implemented.
    void operator=(const vtkEventProvider &);   // Not implemented.
  };
}
#endif
