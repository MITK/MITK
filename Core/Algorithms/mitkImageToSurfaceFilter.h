#ifndef _MITKIMAGETOSURFACEFILTER_h__
#define _MITKIMAGETOSURFACEFILTER_h__

#include <vtkPolyData.h>
#include "mitkCommon.h"
#include "mitkSurfaceSource.h"
#include "mitkSurface.h"

#include <mitkImage.h>
#include <vtkImageData.h>

#include <vtkDecimate.h>
#include "vtkSmoothPolyDataFilter.h"
#include "vtkMarchingCubes.h"


namespace mitk {
/**
 * @brief Converts pixel data to surface data
 *
 * The resulting surface has the same size as the input image. The image is smoothed by 
 * vtkDecimate and vtkSmoothPolyDataFilter. Both are enabled by default. It's also posible 
 * to create time sliced surfaces.
 *
 * @ingroup ImageFilters
 * @ingroup Process
 */

class ImageToSurfaceFilter : public SurfaceSource
{
public:  
  mitkClassMacro(ImageToSurfaceFilter, SurfaceSource);
  itkNewMacro(Self);

  /**
  * For each image time slice a surface will be created.
  */
  virtual void GenerateData();
  virtual void GenerateOutputInformation();
  //virtual void GenerateInputRequestRegion();

  /** 
  * Set/Get *image input 3D or 4D
  */
  const mitk::Image *GetInput(void);
  virtual void SetInput(const mitk::Image *image);

  /**
  * Set disired threshold for input image e.g. 1 for binary
  */
  itkSetMacro(Threshold, ScalarType);
  itkGetConstMacro(Threshold, ScalarType);

  /*
  * Turn On/Off Surface smoothing
  */
  itkSetMacro(SetSmooth,bool);
  itkGetConstMacro(SetSmooth,bool);
  itkBooleanMacro(SetSmooth);

  /**
  * Turn On/Off Decimate triangulars
  */
  itkSetMacro(SetDecimate,bool);
  itkGetConstMacro(SetDecimate,bool);
  itkBooleanMacro(SetDecimate);


protected:
  ImageToSurfaceFilter();
  virtual ~ImageToSurfaceFilter();

  /** 
  * With the given threshold vtkMarchingcube creates the Surface. By default nothing
  * will be done. Optionaly its possible to reduce the number of triangles (vtkDecimat)
  * or smooth th data (vtkSmoothPolyDataFilter).
  * @param time selected slice or "0" for single
  * @param *vtkimage input image
  * @param *surface output 
  * @param threshold can be different from SetThreshold()
  */
  void CreateSurface(int time, vtkImageData *vtkimage, mitk::Surface * surface, const ScalarType threshold);
  
  bool m_SetSmooth;
  bool m_SetDecimate;
  ScalarType m_Threshold; //für MarchingCube oder Gaus bei ManualSegmentationFilter

};

} // namespace mitk

#endif //_MITKIMAGETOSURFACEFILTER_h__
