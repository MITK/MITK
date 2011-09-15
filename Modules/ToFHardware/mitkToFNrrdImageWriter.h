/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date: 2010-05-27 16:06:53 +0200 (Do, 27 Mai 2010) $
Version:   $Revision:  $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __mitkToFNrrdImageWriter_h
#define __mitkToFNrrdImageWriter_h

#include "mitkToFHardwareExports.h"
#include "mitkToFImageWriter.h"

#include <fstream>

namespace mitk
{
  /**
  * @brief Writer class for ToF nrrd images
  * 
  * This writer class allows streaming of ToF data into a nrrd file. This class uses the itkNrrdImageIO class
  * Writer can simultaneously save "distance", "intensity" and "amplitude" image.
  * Images can be written as 3D volume (ToFImageType::ToFImageType3D) or temporal image stack (ToFImageType::ToFImageType2DPlusT)
  *
  * @ingroup ToFHardware
  */
  class MITK_TOFHARDWARE_EXPORT ToFNrrdImageWriter : public ToFImageWriter
  {
  public: 
    mitkClassMacro( ToFNrrdImageWriter , ToFImageWriter );
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

    std::ofstream m_DistanceOutfile; ///< file for distance image
    std::ofstream m_AmplitudeOutfile; ///< file for amplitude image
    std::ofstream m_IntensityOutfile; ///< file for intensity image

  private:

    ToFNrrdImageWriter();
    ~ToFNrrdImageWriter();

    /*!
    \brief Open file by filename to gain write access to it.
    */
    void OpenStreamFile(std::ofstream &outfile, std::string outfileName);
    /*!
    \brief Close file after work on it is finished.
    */
    void CloseStreamFile(std::ofstream &outfile, std::string fileName);
    /*!
    \brief Write image information to the NrrdFile.
    */
    void ConvertStreamToNrrdFormat( std::string fileName );
  };
} //END mitk namespace
#endif // __mitkToFNrrdImageWriter_h
