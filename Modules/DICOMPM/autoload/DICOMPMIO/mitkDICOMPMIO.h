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
   * \brief Read and write DICOM Parametric Map objects.
   * \ingroup Process
   */
  class DICOMPMIO : public mitk::AbstractFileIO
  {
  public:
    /** \brief Default constructor. Registers reader/writer for the DICOM PM MIME type. */
    DICOMPMIO();

    // -------------- AbstractFileReader -------------

    using AbstractFileReader::Read;

    /** \brief Return the confidence level for reading the given file as a parametric map. */
    ConfidenceLevel GetReaderConfidenceLevel() const override;

    // -------------- AbstractFileWriter -------------

    /** \brief Write the parametric map to a DICOM file. */
    void Write() override;

    /** \brief Return the confidence level for writing the given data as a DICOM parametric map. */
    ConfidenceLevel GetWriterConfidenceLevel() const override;

  protected:
    /**
     * \brief Read a DICOM parametric map from the file system.
     * \return A vector containing the loaded mitk::Image.
     * \throw mitk::Exception if an error occurs during reading.
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
