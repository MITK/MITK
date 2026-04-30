/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkDataStorageReference_h
#define mitkDataStorageReference_h

#include <MitkCoreExports.h>
#include <mitkDataStorage.h>

#include <string>

namespace mitk
{
  /**
   * \brief Lightweight metadata container for DataStorage instances.
   *
   * This class associates a DataStorage with a label and an immutable default flag.
   *
   * The IsDefault property is set at construction time and cannot be changed,
   * since default status is a property of how the storage was created.
   *
   * \ingroup DataManagement
   */
  class MITKCORE_EXPORT DataStorageReference
  {
  public:
    /** \brief Default constructor. Creates an invalid reference with no storage. */
    DataStorageReference();

    /**
     * \brief Construct a DataStorageReference with the given label, storage, and default flag.
     *
     * \param[in] label      Human-readable label for the DataStorage.
     * \param[in] storage    The DataStorage instance to associate.
     * \param[in] isDefault  If true, marks this as the default/primary DataStorage.
     */
    DataStorageReference(const std::string& label, DataStorage::Pointer storage, bool isDefault = false);

    /**
     * \brief Get the human-readable label for this DataStorage.
     *
     * \return The label string.
     */
    std::string GetLabel() const;

    /**
     * \brief Set the human-readable label for this DataStorage.
     *
     * \param[in] label  The new label string.
     */
    void SetLabel(const std::string& label);

    /**
     * \brief Get the DataStorage instance associated with this reference.
     *
     * \return The DataStorage smart pointer.
     */
    DataStorage::Pointer GetStorage() const;

    /**
     * \brief Set the DataStorage instance associated with this reference.
     *
     * \param[in] storage  The DataStorage smart pointer.
     */
    void SetStorage(DataStorage::Pointer storage);

    /**
     * \brief Check whether this is the default/primary DataStorage.
     *
     * This property is immutable after construction.
     *
     * \return True if this is the default DataStorage.
     */
    bool IsDefault() const;

    /**
     * \brief Check if this reference contains a valid (non-null) DataStorage.
     *
     * \return True if the DataStorage is not null.
     */
    bool IsValid() const;

    /**
     * \brief Equality comparison operator.
     *
     * \param[in] other  The other DataStorageReference to compare with.
     * \return True if both references have the same label, storage, and default flag.
     */
    bool operator==(const DataStorageReference& other) const;

    /**
     * \brief Inequality comparison operator.
     *
     * \param[in] other  The other DataStorageReference to compare with.
     * \return True if the references differ.
     */
    bool operator!=(const DataStorageReference& other) const;

  private:
    std::string m_Label;
    DataStorage::Pointer m_Storage;
    bool m_IsDefault{false};
  };

} // namespace mitk

#endif
