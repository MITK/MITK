/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef mitkPropertyListReplacedObserver_h
#define mitkPropertyListReplacedObserver_h

// mitk
#include "mitkPropertyList.h"
#include <MitkCoreExports.h>
#include <string>

namespace mitk
{
  ///
  /// a PropertyListReplacedObserver gets informed as soon as a PropertyList with a given id was replaced during a
  /// Load()
  /// process
  /// \see IPersistenceService::AddPropertyListReplacedObserver()
  ///
  class MITKCORE_EXPORT PropertyListReplacedObserver
  {
  public:
    ///
    /// will be called *before* the propertyList gets replaced with new contents, i.e. propertyList still contains the
    /// old
    /// values
    ///
    virtual void BeforePropertyListReplaced(const std::string &id, mitk::PropertyList *propertyList);
    ///
    /// will be called *after* the propertyList gets replaced with new contents, i.e. propertyList contains the new
    /// values
    ///
    virtual void AfterPropertyListReplaced(const std::string &id, mitk::PropertyList *propertyList);
  };
} // namespace mitk

#endif // mitkPropertyListReplacedObserver_h
