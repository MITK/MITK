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

#ifndef __mitkDICOMSegmentationIO_h
#define __mitkDICOMSegmentationIO_h

#include <mitkAbstractFileIO.h>
#include <mitkLabelSetImage.h>

namespace mitk
{
  /**
  * Writes a LabelSetImage to a dcm file
  * @ingroup Process
  */
  class DICOMSegmentationIO : public mitk::AbstractFileIO
  {
  public:
    typedef mitk::LabelSetImage InputType;
    typedef itk::Image<unsigned short, 3> itkInputImageType;
    typedef itk::Image<short, 3> itkInternalImageType;

    DICOMSegmentationIO();

    // -------------- AbstractFileReader -------------

    using AbstractFileReader::Read;
    /**
    * @brief Reads a number of DICOM segmentation from the file system
    * @return a vector of mitk::LabelSetImages
    * @throws throws an mitk::Exception if an error ocurrs
    */
    virtual std::vector<BaseData::Pointer> Read() override;
    virtual ConfidenceLevel GetReaderConfidenceLevel() const override;

    // -------------- AbstractFileWriter -------------

    virtual void Write() override;
    virtual ConfidenceLevel GetWriterConfidenceLevel() const override;

    // -------------- DICOMSegmentationIO specific functions -------------

    const std::string CreateMetaDataJsonFile(int layer);
    //const  std::map<itkInternalImageType::PixelType, itkInternalImageType::Pointer> GetImagesFromLayer(Image::Pointer layerImage);

  private:
    DICOMSegmentationIO *IOClone() const override;
  };
} // end of namespace mitk

#endif // __mitkDICOMSegmentationIO_h
