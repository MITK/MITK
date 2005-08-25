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

namespace mitk {

class ImageToSurfaceFilter : public SurfaceSource
{
public:  
  mitkClassMacro(ImageToSurfaceFilter, SurfaceSource);
  itkNewMacro(Self);

  virtual void GenerateData();
  virtual void GenerateOutputInformation();
  //virtual void GenerateInputRequestRegion();

  const mitk::Image *GetInput(void);
  virtual void SetInput(const mitk::Image *image);

  itkSetMacro(Threshold, ScalarType);
  itkGetConstMacro(Threshold, ScalarType);

  itkSetMacro(SetSmooth,bool);
  itkGetConstMacro(SetSmooth,bool);
  itkBooleanMacro(SetSmooth);

  itkSetMacro(SetDecimate,bool);
  itkGetConstMacro(SetDecimate,bool);
  itkBooleanMacro(SetDecimate);


protected:
  ImageToSurfaceFilter();
  virtual ~ImageToSurfaceFilter();
  void CreateSurface(int time, vtkImageData *vtkimage, mitk::Surface * surface, const ScalarType threshold);
  
  bool m_SetSmooth;
  bool m_SetDecimate;
  ScalarType m_Threshold; //für MarchingCube oder Gaus bei ManualSegmentationFilter

};

} // namespace mitk

#endif //_MITKIMAGETOSURFACEFILTER_h__
