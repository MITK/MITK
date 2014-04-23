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

#ifndef mitkNormalDirectionConsistencySorter_h
#define mitkNormalDirectionConsistencySorter_h

#include "mitkDICOMDatasetSorter.h"
#include "mitkGantryTiltInformation.h"

namespace mitk
{

/**
 \ingroup DICOMReaderModule
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
class MitkDICOMReader_EXPORT NormalDirectionConsistencySorter : public DICOMDatasetSorter
{
  public:

    mitkClassMacro( NormalDirectionConsistencySorter, DICOMDatasetSorter )
    itkNewMacro( NormalDirectionConsistencySorter )

    virtual DICOMTagList GetTagsOfInterest();

    /// See class description.
    virtual void Sort();

    /// See class description and DICOMITKSeriesGDCMReader.
    GantryTiltInformation GetTiltInformation() const;

    virtual void PrintConfiguration(std::ostream& os, const std::string& indent = "") const;

    virtual bool operator==(const DICOMDatasetSorter& other) const;

  protected:

    NormalDirectionConsistencySorter();
    virtual ~NormalDirectionConsistencySorter();

    NormalDirectionConsistencySorter(const NormalDirectionConsistencySorter& other);
    NormalDirectionConsistencySorter& operator=(const NormalDirectionConsistencySorter& other);

    GantryTiltInformation m_TiltInfo;
};

}

#endif
