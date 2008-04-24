/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#ifndef VOLUME_CALCULATOR_H_HEADER_INCLUDED
#define VOLUME_CALCULATOR_H_HEADER_INCLUDED

#include "itkObject.h"
#include "itkObjectFactory.h"
#include "itkImage.h"

#include "mitkCommon.h"
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

    itkNewMacro(Self);
    itkSetObjectMacro(Image,Image);
    itkSetMacro(Threshold,int);
    itkGetMacro(Volume,float);
    itkGetMacro(VoxelCount,unsigned long int);
   
    std::vector<float> GetVolumes();
    void ComputeVolume();

  protected:
    
    VolumeCalculator();
    virtual ~VolumeCalculator();

    template < typename TPixel, unsigned int VImageDimension >
    void InternalCompute(itk::Image< TPixel, VImageDimension >* itkImage);

    Image::ConstPointer        m_Image;
    int                        m_Threshold;
    float                      m_Volume;
    unsigned long int          m_VoxelCount;
    std::vector<float>         m_Volumes;
    ImageTimeSelector::Pointer m_TimeSelector;
  };

} // namespace mitk

#endif /* VOLUME_CALCULATOR_H_HEADER_INCLUDED */

