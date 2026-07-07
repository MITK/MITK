/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkITKEventObserverGuard_h
#define mitkITKEventObserverGuard_h

#include <functional>
#include <memory>
#include <MitkCoreExports.h>

namespace itk
{
  class Object;
  class Command;
  class EventObject;
}

namespace mitk
{
  /**
    * \brief Convenience class that helps to manage the lifetime of itk event observers.
    *
    * This helper class can be used to ensure itk event observers are removed from
    * a sender object at the end of a certain scope.
    * This class behaves similar to a std::unique_ptr but for event observers.
    * Therefore the observer will be removed from the sender when one of the following
    * conditions are met:
    * - the guard is destroyed
    * - the guard is reset (by Reset() or move assignment)
    *
    * Sample usage:
    * \code
    *   {
    *     auto &objRef = *o.GetPointer();
    *     auto guard = ITKEventObserverGuard(o, itk::AnyEvent(), [&objRef](const itk::EventObject &event)
    *     { std::cout << "Object: " << objRef.GetNameOfClass() << " Event: " << event << std::endl; });
    *     //some code
    *   }
    *   //now the guard is destroyed
    * \endcode
    *
    * This will add an observer to \c o executing the lambda for any event as long as the guard exists.
    *
    * \note If the sender is already destroyed at the moment when the guard wants to
    * remove the observer, the removal will be skipped.
    */

  class MITKCORE_EXPORT ITKEventObserverGuard
  {
  public:
    /** \brief Construct an uninitialized guard (no observer is guarded). */
    ITKEventObserverGuard();

    /**
     * \brief Construct a guard for an existing observer tag.
     *
     * \param sender The ITK object that owns the observer.
     * \param observerTag The tag returned by itk::Object::AddObserver().
     */
    ITKEventObserverGuard(const itk::Object* sender, unsigned long observerTag);

    /**
     * \brief Construct a guard that adds and manages an observer using an itk::Command.
     *
     * \param sender The ITK object to observe.
     * \param event The event type to observe.
     * \param command The command to execute when the event occurs.
     */
    ITKEventObserverGuard(const itk::Object* sender, const itk::EventObject& event, itk::Command* command);

    /**
     * \brief Construct a guard that adds and manages an observer using a lambda or function.
     *
     * \param sender The ITK object to observe.
     * \param event The event type to observe.
     * \param function The function to execute when the event occurs.
     */
    ITKEventObserverGuard(const itk::Object* sender, const itk::EventObject& event, std::function<void(const itk::EventObject&)> function);

    /** \brief Move constructor. Transfers ownership of the guarded observer. */
    ITKEventObserverGuard(ITKEventObserverGuard&&);

    /** \brief Move assignment. Releases any currently guarded observer and takes ownership from the other guard. */
    ITKEventObserverGuard& operator=(ITKEventObserverGuard&&);

    /** \brief Destructor. Removes the guarded observer from the sender if still alive. */
    ~ITKEventObserverGuard();

    /**
     * \brief Reset the guard by removing the currently guarded observer.
     *
     * After the reset, the guard is uninitialized.
     *
     * \note Resetting an uninitialized guard has no effect.
     */
    void Reset();

    /**
     * \brief Reset the guard and start guarding a new observer tag.
     *
     * First removes the currently guarded observer, then guards the passed observer tag
     * for the passed sender.
     *
     * \param sender The ITK object that owns the observer.
     * \param observerTag The tag returned by itk::Object::AddObserver().
     */
    void Reset(const itk::Object* sender, unsigned long observerTag);

    /**
     * \brief Reset the guard and add a new observer using an itk::Command.
     *
     * First removes the currently guarded observer, then adds a new observer for
     * the passed sender with the passed event and command. The new observer is now guarded.
     *
     * \param sender The ITK object to observe.
     * \param event The event type to observe.
     * \param command The command to execute when the event occurs.
     */
    void Reset(const itk::Object* sender, const itk::EventObject& event, itk::Command* command);

    /**
     * \brief Reset the guard and add a new observer using a lambda or function.
     *
     * First removes the currently guarded observer, then adds a new observer for
     * the passed sender with the passed event and function. The new observer is now guarded.
     *
     * \param sender The ITK object to observe.
     * \param event The event type to observe.
     * \param function The function to execute when the event occurs.
     */
    void Reset(const itk::Object* sender, const itk::EventObject& event, std::function<void(const itk::EventObject&)> function);

    /**
     * \brief Check whether this guard is currently managing an observer.
     *
     * \return \c true if the guard is initialized and managing an observer, \c false otherwise.
     */
    bool IsInitialized() const;

  private:
    struct Impl;
    std::unique_ptr<Impl> m_ITKEventObserverGuardImpl;
  };


}

#endif
