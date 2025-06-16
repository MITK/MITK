/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkMultiLabelSegmentationStackWriterBase_h
#define mitkMultiLabelSegmentationStackWriterBase_h

#include <mitkAbstractFileWriter.h>
#include <mitkLabelSetImage.h>

namespace mitk
{
  /**
  * Writes a MultiLabelSegmentation to a file.
  * mitk::Identifiable UID is supported and will be serialized.
  * @ingroup Process
  */
  // The export macro should be removed. Currently, the unit
  // tests directly instantiate this class.
  class MultiLabelSegmentationStackWriterBase : public mitk::AbstractFileWriter
  {
  public:
    typedef mitk::MultiLabelSegmentation InputType;

    MultiLabelSegmentationStackWriterBase();

    // -------------- AbstractFileReader -------------

    using AbstractFileReader::Read;

    ConfidenceLevel GetReaderConfidenceLevel() const override;

    // -------------- AbstractFileWriter -------------

    void Write() override;
    ConfidenceLevel GetWriterConfidenceLevel() const override;

  protected:
    /**
    * @brief Reads a number of mitk::LabelSetImages from the file system
    * @return a vector of mitk::LabelSetImages
    * @throws throws an mitk::Exception if an error ocurrs during parsing the nrrd header
    */
    std::vector<itk::SmartPointer<BaseData>> DoRead() override;

    // Fills the m_DefaultMetaDataKeys vector with default values
    virtual void InitializeDefaultMetaDataKeys();

  private:
    MultiLabelSegmentationStackWriterBase *IOClone() const override;

    std::vector<std::string> m_DefaultMetaDataKeys;
  };
} // end of namespace mitk

#endif
