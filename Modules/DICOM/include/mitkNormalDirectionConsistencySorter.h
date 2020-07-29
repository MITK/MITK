/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkNormalDirectionConsistencySorter_h
#define mitkNormalDirectionConsistencySorter_h

#include "mitkDICOMDatasetSorter.h"
#include "mitkGantryTiltInformation.h"

namespace mitk
{

/**
 \ingroup DICOMModule
 \brief Makes sure that the order of files is along the image plane normals.

 When loading with ImageSeriesReader and initializing an mitk::Image with the result
 we need to make sure that the order of inputs for the ImageSeriesReader is along the
 normal of the images. I.e. The direction of the normal needs to be the same direction
 as the vector from the first to the last image origin.

 Since this class is used as a last sorting step before loading, it will also
 calculate (and return) an updated GantryTiltInformation object.

 \note This class might be a workaround for another bug in MITK, but until this issue
       is completely understood, the workaround fixes the problem of images that
       appear upside-down.
*/
class MITKDICOM_EXPORT NormalDirectionConsistencySorter : public DICOMDatasetSorter
{
  public:

    mitkClassMacro( NormalDirectionConsistencySorter, DICOMDatasetSorter );

    itkNewMacro( NormalDirectionConsistencySorter );

    DICOMTagList GetTagsOfInterest() override;

    /// See class description.
    void Sort() override;

    /// See class description and DICOMITKSeriesGDCMReader.
    GantryTiltInformation GetTiltInformation() const;

    void PrintConfiguration(std::ostream& os, const std::string& indent = "") const override;

    bool operator==(const DICOMDatasetSorter& other) const override;

  protected:

    NormalDirectionConsistencySorter();
    ~NormalDirectionConsistencySorter() override;

    NormalDirectionConsistencySorter(const NormalDirectionConsistencySorter& other);
    NormalDirectionConsistencySorter& operator=(const NormalDirectionConsistencySorter& other);

    GantryTiltInformation m_TiltInfo;
};

}

#endif
