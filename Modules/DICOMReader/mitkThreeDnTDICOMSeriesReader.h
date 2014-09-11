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

#ifndef mitkThreeDnTDICOMSeriesReader_h
#define mitkThreeDnTDICOMSeriesReader_h

#include "mitkDICOMITKSeriesGDCMReader.h"

#include "MitkDICOMReaderExports.h"

namespace mitk
{

/**
   \ingroup DICOMReader
   \brief Extends DICOMITKSeriesGDCMReader by sorting/grouping into 3D+t image blocks.

   This class reuses the DICOMITKSeriesGDCMReader class and adds the option of
   grouping 3D blocks at the same spatial position into a single block, which
   is loaded as a 3D+t mitk::Image (compare to \ref DICOMITKSeriesGDCMReader_Condensing).

   To group two output blocks into a single 3D+t block, this class tests a number
   of requirements that the two blocks must fulfill:
    - the <b>origin of the first slice</b> must be identical
    - the <b>origin of the last slice</b> must be identical
    - the <b>number of slices</b> must be identical

   The output blocks described by DICOMImageBlockDescriptor will contains the following properties:
    - \b "3D+t": true if the image is 3D+t
    - \b "timesteps": number of timesteps of an image (only defined if "3D+t" is true)
*/
class MitkDICOMReader_EXPORT ThreeDnTDICOMSeriesReader : public DICOMITKSeriesGDCMReader
{
  public:

    mitkClassMacro( ThreeDnTDICOMSeriesReader, DICOMITKSeriesGDCMReader );
    mitkCloneMacro( ThreeDnTDICOMSeriesReader );
    itkNewMacro( ThreeDnTDICOMSeriesReader );
    mitkNewMacro1Param( ThreeDnTDICOMSeriesReader, unsigned int );

    /// \brief Control whether 3D+t grouping shall actually be attempted.
    void SetGroup3DandT(bool on);
    bool GetGroup3DandT() const;

    // void AllocateOutputImages();
    /// \brief Load via multiple calls to itk::ImageSeriesReader.
    virtual bool LoadImages();

    virtual bool operator==(const DICOMFileReader& other) const;

  protected:

    ThreeDnTDICOMSeriesReader(unsigned int decimalPlacesForOrientation = 5);
    virtual ~ThreeDnTDICOMSeriesReader();

    ThreeDnTDICOMSeriesReader(const ThreeDnTDICOMSeriesReader& other);
    ThreeDnTDICOMSeriesReader& operator=(const ThreeDnTDICOMSeriesReader& other);

    /**
      \brief Analyze the groups produced by DICOMITKSeriesGDCMReader for 3D+t properties.
      This method tests whether some blocks are at the same spatial position and groups
      them into single blocks.
    */
    virtual SortingBlockList Condense3DBlocks(SortingBlockList&);

    bool LoadMitkImageForImageBlockDescriptor(DICOMImageBlockDescriptor& block) const;

    bool m_Group3DandT;
};

}

#endif
