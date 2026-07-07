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
#include <mitkPropertyList.h>
#include <MitkCoreExports.h>
#include <string>

namespace mitk
{
  /**
   * \brief Observer that gets informed when a PropertyList with a given id is replaced during a Load() process.
   *
   * \sa IPersistenceService::AddPropertyListReplacedObserver()
   */
  class MITKCORE_EXPORT PropertyListReplacedObserver
  {
  public:
    /** \brief Called before the property list is replaced with new contents.
     *
     * At this point, \p propertyList still contains the old values.
     *
     * \param id the identifier of the property list being replaced.
     * \param propertyList the property list that is about to be replaced.
     */
    virtual void BeforePropertyListReplaced(const std::string &id, mitk::PropertyList *propertyList);

    /** \brief Called after the property list has been replaced with new contents.
     *
     * At this point, \p propertyList contains the new values.
     *
     * \param id the identifier of the property list that was replaced.
     * \param propertyList the property list with new values.
     */
    virtual void AfterPropertyListReplaced(const std::string &id, mitk::PropertyList *propertyList);
  };
} // namespace mitk

#endif
