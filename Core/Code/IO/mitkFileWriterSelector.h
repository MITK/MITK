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

#ifndef MITKFILEWRITERSELECTOR_H
#define MITKFILEWRITERSELECTOR_H

#include <mitkIFileWriter.h>

#include <mitkMimeType.h>

#include <usServiceReference.h>

#include <string>
#include <vector>

namespace mitk {

class BaseData;

class MITK_CORE_EXPORT FileWriterSelector
{
public:

  class MITK_CORE_EXPORT Item
  {
  public:

    Item(const Item& other);
    ~Item();

    Item& operator=(const Item& other);

    IFileWriter* GetWriter() const;
    std::string GetDescription() const;
    IFileWriter::ConfidenceLevel GetConfidenceLevel() const;
    MimeType GetMimeType() const;
    std::string GetBaseDataType() const;
    us::ServiceReference<IFileWriter> GetReference() const;
    long GetServiceId() const;

    bool operator<(const Item& other) const;

  private:

    friend class FileWriterSelector;

    Item();

    struct Impl;
    us::SharedDataPointer<Impl> d;
  };

  FileWriterSelector(const FileWriterSelector& other);
  FileWriterSelector(const BaseData* baseData, const std::string& destMimeType = std::string(),
                     const std::string& path = std::string());

  ~FileWriterSelector();

  FileWriterSelector& operator=(const FileWriterSelector& other);

  bool IsEmpty() const;

  /**
   * @brief Get a sorted list of file writer info objects.
   *
   * <ol>
   * <li>Confidence level (ascending)</li>
   * <li>File Writer service ranking (ascending)</li>
   * </ol>
   *
   * This means the best matching item is at the back of the returned
   * container.
   *
   * @param mimeType
   * @return
   */
  std::vector<Item> Get(const std::string& mimeType) const;

  /**
   * @brief Get a sorted list of file writer info objects.
   *
   * The returned objects will have the same mime-type as the currently
   * selected item.
   *
   * @return Ordered list of file writer items.
   */
  std::vector<Item> Get() const;

  Item Get(long id) const;

  Item GetDefault() const;
  long GetDefaultId() const;

  Item GetSelected() const;
  long GetSelectedId() const;

  bool Select(const std::string& mimeType);
  bool Select(const Item& item);
  bool Select(long id);

  std::vector<MimeType> GetMimeTypes() const;

  void Swap(FileWriterSelector& fws);

private:

  struct Impl;
  us::ExplicitlySharedDataPointer<Impl> m_Data;
};

void swap(FileWriterSelector& fws1, FileWriterSelector& fws2);

}

#endif // MITKFILEWRITERSELECTOR_H
