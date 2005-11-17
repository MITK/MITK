#ifndef _MITKIMAGETOSURFACEFILTER_h__
#define _MITKIMAGETOSURFACEFILTER_h__

#include <vtkPolyData.h>
#include "mitkCommon.h"
#include "mitkSurfaceSource.h"
#include "mitkSurface.h"

#include <mitkImage.h>
#include <vtkImageData.h>

#include "vtkSmoothPolyDataFilter.h"
#include "vtkMarchingCubes.h"


namespace mitk {
/**
 * @brief Converts pixel data to surface data
 *
 * The resulting surface has the same size as the input image. The image is smoothed by
 * vtkDecimatePro and vtkSmoothPolyDataFilter. Both are enabled by default. It's also possible
 * to create time sliced surfaces.
 *
 * @ingroup ImageFilters
 * @ingroup Process
 */

class ImageToSurfaceFilter : public SurfaceSource
{
public:

  enum DecimationType {NoDecimation,Decimate,DecimatePro};

  mitkClassMacro(ImageToSurfaceFilter, SurfaceSource);
  itkNewMacro(Self);

  /**
  * For each image time slice a surface will be created.
  */
  virtual void GenerateData();
  virtual void GenerateOutputInformation();
  //virtual void GenerateInputRequestRegion();

  /**
  * Set/Get *image input 3D or 3D+t
  */
  const mitk::Image *GetInput(void);
  
  /**
  * Set/Get *image input 3D or 3D+t
  */
  virtual void SetInput(const mitk::Image *image);

  /**
  * Set disired threshold for input image 1 for binary
  */
  itkSetMacro(Threshold, ScalarType);

  /**
  * Get Threshold from SkinExtractor. Threshold can be manipulated by inherited classes.
  */
  itkGetConstMacro(Threshold, ScalarType);

  /*
  * Enable/Disale surface smoothing.
  */
  itkGetConstMacro(Smooth,bool);
  itkSetMacro(Smooth,bool);
  itkBooleanMacro(Smooth);

  /** Switch Decimate mode  */
  itkGetConstMacro(Decimate,DecimationType);
  itkSetMacro(Decimate,DecimationType);
  
  /**
  * Set disired TargetReduction for decimate traiangles
  */
  itkSetMacro(TargetReduction, float);

  /**
  * Return reduction factor for VtkDecimatePro
  */
  itkGetConstMacro(TargetReduction, float);


protected:
  ImageToSurfaceFilter();
  virtual ~ImageToSurfaceFilter();

  /**
  * With the given threshold vtkMarchingcube creates the Surface. By default nothing
  * will be done. Optional its possible to reduce the number of triangles (vtkDecimat)
  * or smooth the data (vtkSmoothPolyDataFilter).
  * @param time selected slice or "0" for single
  * @param *vtkimage input image
  * @param *surface output
  * @param threshold can be different from SetThreshold()
  */
  void CreateSurface(int time, vtkImageData *vtkimage, mitk::Surface * surface, const ScalarType threshold);

  bool m_Smooth;
  DecimationType m_Decimate;
  ScalarType m_Threshold; 
  float m_TargetReduction;

};

} // namespace mitk

#endif //_MITKIMAGETOSURFACEFILTER_h__
