/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkFileReaderSelector_h
#define mitkFileReaderSelector_h

#include <mitkIFileReader.h>

#include <mitkMimeType.h>

#include <usServiceReference.h>

#include <string>
#include <vector>

namespace mitk
{
  class BaseData;

  /**
   * \ingroup IO
   *
   * \brief Selects the most suitable IFileReader for a given file path.
   *
   * FileReaderSelector queries the service registry for all IFileReader
   * services whose registered mime-types match the given file path. It
   * ranks the matching readers by confidence level, mime-type ordering,
   * and service ranking, and provides access to the best-matching
   * (default) reader as well as all candidates.
   *
   * \sa IFileReader
   * \sa FileReaderRegistry
   * \sa FileWriterSelector
   * \sa MimeType
   */
  class MITKCORE_EXPORT FileReaderSelector
  {
  public:
    /**
     * \brief Represents a single candidate file reader with associated metadata.
     *
     * Items are comparable and sorted by confidence level, mime-type rank,
     * and service ranking. The best-matching item sorts last (highest).
     */
    class MITKCORE_EXPORT Item
    {
    public:
      /**
       * \brief Copy constructor.
       * \param[in] other The Item to copy.
       */
      Item(const Item &other);

      ~Item();

      /**
       * \brief Copy assignment operator.
       * \param[in] other The Item to assign from.
       * \return Reference to this Item.
       */
      Item &operator=(const Item &other);

      /**
       * \brief Get the IFileReader instance for this item.
       * \return Pointer to the reader, or \c nullptr if unavailable.
       */
      IFileReader *GetReader() const;

      /**
       * \brief Get the human-readable description of this reader.
       * \return The description string, or an empty string if not set.
       */
      std::string GetDescription() const;

      /**
       * \brief Get the confidence level of this reader for the given file.
       * \return The confidence level.
       */
      IFileReader::ConfidenceLevel GetConfidenceLevel() const;

      /**
       * \brief Get the mime-type associated with this reader item.
       * \return The MimeType for this reader.
       */
      MimeType GetMimeType() const;

      /**
       * \brief Get the service reference for this reader.
       * \return The CppMicroServices service reference.
       */
      us::ServiceReference<IFileReader> GetReference() const;

      /**
       * \brief Get the service id for this reader.
       * \return The unique service id.
       */
      long GetServiceId() const;

      /**
       * \brief Compare two items for sorting.
       * \param[in] other The Item to compare with.
       * \return \c true if this item ranks lower than \c other.
       *
       * Sorting order (ascending): confidence level, then mime-type rank,
       * then service ranking. The best match sorts last.
       */
      bool operator<(const Item &other) const;

    private:
      friend class FileReaderSelector;

      Item();

      struct Impl;
      us::SharedDataPointer<Impl> d;
    };

    /**
     * \brief Copy constructor.
     * \param[in] other The FileReaderSelector to copy.
     */
    FileReaderSelector(const FileReaderSelector &other);

    /**
     * \brief Construct a selector for the given file path.
     * \param[in] path The absolute file path to find readers for.
     *
     * Queries the service registry for all IFileReader services matching
     * the file's mime-type(s) and evaluates their confidence levels.
     * If the file does not exist, the selector will be empty.
     */
    FileReaderSelector(const std::string &path);

    ~FileReaderSelector();

    /**
     * \brief Copy assignment operator.
     * \param[in] other The FileReaderSelector to assign from.
     * \return Reference to this selector.
     */
    FileReaderSelector &operator=(const FileReaderSelector &other);

    /**
     * \brief Check whether any suitable readers were found.
     * \return \c true if no readers are available for the given file.
     */
    bool IsEmpty() const;

    /**
     * \brief Get all mime-types that matched the file path.
     * \return A vector of MimeType objects.
     */
    std::vector<MimeType> GetMimeTypes() const;

    /**
     * \brief Get a sorted list of all file reader items.
     *
     * Items are sorted in ascending order by:
     * -# Confidence level
     * -# MimeType ordering
     * -# File reader service ranking
     *
     * The best-matching item is at the back of the returned container.
     *
     * \return A sorted vector of Item objects.
     */
    std::vector<Item> Get() const;

    /**
     * \brief Get a reader item by its service id.
     * \param[in] id The service id to look up.
     * \return The matching Item, or a default-constructed Item if not found.
     */
    Item Get(long id) const;

    /**
     * \brief Get the best-matching (highest-ranked) reader item.
     * \return The default Item, or a default-constructed Item if the selector is empty.
     */
    Item GetDefault() const;

    /**
     * \brief Get the service id of the best-matching reader.
     * \return The service id, or -1 if the selector is empty.
     */
    long GetDefaultId() const;

    /**
     * \brief Get the currently selected reader item.
     * \return The selected Item, or a default-constructed Item if none is selected.
     */
    Item GetSelected() const;

    /**
     * \brief Get the service id of the currently selected reader.
     * \return The service id, or -1 if none is selected.
     */
    long GetSelectedId() const;

    /**
     * \brief Select a reader by Item.
     * \param[in] item The Item to select.
     * \return \c true if the selection was successful.
     */
    bool Select(const Item &item);

    /**
     * \brief Select a reader by its service id.
     * \param[in] id The service id of the reader to select.
     * \return \c true if a reader with the given id exists and was selected.
     */
    bool Select(long id);

    /**
     * \brief Swap the contents of this selector with another.
     * \param[in,out] fws The FileReaderSelector to swap with.
     */
    void Swap(FileReaderSelector &fws);

  private:
    struct Impl;
    us::ExplicitlySharedDataPointer<Impl> m_Data;
  };

  /**
   * \brief Swap two FileReaderSelector objects.
   * \param[in,out] fws1 First selector.
   * \param[in,out] fws2 Second selector.
   */
  void swap(FileReaderSelector &fws1, FileReaderSelector &fws2);
}

#endif
