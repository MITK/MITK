/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkSceneFileReader_h
#define mitkSceneFileReader_h

// MITK
#include <mitkAbstractFileReader.h>

namespace mitk
{
  /**
   * \brief Reader service for MITK scene files (.mitk).
   *
   * Loads a scene archive and populates the DataStorage with the contained
   * data nodes and their properties.
   */
  class SceneFileReader : public mitk::AbstractFileReader
  {
  public:
    /** \brief Default constructor. Registers reader for the MITK scene MIME type. */
    SceneFileReader();

    using AbstractFileReader::Read;

    /**
     * \brief Read the scene file and add all nodes to the given DataStorage.
     * \param ds The DataStorage to populate.
     * \return The set of DataNode objects that were added.
     */
    DataStorage::SetOfObjects::Pointer Read(DataStorage &ds) override;

  protected:
    /**
     * \brief Perform the actual reading of scene file data.
     * \return A vector of loaded BaseData objects.
     */
    std::vector<itk::SmartPointer<BaseData>> DoRead() override;

  private:
    SceneFileReader *Clone() const override;
  };

} // namespace mitk

#endif
