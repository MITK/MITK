/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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

#ifndef MITK_CALCULATE_SEGMENTATION_VOLUME_H_INCLUDET_WAD
#define MITK_CALCULATE_SEGMENTATION_VOLUME_H_INCLUDET_WAD

#include "mitkSegmentationSink.h"
#include "mitkImageCast.h"

namespace mitk 
{

class CalculateSegmentationVolume : public SegmentationSink
{
  public:
    
    mitkClassMacro( CalculateSegmentationVolume, SegmentationSink )
    mitkAlgorithmNewMacro( CalculateSegmentationVolume );
    
  protected:
    
    CalculateSegmentationVolume();  // use smart pointers
    virtual ~CalculateSegmentationVolume();

    virtual bool ReadyToRun();

    virtual bool ThreadedUpdateFunction(); // will be called from a thread after calling StartAlgorithm
    
    template < typename TPixel, unsigned int VImageDimension >
    void ItkImageProcessing( itk::Image< TPixel, VImageDimension >* itkImage, TPixel* dummy = NULL );

  private:

    unsigned int m_Volume;

    Vector3D m_CenterOfMass;
    Vector3D m_MinIndexOfBoundingBox;
    Vector3D m_MaxIndexOfBoundingBox;

};

} // namespace

#endif

