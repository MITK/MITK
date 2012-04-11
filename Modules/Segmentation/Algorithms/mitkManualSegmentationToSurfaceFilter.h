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

#ifndef _MITKMANUALSEGMENTATIONTISURFACEFILTER_h__
#define _MITKMANUALSEGMENTATIONTISURFACEFILTER_h__

#include <mitkImageToSurfaceFilter.h>
#include "MitkExtExports.h"

#include <vtkImageGaussianSmooth.h>
#include <vtkImageMedian3D.h>
#include <vtkImageResample.h>
#include <vtkImageThreshold.h>


namespace mitk {
  /**
   * @brief Supplies a 3D surface from pre-processed segmentation. 
   *
   * The resulting surface depends on a filter pipeline based on vtkMedian (1) and a Gaussian filter with vtkImageGaussianSmooth (2).
   * All voxel can be changed to an isotropic representation of the
   * image (ATTANTION: the number of voxels in the will change). The
   * resulting isotropic image has 1mm isotropic voxel by default. But 
   * can be varied freely.
   *
   * @ingroup ImageFilters
   * @ingroup Process
   */

  class MitkExt_EXPORT ManualSegmentationToSurfaceFilter : public ImageToSurfaceFilter
  {
    public:
      mitkClassMacro(ManualSegmentationToSurfaceFilter,ImageToSurfaceFilter);

      typedef double vtkDouble;

      /**
       * Will pre-process a segmentation voxelwise. The segmentation can use
       * a hole fill relating a median filter and smooth by a Gaussian
       * filter. 
       * The image can be interpolated to an isotropic image.
       * By default every filter is disabled.
       * This method calls CreateSurface from mitkImageToSurfaceFilter and
       * does not need a manual call since we use Update().
       */
      virtual void GenerateData();
      itkNewMacro(Self);


      /**
       * Supplies a method for setting median filter by a bool value.
       */
      itkSetMacro(MedianFilter3D,bool);
      
      /**
       * Return state if median filter is enabled.
       */
      itkGetConstMacro(MedianFilter3D,bool);
      
      /**
       * Enable the median filter (first filter in pipeline).
       */
      itkBooleanMacro(MedianFilter3D);

      /**
       * Supplies a method to enable Interpolation.
       */
      itkSetMacro(Interpolation,bool);
      
      /**
       * Returns activation state of interpolation filter.
       */
      itkGetConstMacro(Interpolation,bool);
      
      /**
       * Enable the interpolation filter (second filter in pipeline) for
       * isotropic voxel.
       */
      itkBooleanMacro(Interpolation);

      /**
       * Supplies a method for Gaussian filter (third filter in pipeline). 
       */
      itkSetMacro(UseGaussianImageSmooth,bool);
      
      /**
       * Returns activation state of standard deviation filter.
       */
      itkGetConstMacro(UseGaussianImageSmooth,bool);
      
      /**
       *  Enables Gaussian image smooth. As well the threshold for the
       *  CreateSurface() method will raise the threshold to 49 and changes 
       *  the image range set  from 0 to 100. It is made for reasons in 
       *  binary images to prevent conflicts with the used filter. There are 
       *  better results for dividing fore- and background.
       */
      itkBooleanMacro(UseGaussianImageSmooth);

      /**
       * Set standard deviation for Gaussian Filter.
       * @param _arg by default 1.5
       */
      itkSetMacro(GaussianStandardDeviation, double);
      
      /**
       * Returns the standard deviation of the Gaussian filter which will be
       * used when filter is enabled.
       */
      itkGetConstMacro(GaussianStandardDeviation, double);

      /**
       * Set the Kernel for Median3DFilter. By default kernel is set to 3x3x3. 
       * If you choose '1' nothing will be processed in this direction.
       */
      void SetMedianKernelSize(int x, int y, int z);
      
      /**
       * Returns the kernel size in the first direction.
       */
      itkGetConstMacro(MedianKernelSizeX, int);
      
      /**
       * Returns the kernel size in the second direction.
       */
      itkGetConstMacro(MedianKernelSizeY, int);
      
      /**
       * Returns the kernel size in the third direction.
       */
      itkGetConstMacro(MedianKernelSizeZ, int);

      /**
       * Set the values for Spacing in X, Y and Z-Dimension
       */
      void SetInterpolation(vtkDouble x, vtkDouble y, vtkDouble z);


    protected:
      ManualSegmentationToSurfaceFilter();
      virtual ~ManualSegmentationToSurfaceFilter();

      bool m_MedianFilter3D;
      int m_MedianKernelSizeX, m_MedianKernelSizeY, m_MedianKernelSizeZ;
      bool m_UseGaussianImageSmooth; //Gaussian Filter
      double m_GaussianStandardDeviation;
      bool m_Interpolation;

      vtkDouble m_InterpolationX;
      vtkDouble m_InterpolationY;
      vtkDouble m_InterpolationZ;

  };//namespace

}
#endif //_MITKMANUALSEGMENTATIONTISURFACEFILTER_h__
