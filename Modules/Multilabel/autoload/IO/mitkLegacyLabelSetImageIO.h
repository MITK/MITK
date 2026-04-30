/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkLegacyLabelSetImageIO_h
#define mitkLegacyLabelSetImageIO_h

#include <mitkAbstractFileReader.h>
#include <mitkLabelSetImage.h>

namespace mitk
{
  /**
   * \brief Reader for legacy LabelSetImage files (older NRRD-based format).
   *
   * Reads a MultiLabelSegmentation from a legacy format file.
   * mitk::Identifiable UID is supported and will be deserialized.
   *
   * \ingroup Process
   */
  class LegacyLabelSetImageIO : public mitk::AbstractFileReader
  {
  public:
    typedef mitk::MultiLabelSegmentation InputType;

    /** \brief Default constructor. Registers reader for the legacy LabelSetImage MIME type. */
    LegacyLabelSetImageIO();

    // -------------- AbstractFileReader -------------

    using AbstractFileReader::Read;

    /** \brief Return the confidence level for reading the given file as a legacy LabelSetImage. */
    ConfidenceLevel GetConfidenceLevel() const override;

  protected:
    /**
     * \brief Read legacy LabelSetImage data from the file system.
     * \return A vector of mitk::LabelSetImages.
     * \throw mitk::Exception if an error occurs during NRRD header parsing.
     */
    std::vector<itk::SmartPointer<BaseData>> DoRead() override;

    // Fills the m_DefaultMetaDataKeys vector with default values
    virtual void InitializeDefaultMetaDataKeys();

  private:
    LegacyLabelSetImageIO *Clone() const override;

    std::vector<std::string> m_DefaultMetaDataKeys;
  };
} // end of namespace mitk

#endif
