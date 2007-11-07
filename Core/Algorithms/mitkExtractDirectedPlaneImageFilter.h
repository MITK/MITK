
#ifndef mitkExtractDirectedPlaneImageFilter_h_Included
#define mitkExtractDirectedPlaneImageFilter_h_Included

//#include "mitkCommon.h"
#include "mitkImageToImageFilter.h"

#include "vtkImageReslice.h"

class vtkPoints;

namespace mitk
{
  /**
  \brief Extracts a 2D slice of arbitrary geometry from a 3D or 4D image.

  \sa mitkImageMapper2D

  \ingroup ImageToImageFilter

  This class takes a 3D or 4D mitk::Image as input and tries to extract one slice from it.
  This slice can be arbitrary oriented in space. The 2D slice is resliced by a 
  vtk::ResliceImage filter if not perpendicular to the input image.

  The world geometry of the plane to be extracted image must be given as an input 
  to the filter in order to correctly calculate world coordinates of the extracted slice.
  Setting a timestep from which the plane should be extracted is optional.

  Output will not be set if there was a problem extracting the desired slice.

  Last contributor: $Author: T. Schwarz$
  */

  class ExtractDirectedPlaneImageFilter : public ImageToImageFilter
  {
  public:

    mitkClassMacro(ExtractDirectedPlaneImageFilter, ImageToImageFilter);
    itkNewMacro(ExtractDirectedPlaneImageFilter);

    itkSetMacro( WorldGeometry, Geometry2D* );

    // The Reslicer is accessible to configure the desired interpolation;
    // (See vtk::ImageReslice class for documentation).
    // Misusage is at your own risk...
    itkGetMacro( Reslicer, vtkImageReslice* );

    // The target timestep in a 4D image from which the 2D plane is supposed
    // to be extracted.
    itkSetMacro( TargetTimestep, unsigned int );
    itkGetMacro( TargetTimestep, unsigned int );

    itkSetMacro( InPlaneResampleExtentByGeometry, bool );
    itkGetMacro( InPlaneResampleExtentByGeometry, bool );

  protected:

    ExtractDirectedPlaneImageFilter(); // purposely hidden
    virtual ~ExtractDirectedPlaneImageFilter();

    virtual void GenerateData();
    virtual void GenerateOutputInformation();

    bool CalculateClippedPlaneBounds( const Geometry3D *boundingGeometry, 
      const PlaneGeometry *planeGeometry, vtkFloatingPointType *bounds );
    bool LineIntersectZero( vtkPoints *points, int p1, int p2,
      vtkFloatingPointType *bounds );

    const Geometry2D*  m_WorldGeometry;
    vtkImageReslice *  m_Reslicer;

    unsigned int       m_TargetTimestep;
    bool               m_InPlaneResampleExtentByGeometry;
  };

} // namespace mitk

#endif // mitkExtractDirectedPlaneImageFilter_h_Included
