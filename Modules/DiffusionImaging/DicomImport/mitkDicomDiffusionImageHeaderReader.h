/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2008-02-08 13:23:19 +0100 (Fr, 08 Feb 2008) $
Version:   $Revision: 13561 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef __mitkDicomDiffusionImageHeaderReader_h__
#define __mitkDicomDiffusionImageHeaderReader_h__

#include "MitkDiffusionImagingExports.h"

#include <mitkFileReader.h>
#include "itkImageSeriesReader.h"
#include "itkImageFileReader.h"
#include "itkOrientedImage.h"
#include "itkGDCMImageIO.h"
#include "mitkDiffusionImageHeaderInformation.h"

namespace mitk
{

  class MitkDiffusionImaging_EXPORT DicomDiffusionImageHeaderReader : public itk::Object
  {
  public:

    mitkClassMacro( DicomDiffusionImageHeaderReader, itk::Object );
    itkNewMacro(Self);

    enum SupportedVendors{ 
      SV_SIEMENS,
      SV_SIEMENS_MOSAIC,
      SV_PHILIPS,
      SV_GE,
      SV_UNKNOWN_VENDOR
    }; 

    typedef std::vector< std::string > FileNamesContainer;

    typedef short PixelValueType;
    typedef itk::OrientedImage< PixelValueType, 2 > SliceType;
    typedef itk::OrientedImage< PixelValueType, 3 > VolumeType;
    typedef itk::ImageSeriesReader< VolumeType > VolumeReaderType;
    typedef itk::ImageFileReader< SliceType > SliceReaderType;
    typedef itk::GDCMImageIO ImageIOType;

    // set input
    void SetSeriesDicomFilenames(FileNamesContainer dicomFilenames)
    { this->m_DicomFilenames = dicomFilenames; }

    void SetGdcmIO(ImageIOType::Pointer gdcmIO)
    { this->m_GdcmIO = gdcmIO; }

    void SetVolumeReader(VolumeReaderType::Pointer volumeReader)
    { this->m_VolumeReader = volumeReader; }

    void SetOutputPointer(DiffusionImageHeaderInformation::Pointer output)
    { this->m_Output = output; }

    // do the work
    virtual void Update();

    // return output
    DiffusionImageHeaderInformation::Pointer GetOutput();

    // identify vendor
    SupportedVendors GetVendorID();

    //virtual const char* GetFileName() const {return 0;}
    //virtual void SetFileName(const char* aFileName) {}
    //virtual const char* GetFilePrefix() const {return 0;}
    //virtual void SetFilePrefix(const char* aFilePrefix) {}
    //virtual const char* GetFilePattern() const {return 0;}
    //virtual void SetFilePattern(const char* aFilePattern) {}

  protected:

    DicomDiffusionImageHeaderReader();
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



