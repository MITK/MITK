/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkROIIO_h
#define mitkROIIO_h

#include <mitkAbstractFileIO.h>

namespace mitk
{
  /**
   * \brief Read and write mitk::ROI objects to/from JSON files.
   */
  class ROIIO : public AbstractFileIO
  {
  public:
    /** \brief Default constructor. Registers reader/writer for the ROI MIME type. */
    ROIIO();

    using AbstractFileReader::Read;

    /** \brief Write the ROI to a JSON file. */
    void Write() override;

  protected:
    /**
     * \brief Perform the actual reading of ROI data from a JSON file.
     * \return A vector of loaded BaseData objects (ROI instances).
     */
    std::vector<BaseData::Pointer> DoRead() override;

  private:
    ROIIO* IOClone() const override;
  };
}

#endif
