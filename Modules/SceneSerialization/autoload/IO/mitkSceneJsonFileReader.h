/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkSceneJsonFileReader_h
#define mitkSceneJsonFileReader_h

#include <mitkAbstractFileReader.h>

namespace mitk
{
  /**
   * \brief Reader service for MITK JSON scene files (.mitkscene.json).
   *
   * Loads a scene described in the JSON scene format and populates the
   * given DataStorage with the contained data nodes and their properties.
   * Internally delegates to SceneJsonReader.
   */
  class SceneJsonFileReader : public mitk::AbstractFileReader
  {
  public:
    /** \brief Default constructor. Registers the reader for the MITK JSON scene MIME type. */
    SceneJsonFileReader();

    using AbstractFileReader::Read;

    /**
     * \brief Read the JSON scene file and add all nodes to the given DataStorage.
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
    SceneJsonFileReader *Clone() const override;
  };
}

#endif
