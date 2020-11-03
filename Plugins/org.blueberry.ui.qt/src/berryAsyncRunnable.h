/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYASYNCRUNNABLE_H_
#define BERRYASYNCRUNNABLE_H_

#include <Poco/ActiveRunnable.h>

namespace berry {

/**
 * This class can be used to call a method asynchronously from the UI thread using
 * the Display::AsyncExec(Poco::Runnable*) method.
 *
 */
template <class ArgType, class OwnerType>
class AsyncRunnable : public Poco::ActiveRunnableBase
{
public:
  typedef void (OwnerType::*Callback)(const ArgType&);

  AsyncRunnable(OwnerType* pOwner, Callback method, const ArgType& arg):
    _pOwner(pOwner),
    _method(method),
    _arg(arg)
  {
    poco_check_ptr (pOwner);
  }

  void run() override
  {
    ActiveRunnableBase::Ptr guard(this, false); // ensure automatic release when done
    (_pOwner->*_method)(_arg);
  }

private:
  OwnerType* _pOwner;
  Callback   _method;
  ArgType    _arg;
};

}

#endif /* BERRYASYNCRUNNABLE_H_ */
