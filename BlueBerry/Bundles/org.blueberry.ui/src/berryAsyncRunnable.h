/*=========================================================================

 Program:   BlueBerry Platform
 Language:  C++
 Date:      $Date$
 Version:   $Revision$

 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/


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

  void run()
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
