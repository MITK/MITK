/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef mitkIPersistable_h
#define mitkIPersistable_h
#include <string>

namespace mitk
{
  /**
   * \brief Abstract interface for objects that can persist their state to and from files.
   *
   * Classes implementing this interface provide the ability to serialize their internal
   * state to a file and restore it later. Each persistable object is identified by a
   * unique string identifier.
   *
   * \sa IPersistenceService
   * \sa PersistenceService
   */
  class MITKPERSISTENCE_EXPORT IPersistable
  {
  public:
    /**
     * \brief Save the object's state to a file.
     *
     * \param[in] fileName Path to the file to save to. If empty, a default file is used.
     * \return True if the state was saved successfully, false otherwise.
     */
    virtual bool Save(const std::string &fileName = "") = 0;

    /**
     * \brief Load the object's state from a file.
     *
     * \param[in] fileName Path to the file to load from. If empty, a default file is used.
     * \return True if the state was loaded successfully, false otherwise.
     */
    virtual bool Load(const std::string &fileName = "") = 0;

    /**
     * \brief Set the unique identifier for this persistable object.
     *
     * \param[in] id The unique string identifier to assign.
     */
    virtual void SetId(const std::string &id) = 0;

    /**
     * \brief Get the unique identifier of this persistable object.
     *
     * \return The unique string identifier.
     */
    virtual std::string GetId() const = 0;
  };
}

#endif
