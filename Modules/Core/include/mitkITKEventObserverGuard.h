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
#include "MitkCoreExports.h"

namespace itk
{
  class Object;
  class Command;
  class EventObject;
}

namespace mitk
{
  /**
    \brief Convenience class that helps to manage the lifetime of itk event observers.
    
    This helper class can be used to ensure itk event observers are removed form
    a sender object at the end of a certain scope.
    This class behaves similar to a std::unique_ptr but for event observers.
    Therefore the observer will be removed from the sender when one of the following
    conditions are met:
    - the guard is destroyed
    - the guard is resetted (by Reset() or operator = )

    Sample usage:
     \code
       {
         auto &objRef = *o.GetPointer();
         auto guard = ITKEventObserverGuard(o, itk::AnyEvent(), [&objRef](const itk::EventObject &event)
         { std::cout << "Object: " << objRef.GetNameOfClass() << " Event: " << event << std::endl; });
         //some code
       }
       //now the guard is destroyed
     \endcode
    This will add an Observer to o executing the lambda for any event as long as the guard exists.
    @remark If the sender is already destroyed at the moment, when the guard wants to
    remove the observer, the removal will be skipped.
  */

  class MITKCORE_EXPORT ITKEventObserverGuard
  {
  public:
    ITKEventObserverGuard();
    ITKEventObserverGuard(const itk::Object* sender, unsigned long observerTag);
    ITKEventObserverGuard(const itk::Object* sender, const itk::EventObject& event, itk::Command* command);
    ITKEventObserverGuard(const itk::Object* sender, const itk::EventObject& event, std::function<void(const itk::EventObject&)> function);
    ITKEventObserverGuard(ITKEventObserverGuard&&);
    ITKEventObserverGuard& operator=(ITKEventObserverGuard&&);

    ~ITKEventObserverGuard();

    /** Resets the guard by removing the currently guarded observer. After the reset the guard is uninitialized.
     *@remark resetting an uninitialized guard has no effect.*/
    void Reset();

    /** Resets the guard by first removing the currently guarded observer. Then the passed observer tag for the
     * passed sender will be guarded.*/
    void Reset(const itk::Object* sender, unsigned long observerTag);

    /** Resets the guard by first removing the currently guarded observer. Then a observer will be added for
     * the passed sender with the passed event and command. The new observer is now guarded.*/
    void Reset(const itk::Object* sender, const itk::EventObject& event, itk::Command* command);

    /** Resets the guard by first removing the currently guarded observer. Then a observer will be added for
     * the passed sender with the passed event and lambda function. The new observer is now guarded.*/
    void Reset(const itk::Object* sender, const itk::EventObject& event, std::function<void(const itk::EventObject&)> function);

    bool IsInitialized() const;

  private:
    struct Impl;
    std::unique_ptr<Impl> m_ITKEventObserverGuardImpl;
  };


}

#endif
