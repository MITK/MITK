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

#ifndef __mitkLabelSetImageWriter_h
#define __mitkLabelSetImageWriter_h

#include "MitkMultilabelExports.h"
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
class MITKMULTILABEL_EXPORT LabelSetImageIO : public mitk::AbstractFileIO
{
public:

  typedef LabelSetImage::PixelType            PixelType;
  typedef LabelSetImage::LabelSetImageType    ImageType;
  typedef LabelSetImage::VectorImageType      VectorImageType;

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

  static bool SaveLabelSetImagePreset(const std::string & presetFilename, mitk::LabelSetImage::Pointer & inputImage);
  int GetIntByKey(const itk::MetaDataDictionary &dic, const std::string &str);
  std::string GetStringByKey(const itk::MetaDataDictionary &dic, const std::string &str);
  static Label::Pointer LoadLabelFromTiXmlDocument(TiXmlElement *labelElem);
  static void LoadLabelSetImagePreset(const std::string &presetFilename, mitk::LabelSetImage::Pointer &inputImage);

private:

  LabelSetImageIO* IOClone() const override;

  static TiXmlElement *GetLabelAsTiXmlElement(Label *label);

  static TiXmlElement * PropertyToXmlElem( const std::string& key, const BaseProperty* property );
  static bool PropertyFromXmlElem(std::string& key, mitk::BaseProperty::Pointer& prop, TiXmlElement* elem);
};

} // end of namespace mitk

#endif // __mitkLabelSetImageWriter_h
