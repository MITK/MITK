/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkFileWriterSelector_h
#define mitkFileWriterSelector_h

#include <mitkIFileWriter.h>

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
   * \brief Selects the most suitable IFileWriter for a given BaseData object.
   *
   * FileWriterSelector queries the service registry for all IFileWriter
   * services that can write the given BaseData instance, optionally
   * filtered by a destination mime-type or file path. It ranks the
   * matching writers by confidence level, BaseData class hierarchy index,
   * and service ranking, providing access to the best-matching (default)
   * writer as well as all candidates.
   *
   * \sa IFileWriter
   * \sa FileReaderSelector
   * \sa MimeType
   */
  class MITKCORE_EXPORT FileWriterSelector
  {
  public:
    /**
     * \brief Represents a single candidate file writer with associated metadata.
     *
     * Items are comparable and sorted by confidence level, BaseData type
     * specificity, and service ranking. The best-matching item sorts last
     * (highest).
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
       * \brief Get the IFileWriter instance for this item.
       * \return Pointer to the writer, or \c nullptr if unavailable.
       */
      IFileWriter *GetWriter() const;

      /**
       * \brief Get the human-readable description of this writer.
       * \return The description string, or an empty string if not set.
       */
      std::string GetDescription() const;

      /**
       * \brief Get the confidence level of this writer for the given data.
       * \return The confidence level.
       */
      IFileWriter::ConfidenceLevel GetConfidenceLevel() const;

      /**
       * \brief Get the mime-type associated with this writer item.
       * \return The MimeType for this writer.
       */
      MimeType GetMimeType() const;

      /**
       * \brief Get the BaseData type name this writer supports.
       * \return The class name of the supported BaseData sub-class.
       */
      std::string GetBaseDataType() const;

      /**
       * \brief Get the service reference for this writer.
       * \return The CppMicroServices service reference.
       */
      us::ServiceReference<IFileWriter> GetReference() const;

      /**
       * \brief Get the service id for this writer.
       * \return The unique service id.
       */
      long GetServiceId() const;

      /**
       * \brief Compare two items for sorting.
       * \param[in] other The Item to compare with.
       * \return \c true if this item ranks lower than \c other.
       *
       * Sorting order (ascending): confidence level, then BaseData class
       * hierarchy specificity, then service ranking. The best match sorts last.
       */
      bool operator<(const Item &other) const;

    private:
      friend class FileWriterSelector;

      Item();

      struct Impl;
      us::SharedDataPointer<Impl> d;
    };

    /**
     * \brief Copy constructor.
     * \param[in] other The FileWriterSelector to copy.
     */
    FileWriterSelector(const FileWriterSelector &other);

    /**
     * \brief Construct a selector for the given BaseData and optional mime-type/path.
     * \param[in] baseData The BaseData object to find writers for.
     * \param[in] destMimeType Optional mime-type name to restrict results.
     *            If empty and a path is given, the mime-type is inferred from the path.
     * \param[in] path Optional output file path to help infer the mime-type.
     *
     * Queries the service registry for all IFileWriter services matching
     * the given BaseData type and evaluates their confidence levels.
     */
    FileWriterSelector(const BaseData *baseData,
                       const std::string &destMimeType = std::string(),
                       const std::string &path = std::string());

    ~FileWriterSelector();

    /**
     * \brief Copy assignment operator.
     * \param[in] other The FileWriterSelector to assign from.
     * \return Reference to this selector.
     */
    FileWriterSelector &operator=(const FileWriterSelector &other);

    /**
     * \brief Check whether any suitable writers were found.
     * \return \c true if no writers are available for the given data.
     */
    bool IsEmpty() const;

    /**
     * \brief Get a sorted list of writer items filtered by mime-type.
     *
     * Items are sorted in ascending order by:
     * -# Confidence level
     * -# File writer service ranking
     *
     * The best-matching item is at the back of the returned container.
     * If \c mimeType is empty, all items are returned regardless of mime-type.
     *
     * \param[in] mimeType The mime-type name to filter by, or empty for all.
     * \return A sorted vector of Item objects.
     */
    std::vector<Item> Get(const std::string &mimeType) const;

    /**
     * \brief Get a sorted list of writer items matching the currently selected mime-type.
     * \return An ordered vector of writer items.
     */
    std::vector<Item> Get() const;

    /**
     * \brief Get a writer item by its service id.
     * \param[in] id The service id to look up.
     * \return The matching Item, or a default-constructed Item if not found.
     */
    Item Get(long id) const;

    /**
     * \brief Get the best-matching (highest-ranked) writer item.
     * \return The default Item, or a default-constructed Item if the selector is empty.
     */
    Item GetDefault() const;

    /**
     * \brief Get the service id of the best-matching writer.
     * \return The service id, or -1 if the selector is empty.
     */
    long GetDefaultId() const;

    /**
     * \brief Get the currently selected writer item.
     * \return The selected Item, or a default-constructed Item if none is selected.
     */
    Item GetSelected() const;

    /**
     * \brief Get the service id of the currently selected writer.
     * \return The service id, or -1 if none is selected.
     */
    long GetSelectedId() const;

    /**
     * \brief Select the best writer for the given mime-type.
     * \param[in] mimeType The mime-type name to select by.
     * \return \c true if at least one writer with the given mime-type exists and was selected.
     */
    bool Select(const std::string &mimeType);

    /**
     * \brief Select a writer by Item.
     * \param[in] item The Item to select.
     * \return \c true if the selection was successful.
     */
    bool Select(const Item &item);

    /**
     * \brief Select a writer by its service id.
     * \param[in] id The service id of the writer to select.
     * \return \c true if a writer with the given id exists and was selected.
     */
    bool Select(long id);

    /**
     * \brief Get all mime-types for which at least one writer was found.
     * \return A vector of MimeType objects.
     */
    std::vector<MimeType> GetMimeTypes() const;

    /**
     * \brief Swap the contents of this selector with another.
     * \param[in,out] fws The FileWriterSelector to swap with.
     */
    void Swap(FileWriterSelector &fws);

  private:
    struct Impl;
    us::ExplicitlySharedDataPointer<Impl> m_Data;
  };

  /**
   * \brief Swap two FileWriterSelector objects.
   * \param[in,out] fws1 First selector.
   * \param[in,out] fws2 Second selector.
   */
  void swap(FileWriterSelector &fws1, FileWriterSelector &fws2);
}

#endif
