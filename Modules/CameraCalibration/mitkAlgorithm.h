/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef mitkAlgorithm_h
#define mitkAlgorithm_h

namespace mitk
{
  ///
  /// filter "Algorithm" interface = a class which does
  /// its work when calling this function
  /// this is a general polling mechansim
  ///
  class Algorithm
  {
  public:
    ///
    /// do update steps here
    ///
    virtual void Update() = 0;
  };

} // namespace mitk

#endif
