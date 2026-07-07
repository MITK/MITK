/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkMultiLabelSegmentationStackReader_h
#define mitkMultiLabelSegmentationStackReader_h

#include <mitkAbstractFileReader.h>
#include <mitkLabelSetImage.h>

namespace mitk
{
  /**
   * \brief Read a MultiLabelSegmentation from a mitklabel.json file and its associated image files.
   *
   * mitk::Identifiable UID is supported and will be deserialized.
   *
   * \ingroup Process
   */
  class MultiLabelSegmentationStackReader : public mitk::AbstractFileReader
  {
  public:
    typedef mitk::MultiLabelSegmentation InputType;

    /** \brief Default constructor. Registers reader for the multi-label meta MIME type. */
    MultiLabelSegmentationStackReader();
    ~MultiLabelSegmentationStackReader() = default;

    /** \brief Return the confidence level for reading the given file as a multi-label stack. */
    ConfidenceLevel GetConfidenceLevel() const override;

  protected:
    /**
     * \brief Perform the actual reading of the multi-label stack.
     * \return A vector of loaded BaseData objects (MultiLabelSegmentations).
     */
    std::vector<itk::SmartPointer<BaseData>> DoRead() override;


  private:
    MultiLabelSegmentationStackReader* Clone() const override;
  };
} // end of namespace mitk

#endif
