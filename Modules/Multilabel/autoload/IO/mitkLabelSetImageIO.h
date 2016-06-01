/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef __mitkLabelSetImageIO_h
#define __mitkLabelSetImageIO_h

#include <mitkAbstractFileIO.h>
#include <mitkLabelSetImage.h>

namespace mitk
{
  /**
  * Writes a LabelSetImage to a file
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
    /**
    * @brief Reads a number of mitk::LabelSetImages from the file system
    * @return a vector of mitk::LabelSetImages
    * @throws throws an mitk::Exception if an error ocurrs during parsing the nrrd header
    */
    virtual std::vector<BaseData::Pointer> Read() override;
    virtual ConfidenceLevel GetReaderConfidenceLevel() const override;

    // -------------- AbstractFileWriter -------------

    virtual void Write() override;
    virtual ConfidenceLevel GetWriterConfidenceLevel() const override;

    // -------------- LabelSetImageIO specific functions -------------

    int GetIntByKey(const itk::MetaDataDictionary &dic, const std::string &str);
    std::string GetStringByKey(const itk::MetaDataDictionary &dic, const std::string &str);

  private:

    LabelSetImageIO* IOClone() const override;
  };
} // end of namespace mitk

#endif // __mitkLabelSetImageIO_h
