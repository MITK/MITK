#ifndef _MITKMANUALSEGMENTATIONTISURFACEFILTER_h__
#define _MITKMANUALSEGMENTATIONTISURFACEFILTER_h__

#include <mitkImageToSurfaceFilter.h>

#include <vtkImageGaussianSmooth.h>
#include <vtkImageMedian3D.h>
#include <vtkImageResample.h>
#include <vtkImageThreshold.h>


namespace mitk {
  /**
   * @brief Creates a 3D surface from a segmentation. 
   *
   * The resulting surface depends on a filter pipline based on vtkMedian (1) and a gausian filter with vtkStandardDeviation (2).
   * Additional voxel can be changed to an isotropic represantation of the
   * image (ATTANTION: the number of voxels in the will change). The
   * resulting isotropic image has 1mm isotropic voxel.
   *
   * @ingroup ImageFilters
   * @ingroup Process
   */

  class ManualSegmentationToSurfaceFilter : public ImageToSurfaceFilter
  {
    public:
      mitkClassMacro(ManualSegmentationToSurfaceFilter,ImageToSurfaceFilter);

      /**
       * Will preprocess an segmentation voxelwise. The segmentation can use
       * a hole fill relating a median filter and smoth by a gaussian
       * filter. 
       * The image can be interpolatet to an isotropic image.
       * By default every filter is disabled.
       * This method calls CreateSurface from  mitkImageToSurfaceFilter.
       */
      virtual void GenerateData();
      itkNewMacro(Self);


      /**
       * Supplies a method for setting median filter by a bool value.
       */
      itkSetMacro(MedianFilter3D,bool);
      /**
       * Return state of median filter
       */
      itkGetConstMacro(MedianFilter3D,bool);
      /**
       * Enable the median filter (first in pipline).
       */
      itkBooleanMacro(MedianFilter3D);

      /**
       * Supplies a method for activating Interpolation.
       */
      itkSetMacro(Interpolation,bool);
      /**
       * Returns activation state of interpolation filter.
       */
      itkGetConstMacro(Interpolation,bool);
      /**
       * Enable the interpolation filter (second in pipline) for isotropic
       * voxel.
       */
      itkBooleanMacro(Interpolation);

      /**
       * Supplies a method for gaussian filter (third in pipline). 
       */
      itkSetMacro(UseStandardDeviation,bool);
      /**
       * Returns activation state of standard deviation filter.
       */
      itkGetConstMacro(UseStandardDeviation,bool);
      /**
       *  Enable Gaussian StandardDeviation. As well the threshold for 
       *  CreatedSurface will be set to 49 and the image range set 
       *  from 0 to 100. Than there are better results for dividing 
       *  fore- and background.
       */
      itkBooleanMacro(UseStandardDeviation);

      /**
       * Set standard deviation for gaussian Filter.
       * @param _arg by default 1.5
       */
      itkSetMacro(StandardDeviation, double);
      /**
       * Returns the standard deviation of the gaussian filter wich will be
       * used when filter is enabled.
       */
      itkGetConstMacro(StandardDeviation, double);

      /**
       * Set the Kernel for Median3DFilter. By default its set to 3 x 3 x 3. If you choose '1' nothing will be processed in this direction.
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
      void SetInterpolation(int x, int y, int z);

    protected:
      ManualSegmentationToSurfaceFilter();
      virtual ~ManualSegmentationToSurfaceFilter();

      bool m_MedianFilter3D;
      int m_MedianKernelSizeX, m_MedianKernelSizeY, m_MedianKernelSizeZ;
      bool m_UseStandardDeviation; //Gaussian Filter
      double m_StandardDeviation;
      bool m_Interpolation;

      int m_InterpolationX;
      int m_InterpolationY;
      int m_InterpolationZ;

  };//namespace

}
#endif //_MITKMANUALSEGMENTATIONTISURFACEFILTER_h__
