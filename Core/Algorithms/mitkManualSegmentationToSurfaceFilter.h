#ifndef _MITKMANUALSEGMENTATIONTISURFACEFILTER_h__
#define _MITKMANUALSEGMENTATIONTISURFACEFILTER_h__

#include "mitkImageToSurfaceFilter.h"
#include <vtkImageGaussianSmooth.h>
#include <vtkImageMedian3D.h>
#include <vtkImageResample.h>
#include <vtkImageThreshold.h>


namespace mitk {
/**
 * @brief Smooths 3D pixel data and create surface.
 *
 * The resulting image can be smoothed by a Median3D and a Gaussian Filter. If interpolation
 * is used all voxels are resized to 1mm in each direction.
 *
 * @ingroup ImageFilters
 * @ingroup Process
 */

class ManualSegmentationToSurfaceFilter : public ImageToSurfaceFilter
{
public:
  mitkClassMacro(ManualSegmentationToSurfaceFilter,ImageToSurfaceFilter);

  /**
  * New Implementation from mitkImageToSurfaceFilter
  */
  virtual void GenerateData();
  itkNewMacro(Self);

  
  itkSetMacro(MedianFilter3D,bool);
  itkGetConstMacro(MedianFilter3D,bool);
  /**
  * Enable/Disable Median3DFilter for smoothing the segementation
  */
  itkBooleanMacro(MedianFilter3D);

  /**
  * Enable/Disable Interpolation to 1mm x 1mm x 1mm pixel.
  */
  itkSetMacro(SetInterpolation,bool);
  itkGetConstMacro(SetInterpolation,bool);
  /**
  * Enable/Disable Interpolation to 1mm x 1mm x 1mm pixel.
  */
  itkBooleanMacro(SetInterpolation);

  
  itkSetMacro(UseStandardDeviation,bool);
  itkGetConstMacro(UseStandardDeviation,bool);
  /**
  * Enable/Disable Gaussian StandardDeviation. As well the threshold for CreatedSurface will be set to 49 and the image range set from 0 to 100.
  */
  itkBooleanMacro(UseStandardDeviation);

  /**
  * Set/Get Standard Deviation for Gaussian Filter.
  * @param double deviation by default 1.5
  */
  itkSetMacro(StandardDeviation, double);
  itkGetConstMacro(StandardDeviation, double);

  /**
  * Set the Kernel for Median3DFilter. By default its set to 3 x 3 x 3. If you choose '1' nothing will be processed in this direction.
  */
  void SetMedianKernelSize(int x, int y, int z);
  itkGetConstMacro(MedianKernelSizeX, int);
  itkGetConstMacro(MedianKernelSizeY, int);
  itkGetConstMacro(MedianKernelSizeZ, int);

protected:
  ManualSegmentationToSurfaceFilter();
  virtual ~ManualSegmentationToSurfaceFilter();

  bool m_MedianFilter3D;
  int m_MedianKernelSizeX, m_MedianKernelSizeY, m_MedianKernelSizeZ;
  bool m_UseStandardDeviation; //Gaussian Filter
  double m_StandardDeviation;
  bool m_SetInterpolation;
  
};//namespace

}
#endif //_MITKMANUALSEGMENTATIONTISURFACEFILTER_h__