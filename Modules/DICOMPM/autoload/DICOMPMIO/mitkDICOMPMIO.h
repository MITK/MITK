/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkDICOMPMIO_h
#define mitkDICOMPMIO_h

#include <mitkAbstractFileIO.h>
#include <mitkImage.h>
#include <dcmtk/dcmpmap/dpmparametricmapiod.h>
#include <dcmqi/JSONSegmentationMetaInformationHandler.h>
#include <dcmqi/JSONParametricMapMetaInformationHandler.h>


namespace mitk
{
  /**
   * Read and Writes a Parametric map to a dcm file
   * @ingroup Process
   */
  class DICOMPMIO : public mitk::AbstractFileIO
  {
  public:


    DICOMPMIO();

    // -------------- AbstractFileReader -------------

    using AbstractFileReader::Read;

    ConfidenceLevel GetReaderConfidenceLevel() const override;

    // -------------- AbstractFileWriter -------------

    void Write() override;
    ConfidenceLevel GetWriterConfidenceLevel() const override;
  protected:
    /**
     * @brief Reads a DICOM parametric map from the file system
     * @return an mitk::Image
     * @throws an mitk::Exception if an error ocurrs
     */
    std::vector<itk::SmartPointer<BaseData>> DoRead() override;

  private:

    typedef mitk::Image PMInputType;
    typedef itk::Image<double, 3> PMitkInputImageType;
    typedef IODFloatingPointImagePixelModule::value_type PMFloatPixelType; // input type required for DCMQI
    typedef itk::Image<PMFloatPixelType, 3> PMitkInternalImageType;
    DICOMPMIO *IOClone() const override;

    // -------------- DICOMPMIO specific functions -------------

	const std::string CreateMetaDataJsonFilePM() const;

  };
} // end of namespace mitk

#endif
