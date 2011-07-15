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

    typedef std::pair<int, int> OutputType;

    OutputType GetMinMaxSpatialDirectionX();
    OutputType GetMinMaxSpatialDirectionY();
    OutputType GetMinMaxSpatialDirectionZ();

  protected:
    const mitk::PlaneGeometry* m_PlaneGeometry;
    mitk::Image::Pointer m_Image;
    std::vector< OutputType > m_MinMaxOutput;

};

} //namespace mitk
#endif


