#ifndef MitkPolyDataGLMapper2D_H
#define MitkPolyDataGLMapper2D_H

#include "mitkCommon.h"
#include "mitkGLMapper2D.h"

class vtkCutter;
class vtkPlane;
class vtkLookupTable;
class vtkPolyData;

namespace mitk
{

class BaseRenderer;

/*
 * The corresponding Mapper3D has to generate the PolyData in its GenerateData-Method,
 * otherwise this mapper won't work!
 */
class PolyDataGLMapper2D : public GLMapper2D
{
public:

    mitkClassMacro( PolyDataGLMapper2D, GLMapper2D );
    
    const mitk::BaseData * GetInput(void);

    itkNewMacro( Self );

    virtual void Paint( mitk::BaseRenderer * renderer );

    virtual void Update();

protected:

    PolyDataGLMapper2D();
    
    virtual ~PolyDataGLMapper2D();

    virtual vtkPolyData* GetVtkPolyData( void );

    virtual bool IsConvertibleToVtkPolyData();

    vtkPlane* m_Plane;

    vtkCutter* m_Cutter;

    vtkLookupTable *m_LUT;

};

} // namespace mitk
#endif /* MitkPolyDataGLMapper2D_H */
