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

#ifndef __mitkDICOMPMIO_h
#define __mitkDICOMPMIO_h

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

    /**
     * @brief Reads a DICOM parametric maps from the file system
     * @return a  mitk::Image
     * @throws throws an mitk::Exception if an error ocurrs
     */
    virtual std::vector<BaseData::Pointer> Read() override;
    virtual ConfidenceLevel GetReaderConfidenceLevel() const override;

    // -------------- AbstractFileWriter -------------

    virtual void Write() override;
    virtual ConfidenceLevel GetWriterConfidenceLevel() const override;

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

#endif // __mitkDICOMPMIO_h
