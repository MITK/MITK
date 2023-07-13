/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef TimeNavigationController_h
#define TimeNavigationController_h

#include <MitkCoreExports.h>

#include <mitkBaseController.h>
#include <mitkTimeGeometry.h>

#include <itkCommand.h>

namespace mitk
{
  /**
   * \brief Controls the time-related properties of the time stepper, according to the
   *        given input world time geomtry.
   *
   * A TimeNavigationController takes a TimeGeometry as input world time geometry
   * and sets the properties of the associated stepper (BaseController).
   * The time-related properties are:
   *  - steps: the number of input time steps
   *  - pos: the current time step / position of the stepper
   *  - range: the time bounds for the slider (minimum, maximum)
   *
   * The timestep controls the selection of a specific time point from the TimeGeometry.
   * The TimeNavigationController generates ITK events to tell observers,
   * like a BaseRenderer, when the selected timestep changes.
   *
   * Example:
   * \code
   * // Initialize the time navigation controller.
   * timeCtrl = mitk::TimeNavigationController::New();
   *
   * // Set the required input world time geomtry (a TimeGeometry::ConstPointer).
   * timeCtrl->SetInputWorldTimeGeometry(geometry.GetPointer());
   *
   * // Set the time-related properties and send the information to the connected renderer(s).
   * timeCtrl->Update();
   * \endcode
   *
   * Vvisible navigation widgets can be connected to a TimeNavigationController, e.g., a
   * QmitkSliceNavigationWidget (for Qt):
   *
   * \code
   * // Create the visible navigation widget (a slider with a spin-box).
   * QmitkSliceNavigationWidget* navigationWidget = new QmitkSliceNavigationWidget(parent);
   *
   * // Connect the navigation widget to the time-stepper of the
   * // TimeNavigationController. For initialization (timestep, minimal and
   * // maximal values) the values of the TimeNavigationController are used.
   * // Thus, accessing methods of a navigation widget is normally not necessary,
   * // since everything can be set via the (Qt-independent) TimeNavigationController.
   * // The QmitkStepperAdapter converts the Qt-signals to Qt-independent
   * // itk-events.
   * new QmitkStepperAdapter(navigationWidget, timeCtrl->GetStepper());
   * \endcode
   */
  class MITKCORE_EXPORT TimeNavigationController : public BaseController
  {
  public:

    mitkClassMacro(TimeNavigationController, BaseController);
    itkNewMacro(Self);

    /**
     * \brief Set the input time geometry out of which the
     *        time-related properties will be generated.
     *
     * Any previous set input geometry (3D or Time) will
     * be ignored in the future.
     */
    void SetInputWorldTimeGeometry(const TimeGeometry* geometry);
    itkGetConstObjectMacro(InputWorldTimeGeometry, mitk::TimeGeometry);

    /**
     * \brief Do the actual time-related properties extraction
              and send the currently selected time step to the connecte
              observers (renderers).
     */
    virtual void Update();

    /**
     * \brief Send the currently selected time step to the connected
     *        observers (renderers).
     *
     * Called by Update().
     */
    virtual void SendTime();

    class MITKCORE_EXPORT TimeEvent : public itk::AnyEvent
    {
    public:

      typedef TimeEvent Self;
      typedef itk::AnyEvent Superclass;

      TimeEvent(TimeStepType timeStep) : m_TimeStep(timeStep) {}
      ~TimeEvent() override {}
      const char *GetEventName() const override { return "TimeEvent"; }
      bool CheckEvent(const ::itk::EventObject *e) const override { return dynamic_cast<const Self *>(e); }
      ::itk::EventObject *MakeObject() const override { return new Self(m_TimeStep); }
      TimeStepType GetTimeStep() const { return m_TimeStep; }
    private:

      TimeStepType m_TimeStep;
      void operator=(const Self &);
    };

    template <typename T>
    void ConnectTimeEvent(T* receiver)
    {
      typedef typename itk::ReceptorMemberCommand<T>::Pointer ReceptorMemberCommandPointer;
      ReceptorMemberCommandPointer eventReceptorCommand = itk::ReceptorMemberCommand<T>::New();
      eventReceptorCommand->SetCallbackFunction(receiver, &T::SetGeometryTime);
      unsigned long tag = AddObserver(TimeEvent(0), eventReceptorCommand);
      m_ReceiverToObserverTagsMap[static_cast<void *>(receiver)].push_back(tag);
    }

    // use a templated method to get the right offset when casting to void*
    template <typename T>
    void Disconnect(T* receiver)
    {
      auto i = m_ReceiverToObserverTagsMap.find(static_cast<void*>(receiver));
      if (i == m_ReceiverToObserverTagsMap.end())
        return;
      const std::list<unsigned long>& tags = i->second;
      for (auto tagIter = tags.begin(); tagIter != tags.end(); ++tagIter)
      {
        RemoveObserver(*tagIter);
      }
      m_ReceiverToObserverTagsMap.erase(i);
    }

    /**
     * \brief Convenience method that returns the time step currently selected by the controller.
     */
    TimeStepType GetSelectedTimeStep() const;

    /**
     * \brief Convenience method that returns the time point that corresponds to the selected
     *        time step. The conversion is done using the time geometry of the controller.
     *        If the time geometry is not yet set, this function will always return 0.0.
     */
    TimePointType GetSelectedTimePoint() const;

  protected:

    TimeNavigationController();
    ~TimeNavigationController() override;

    TimeGeometry::ConstPointer m_InputWorldTimeGeometry;

    bool m_BlockUpdate;

    typedef std::map<void*, std::list<unsigned long>> ObserverTagsMapType;
    ObserverTagsMapType m_ReceiverToObserverTagsMap;
  };

} // namespace mitk

#endif
