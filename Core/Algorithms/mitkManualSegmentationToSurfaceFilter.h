#ifndef _MITKMANUALSEGMENTATIONTISURFACEFILTER_h__
#define _MITKMANUALSEGMENTATIONTISURFACEFILTER_h__

#include "mitkImageToSurfaceFilter.h"
#include <vtkImageGaussianSmooth.h>
#include <vtkImageMedian3D.h>
#include <vtkImageResample.h>
#include <vtkImageThreshold.h>


namespace mitk {
/**
 * @brief Smooths 3D pixel data
 *
 * The resulting image is smoothed by a Median3D and a Gaussian Filter.
 *
 * @ingroup ImageFilters
 * @ingroup Process
 */

class ManualSegmentationToSurfaceFilter : public ImageToSurfaceFilter
{
public:
  mitkClassMacro(ManualSegmentationToSurfaceFilter,ImageToSurfaceFilter);
  
  /**
  * Reimplemented from mitkImageToSurfaceFilter
  */
  virtual void GenerateData();
  itkNewMacro(Self);

  /**
  * Enable/Disable Median3DFilter
  */
  itkSetMacro(UseMedianFilter3D,bool);
  itkGetConstMacro(UseMedianFilter3D,bool);
  itkBooleanMacro(UseMedianFilter3D);

  /**
  * Enable/Disable Interpolation to 1mm x 1mm x 1mm
  */
  itkSetMacro(SetInterpolation,bool);
  itkGetConstMacro(SetInterpolation,bool);
  itkBooleanMacro(SetInterpolation);

  /**
  * Enable/Disable Gaussian StandardDeviation. As well the threshold for CreatedSurface will be set to 49 and the image range set from 0 to 100.
  */
  itkSetMacro(UseStandardDeviation,bool);
  itkGetConstMacro(UseStandardDeviation,bool);
  itkBooleanMacro(UseStandardDeviation);

  /**
  * Set/Get Standard Deviation for Gaussian Filter.
  * @param StdDeviation by default 1.5
  */
  itkSetMacro(StdDeviation, double);
  itkGetConstMacro(StdDeviation, double);

  /**
  * Set the Kernel for Median3DFilter. By default its set to 3 x 3 x 3. If you are choosig 1 nothing will be processed in this direction.
  */
  void SetMedianKernelSize(int x, int y, int z);
  itkGetConstMacro(MedianKernelSizeX, int);
  itkGetConstMacro(MedianKernelSizeY, int);
  itkGetConstMacro(MedianKernelSizeZ, int);
  
protected:
  ManualSegmentationToSurfaceFilter();
  virtual ~ManualSegmentationToSurfaceFilter();

  bool m_UseMedianFilter3D;
  int m_MedianKernelSizeX, m_MedianKernelSizeY, m_MedianKernelSizeZ;
  bool m_UseStandardDeviation; //Gaussian Filter
  bool m_SetInterpolation;
  double m_StdDeviation;
};//namespace

}
#endif //_MITKMANUALSEGMENTATIONTISURFACEFILTER_h__
