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

#endif // mitkAlgorithm_h
