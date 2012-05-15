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
#ifndef __mitkToFPicImageWriter_h
#define __mitkToFPicImageWriter_h

#include "mitkToFHardwareExports.h"
#include "mitkToFImageWriter.h"

#include <mitkIpPic.h>

namespace mitk
{
  /**
  * @brief Writer class for ToF images
  *
  * This writer class allows streaming of ToF data into a file. The .pic file format is used for writing the data.
  * Image information is included in the header of the pic file.
  * Writer can simultaneously save "distance", "intensity" and "amplitude" image.
  * Images can be written as 3D volume (ToFImageType::ToFImageType3D) or temporal image stack (ToFImageType::ToFImageType2DPlusT)
  *
  * @ingroup ToFHardware
  */
  class MITK_TOFHARDWARE_EXPORT ToFPicImageWriter : public ToFImageWriter
  {
  public: 

    ToFPicImageWriter();

    ~ToFPicImageWriter();

    mitkClassMacro( ToFPicImageWriter , ToFImageWriter );
    itkNewMacro( Self );

    /*!
    \brief Open file(s) for writing
    */
    void Open();
    /*!
    \brief Close file(s) add .pic header and write
    */
    void Close();
    /*!
    \brief Add new data to file.
    */
    void Add(float* distanceFloatData, float* amplitudeFloatData, float* intensityFloatData);

  protected:
    
    Image::Pointer m_MitkImage; ///< mitk image used for pic header creation
    FILE* m_DistanceOutfile; ///< file for distance image
    FILE* m_AmplitudeOutfile; ///< file for amplitude image
    FILE* m_IntensityOutfile; ///< file for intensity image


  private:

    /*!
    \brief Open file by filename to gain write access to it.
    */
    void OpenPicFile(FILE** outfile, std::string outfileName);
    /*!
    \brief Close file after work on it is finished.
    */
    void ClosePicFile(FILE* outfile);
    /*!
    \brief Replace current PicFileHeader information.
    */
    void ReplacePicFileHeader(FILE* outfile);
    /*!
    \brief Write image information to the PicFileHeader.
    */
    void WritePicFileHeader(FILE* outfile, mitkIpPicDescriptor* pic);

  };
} //END mitk namespace
#endif // __mitkToFPicImageWriter_h
