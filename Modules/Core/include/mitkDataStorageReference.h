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
    DataStorageReference();
    DataStorageReference(const std::string& label, DataStorage::Pointer storage, bool isDefault = false);

    /** Returns the human-readable label for this DataStorage. */
    std::string GetLabel() const;
    void SetLabel(const std::string& label);

    /** Returns the DataStorage instance associated with the label. */
    DataStorage::Pointer GetStorage() const;
    void SetStorage(DataStorage::Pointer storage);

    /** True if this is the default/primary DataStorage of the DataStorage service that provided the reference.
    Immutable after construction. */
    bool IsDefault() const;

    /** Check if this info contains a valid DataStorage. */
    bool IsValid() const;

    bool operator==(const DataStorageReference& other) const;
    bool operator!=(const DataStorageReference& other) const;

  private:
    std::string m_Label;
    DataStorage::Pointer m_Storage;
    bool m_IsDefault{false};
  };

} // namespace mitk

#endif
