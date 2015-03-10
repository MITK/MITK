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


#ifndef __mitkDicomDiffusionImageHeaderReader_h__
#define __mitkDicomDiffusionImageHeaderReader_h__

#include <MitkDiffusionCoreExports.h>

#include <mitkFileReader.h>
#include "itkImageSeriesReader.h"
#include "itkImageFileReader.h"
#include "itkImage.h"
#include "itkGDCMImageIO.h"
#include "mitkDiffusionImageHeaderInformation.h"

namespace mitk
{
  /**
    @brief Reads the header information from a DICOM series and stores it into an output object of type
    DiffusionImageHeaderInformation
    */
  class MITKDIFFUSIONCORE_EXPORT DicomDiffusionImageHeaderReader : public itk::Object
  {
  public:

    mitkClassMacro( DicomDiffusionImageHeaderReader, itk::Object );
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    /** En enum to distinguish in processing between the different vendoors */
    enum SupportedVendors{
      SV_SIEMENS,
      SV_SIEMENS_MOSAIC,
      SV_PHILIPS,
      SV_GE,
      SV_UNKNOWN_VENDOR
    };

    typedef std::vector< std::string > FileNamesContainer;

    typedef short PixelValueType;
    typedef itk::Image< PixelValueType, 2 > SliceType;
    typedef itk::Image< PixelValueType, 3 > VolumeType;
    typedef itk::ImageSeriesReader< VolumeType > VolumeReaderType;
    typedef itk::ImageFileReader< SliceType > SliceReaderType;
    typedef itk::GDCMImageIO ImageIOType;

    /** Set the dicom file names to be considered */
    void SetSeriesDicomFilenames(FileNamesContainer dicomFilenames)
    { this->m_DicomFilenames = dicomFilenames; }

    void SetGdcmIO(ImageIOType::Pointer gdcmIO)
    { this->m_GdcmIO = gdcmIO; }

    /** Set the volume reader in case the default volume reader shouldn't be used */
    void SetVolumeReader(VolumeReaderType::Pointer volumeReader)
    { this->m_VolumeReader = volumeReader; }

    /** Set the output object that will contain the read-in information after update */
    void SetOutputPointer(DiffusionImageHeaderInformation::Pointer output)
    { this->m_Output = output; }

    /** do the work */
    virtual void Update();

    /** return output */
    DiffusionImageHeaderInformation::Pointer GetOutput();

    /** identify vendor */
    SupportedVendors GetVendorID();

  protected:
    /** Default c'tor */
    DicomDiffusionImageHeaderReader();

    /** Default d'tor */
    virtual ~DicomDiffusionImageHeaderReader();

    void ReadPublicTags();
    void ReadPublicTags2();
    void TransformGradients();

    FileNamesContainer m_DicomFilenames;
    DiffusionImageHeaderInformation::Pointer m_Output;

    ImageIOType::Pointer m_GdcmIO;
    VolumeReaderType::Pointer m_VolumeReader;

    bool m_SliceOrderIS;
    bool m_SingleSeries;
    int m_nSlice;
    std::vector<float> m_sliceLocations;

  };

}

#endif



