/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef mitkObservable_h
#define mitkObservable_h

#include <mitkAlgorithm.h>

namespace mitk
{
  ///
  /// filter "Observable" interface
  ///
  class Observable
  {
  public:
    ///
    /// this class garantuees to call Update() on
    /// all observer if it changes
    /// if you need more then one observer bundle
    /// the algorithms in a BatchAlgorithm
    ///
    virtual void SetObserver( Algorithm* _Algorithm ) = 0;
  };

} // namespace mitk

#endif
