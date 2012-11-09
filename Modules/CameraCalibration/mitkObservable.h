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

#endif // mitkObservable_h
