/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: 13922 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#ifndef mitkShapeBasedInterpolationAlgorithm_h_Included
#define mitkShapeBasedInterpolationAlgorithm_h_Included

#include "mitkSegmentationInterpolationAlgorithm.h"

namespace mitk
{

/**
 * \brief Shape-based binary image interpolation.
 *
 * This class uses legacy code from ipSegmentation to implement 
 * the shape-based interpolation algorithm described in
 *
 * G.T. Herman, J. Zheng, C.A. Bucholtz: "Shape-based interpolation"
 * IEEE Computer Graphics & Applications, pp. 69-79,May 1992
 *
 *  Last contributor:
 *  $Author:$
 */
class MITK_CORE_EXPORT ShapeBasedInterpolationAlgorithm : public SegmentationInterpolationAlgorithm
{
  public:
    
    mitkClassMacro(ShapeBasedInterpolationAlgorithm, SegmentationInterpolationAlgorithm);
    itkNewMacro(ShapeBasedInterpolationAlgorithm);

    Image::Pointer Interpolate(Image::ConstPointer lowerSlice, unsigned int lowerSliceIndex,
                               Image::ConstPointer upperSlice, unsigned int upperSliceIndex,
                               unsigned int requestedIndex,
                               unsigned int sliceDimension,
                               Image::Pointer resultImage,
                               unsigned int timeStep,
                               Image::ConstPointer referenceImage);
};

} // namespace

#endif


