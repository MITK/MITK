/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef __mitkLabelSetImageIO_h
#define __mitkLabelSetImageIO_h

#include <mitkAbstractFileIO.h>
#include <mitkLabelSetImage.h>

namespace mitk
{
  /**
  * Writes a LabelSetImage to a file.
  * mitk::Identifiable UID is supported and will be serialized.
  * @ingroup Process
  */
  // The export macro should be removed. Currently, the unit
  // tests directly instantiate this class.
  class LabelSetImageIO : public mitk::AbstractFileIO
  {
  public:
    typedef mitk::LabelSetImage InputType;

    LabelSetImageIO();

    // -------------- AbstractFileReader -------------

    using AbstractFileReader::Read;

    ConfidenceLevel GetReaderConfidenceLevel() const override;

    // -------------- AbstractFileWriter -------------

    void Write() override;
    ConfidenceLevel GetWriterConfidenceLevel() const override;

    // -------------- LabelSetImageIO specific functions -------------

    int GetIntByKey(const itk::MetaDataDictionary &dic, const std::string &str);
    std::string GetStringByKey(const itk::MetaDataDictionary &dic, const std::string &str);

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
    LabelSetImageIO *IOClone() const override;

    std::vector<std::string> m_DefaultMetaDataKeys;
  };
} // end of namespace mitk

#endif // __mitkLabelSetImageIO_h
