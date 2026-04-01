/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkSegmentationTaskListIO_h
#define mitkSegmentationTaskListIO_h

#include <mitkAbstractFileIO.h>
#include <mitkFileSystem.h>

namespace mitk
{
  class SegmentationTaskList;

  /**
   * \brief File I/O for reading and writing SegmentationTaskList objects.
   *
   * Handles serialization and deserialization of segmentation task lists
   * in JSON format.
   *
   * \sa SegmentationTaskList, MitkSegmentationIOMimeTypes
   */
  class SegmentationTaskListIO : public AbstractFileIO
  {
  public:
    /** \brief Default constructor. Registers reader/writer for the SegmentationTaskList MIME type. */
    SegmentationTaskListIO();

    using AbstractFileReader::Read;

    /** \brief Write the SegmentationTaskList to a JSON file. */
    void Write() override;

  protected:
    /**
     * \brief Perform the actual reading of a SegmentationTaskList from a JSON file.
     * \return A vector of loaded BaseData objects (SegmentationTaskLists).
     */
    std::vector<BaseData::Pointer> DoRead() override;

    /**
     * \brief Determine the minimum file format version required for the given task list.
     * \param segmentationTaskList The task list to inspect.
     * \return The minimum version number.
     */
    int GetMinimumRequiredVersion(const SegmentationTaskList* segmentationTaskList) const;

    /**
     * \brief Resolve a relative path against the task list file's directory.
     * \param path The path to resolve.
     * \return The resolved absolute path.
     */
    fs::path ResolvePath(const fs::path& path) const;

  private:
    SegmentationTaskListIO* IOClone() const override;
  };
}

#endif
