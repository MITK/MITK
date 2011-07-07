#ifndef ClippedSurfaceBoundsCalculator_h_included
#define ClippedSurfaceBoundsCalculator_h_included

#include "mitkGeometry2DData.h"
#include "mitkImage.h"
#include "mitkGeometry2DDataToSurfaceFilter.h"
#include "mitkSlicedGeometry3D.h"
#include "mitkSliceNavigationController.h"
#include "mitkCommon.h"
#include "vtkPoints.h"
#include "mitkVector.h"


/**
 * \brief Class to find
 *
*/

namespace mitk
{

class MITK_CORE_EXPORT ClippedSurfaceBoundsCalculator
{

  public:
    ClippedSurfaceBoundsCalculator(const mitk::PlaneGeometry* geometry = NULL, mitk::Image::Pointer image = NULL);
    virtual ~ClippedSurfaceBoundsCalculator();


    void SetInput(const mitk::PlaneGeometry* geometry, mitk::Image::Pointer image);
    void Update();

    typedef std::pair<int, int> outputType;

    outputType GetMinMaxSpatialDirectionX();
    outputType GetMinMaxSpatialDirectionY();
    outputType GetMinMaxSpatialDirectionZ();

  protected:
    void CalculateSurfaceBounds();
    void CalculateSurfaceBounds(Point3D right, Point3D bottom);
    mitk::Point3D* GetIntersectionPoint(Point3D right, Point3D bottom);

    const mitk::PlaneGeometry* m_Geometry2D;
    mitk::Image::Pointer m_Image;
    std::vector< outputType > m_MinMaxOutput;

};

} //namespace mitk
#endif


