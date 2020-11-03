/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkThreeDnTDICOMSeriesReader_h
#define mitkThreeDnTDICOMSeriesReader_h

#include "mitkDICOMITKSeriesGDCMReader.h"

#include "MitkDICOMExports.h"

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
class MITKDICOM_EXPORT ThreeDnTDICOMSeriesReader : public DICOMITKSeriesGDCMReader
{
  public:

    mitkClassMacro( ThreeDnTDICOMSeriesReader, DICOMITKSeriesGDCMReader );
    mitkCloneMacro( ThreeDnTDICOMSeriesReader );
    itkNewMacro( ThreeDnTDICOMSeriesReader );
    mitkNewMacro1Param( ThreeDnTDICOMSeriesReader, unsigned int );

    /// \brief Control whether 3D+t grouping shall actually be attempted.
    void SetGroup3DandT(bool on);
    bool GetGroup3DandT() const;

    itkBooleanMacro(OnlyCondenseSameSeries);
    itkSetMacro(OnlyCondenseSameSeries, bool);
    itkGetConstMacro(OnlyCondenseSameSeries, bool);

    // void AllocateOutputImages();
    /// \brief Load via multiple calls to itk::ImageSeriesReader.
    bool LoadImages() override;

    bool operator==(const DICOMFileReader& other) const override;

    static bool GetDefaultGroup3DandT()
    {
      return m_DefaultGroup3DandT;
    }
    static bool GetDefaultOnlyCondenseSameSeries()
    {
      return m_DefaultOnlyCondenseSameSeries;
    }

  protected:

    ThreeDnTDICOMSeriesReader(unsigned int decimalPlacesForOrientation = Superclass::m_DefaultDecimalPlacesForOrientation);
    ~ThreeDnTDICOMSeriesReader() override;

    ThreeDnTDICOMSeriesReader(const ThreeDnTDICOMSeriesReader& other);
    ThreeDnTDICOMSeriesReader& operator=(const ThreeDnTDICOMSeriesReader& other);

    /**
      \brief Analyze the groups produced by DICOMITKSeriesGDCMReader for 3D+t properties.
      This method tests whether some blocks are at the same spatial position and groups
      them into single blocks.
    */
    SortingBlockList Condense3DBlocks(SortingBlockList&) override;

    bool LoadMitkImageForImageBlockDescriptor(DICOMImageBlockDescriptor& block) const override;

    bool m_Group3DandT;
    bool m_OnlyCondenseSameSeries;

    const static bool m_DefaultGroup3DandT = true;
    const static bool m_DefaultOnlyCondenseSameSeries = true;
};

}

#endif
