#ifndef MITKGEOMETRY2DDATATOSURFACEDATAFILTER_H_HEADER_INCLUDED_C10B22CD
#define MITKGEOMETRY2DDATATOSURFACEDATAFILTER_H_HEADER_INCLUDED_C10B22CD

#include "mitkCommon.h"
#include "mitkSurfaceSource.h"

class vtkPlaneSource;
class vtkTransformPolyDataFilter;

namespace mitk {

class Geometry2DData;

//##ModelId=3EF4A3D701D4
//##Documentation
//## @brief Superclass of all classes having a Geometry2DData as input and
//## generating Images as output
//## @ingroup Process
//## Currently implemented for PlaneGeometry and VtkAbstractTransformGeometry.
//## Currently, this class does not really have subclasses, but does the job
//## for itself. It checks which kind of Geometry2D is stored in the
//## Geometry2DData and - if it knows how - it generates the respective
//## Surface. Of course, this has the disadvantage that for any new type of
//## Geometry2D this class (Geometry2DDataToSurfaceFilter) has to be
//## changed/extended. The idea is to move the type specific generation code in
//## subclasses, and internally (within this class) use a factory to create an
//## instance of the required subclass and delegate the surface generation to
//## it.
//## @todo make extension easier
class Geometry2DDataToSurfaceFilter : public SurfaceSource
{
  public:
    mitkClassMacro(Geometry2DDataToSurfaceFilter, SurfaceSource);
    itkNewMacro(Self);

    //##ModelId=3EF56DCD021E
    virtual void GenerateOutputInformation();

    //##ModelId=3EF56DD10039
    virtual void GenerateData();

    //##ModelId=3EF5795002B2
    const mitk::Geometry2DData *GetInput(void);
    //##ModelId=3EF5795202A1
    const mitk::Geometry2DData *GetInput(unsigned int idx);

    //##ModelId=3EF59AF703BE
    virtual void SetInput(const mitk::Geometry2DData *image);
    //##ModelId=3EF59AFA01D8
    virtual void SetInput(unsigned int index, const mitk::Geometry2DData *image);

    //##Documentation
    //## @brief If @a true (default), use Geometry3D::GetParametricBounds() to define the resolution in parameter space, 
    //## otherwise use m_XResolution and m_YResolution
    itkGetMacro(UseGeometryParametricBounds, bool);
    //##Documentation
    //## @brief If @a true (default), use Geometry3D::GetParametricBounds() to define the resolution in parameter space, 
    //## otherwise use m_XResolution and m_YResolution
    itkSetMacro(UseGeometryParametricBounds, bool);

    //##Documentation
    //## @brief Get x-resolution in parameter space
    //##
    //## The m_VtkPlaneSource will create this many sub-rectangles
    //## in x-direction (see vtkPlaneSource::SetXResolution)
    //## @note Only used, when GetUseGeometryParametricBounds() is @a false, otherwise the
    //## the x-bounds of Geometry3D::GetParametricBounds() are used.
    //## @sa m_XResolution
    itkGetMacro(XResolution, int);
    //##Documentation
    //## @brief Set x-resolution in parameter space
    //##
    //## The m_VtkPlaneSource will create this many sub-rectangles
    //## in x-direction (see vtkPlaneSource::SetXResolution)
    //## @note Only used, when GetUseGeometryParametricBounds() is @a false, otherwise the
    //## the x-bounds of Geometry3D::GetParametricBounds() are used.
    //## @sa m_XResolution
    itkSetMacro(XResolution, int);

    //##Documentation
    //## @brief Get y-resolution in parameter space
    //##
    //## The m_VtkPlaneSource will create this many sub-rectangles
    //## in y-direction (see vtkPlaneSource::SetYResolution)
    //## @note Only used, when GetUseGeometryParametricBounds() is @a false, otherwise the
    //## the y-bounds of Geometry3D::GetParametricBounds() are used.
    //## @sa m_YResolution
    itkGetMacro(YResolution, int);
    //##Documentation
    //## @brief Set y-resolution in parameter space
    //##
    //## The m_VtkPlaneSource will create this many sub-rectangles
    //## in y-direction (see vtkPlaneSource::SetYResolution)
    //## @note Only used, when GetUseGeometryParametricBounds() is @a false, otherwise the
    //## the y-bounds of Geometry3D::GetParametricBounds() are used.
    //## @sa m_YResolution
    itkSetMacro(YResolution, int);

  protected:
    //##ModelId=3EF4A4A70345
    Geometry2DDataToSurfaceFilter();

    //##ModelId=3EF4A4A70363
    virtual ~Geometry2DDataToSurfaceFilter();

    //##ModelId=3EF5734B013A
    //##Documentation
    //## @brief Source to create the vtk-representation of the parameter space rectangle of the Geometry2D
    vtkPlaneSource* m_VtkPlaneSource;

    //##ModelId=3EF5734B01DA
    //##Documentation
    //## @brief Filter to create the vtk-representation of the Geometry2D, which is a
    //## transformation of the m_VtkPlaneSource
    vtkTransformPolyDataFilter* m_VtkTransformPlaneFilter;

    //##Documentation
    //## @brief If @a true, use Geometry3D::GetParametricBounds() to define the resolution in parameter space, 
    //## otherwise use m_XResolution and m_YResolution
    bool m_UseGeometryParametricBounds;

    //##ModelId=3EF5734B020C
    //##Documentation
    //## @brief X-resolution in parameter space
    //##
    //## The m_VtkPlaneSource will create this many sub-rectangles
    //## in x-direction (see vtkPlaneSource::SetXResolution)
    //## @note Only used, when GetUseGeometryParametricBounds() is @a false, otherwise the
    //## the x-bounds of Geometry3D::GetParametricBounds() are used.
    //## @sa m_XResolution
    int m_XResolution;

    //##ModelId=3EF5734B023E
    //##Documentation
    //## @brief Y-resolution in parameter space
    //##
    //## The m_VtkPlaneSource will create this many sub-rectangles
    //## in y-direction (see vtkPlaneSource::SetYResolution)
    //## @note Only used, when GetUseGeometryParametricBounds() is @a false, otherwise the
    //## the y-bounds of Geometry3D::GetParametricBounds() are used.
    int m_YResolution;
};

} // namespace mitk

#endif /* MITKGEOMETRY2DDATATOSURFACEDATAFILTER_H_HEADER_INCLUDED_C10B22CD */
