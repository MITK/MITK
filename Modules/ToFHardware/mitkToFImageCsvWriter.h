/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef mitkToFImageCsvWriter_h
#define mitkToFImageCsvWriter_h

#include "mitkToFImageWriter.h"
#include <MitkToFHardwareExports.h>

namespace mitk
{
  /**
  * @brief CSV writer class for ToF image data
  *
  * This writer class allows streaming of ToF data into a CSV file.
  * Writer can simultaneously save "distance", "intensity" and "amplitude" image data.
  * Output files are written as 1D CSV data stream.
  *
  * @ingroup ToFHardware
  */
  class MITKTOFHARDWARE_EXPORT ToFImageCsvWriter : public ToFImageWriter
  {
  public:
    /*!
    \brief standard ctor
    */
    ToFImageCsvWriter();
    /*!
    \brief standard ~ctor
    */
    ~ToFImageCsvWriter() override;

    mitkClassMacro( ToFImageCsvWriter , ToFImageWriter );

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self)
    /*!
    \brief Checks for file extensions and opens the output files
    */
    void Open() override;
    /*!
    \brief Closes the output files
    */
    void Close() override;
    /*!
    \brief Pushes the image data to the output files
    \param distanceFloatData from distance image as float value
    \param amplitudeFloatData from amplitude image as float value
    \param intensityFloatData from intensity image as float value
    \param rgbData
    */
    void Add(float* distanceFloatData, float* amplitudeFloatData, float* intensityFloatData, unsigned char* rgbData=nullptr) override;

  protected:

    Image::Pointer m_MitkImage; ///< mitk image used for pic header creation
    FILE* m_DistanceOutfile; ///< file for distance image
    FILE* m_AmplitudeOutfile; ///< file for amplitude image
    FILE* m_IntensityOutfile; ///< file for intensity image

  private:
    /*!
    \brief opens CSV output file
    \param output file, name of the output file
    */
    void OpenCsvFile(FILE** outfile, std::string outfileName);
    /*!
    \brief closes CSV output file
    \param output file
    */
    void CloseCsvFile(FILE* outfile);
    /*!
    \brief writes the data to the CSV output file
    \param output file, data array of float values
    */
    void WriteCsvFile(FILE* outfile, float* floatData);
  };
} //END mitk namespace
#endif
