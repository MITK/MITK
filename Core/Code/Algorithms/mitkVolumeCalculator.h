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

#ifndef VOLUME_CALCULATOR_H_HEADER_INCLUDED
#define VOLUME_CALCULATOR_H_HEADER_INCLUDED

#include "itkObject.h"
#include "itkObjectFactory.h"
#include "itkImage.h"

#include <MitkCoreExports.h>
#include "mitkImage.h"
#include "mitkImageTimeSelector.h"

namespace mitk
{
  /**
   * @brief Calculates the volume of a mitk::Image.
   * The given volume is in milliliters or as a voxel count.
   * Voxels are counted if their gray value is above a threshold (see SetThreshold), the default threshold is 0.
   *
   * The filter works for 2D, 3D and 3D+t. In the 3D+t case a vector of volumes is provided (see GetVolumes()).
   */
  class MITK_CORE_EXPORT VolumeCalculator : public itk::Object
  {
  public:
    mitkClassMacro(VolumeCalculator,itk::Object);

    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)
    itkSetObjectMacro(Image,Image);
    /**
    * Sets threshold, all voxels that are equal or greater are accepted.
    */
    itkSetMacro(Threshold,int);
    /**
    * In case of a 2D image this returns the calculated volume in square cm, in case of 3D images the result is given
    * in cubic cm. For 4D data refer to the GetVolumes() function.
    * @see GetVolumes()
    */
    itkGetMacro(Volume,float);
    /**
    * Returns the number of accepted voxels.
    */
    itkGetMacro(VoxelCount,unsigned long int);
    /**
    * Returns the volume for 4D images, as a vector where each volume is represented by an item in the vector (in cubic cm).
    \note Also for 2D+t images the result will be returned in cubic cm.
    */
    std::vector<float> GetVolumes();
    void ComputeVolume();

    void ComputeVolumeFromImageStatistics();
    static float ComputeVolume(Vector3D spacing, unsigned int voxelCount);

  protected:

    VolumeCalculator();
    virtual ~VolumeCalculator();

    template < typename TPixel, unsigned int VImageDimension >
    void InternalCompute(const itk::Image< TPixel, VImageDimension >* itkImage);

    Image::ConstPointer        m_Image;
    int                        m_Threshold;
    float                      m_Volume;
    unsigned long int          m_VoxelCount;
    std::vector<float>         m_Volumes;
    ImageTimeSelector::Pointer m_TimeSelector;
  };

} // namespace mitk

#endif /* VOLUME_CALCULATOR_H_HEADER_INCLUDED */

