#ifndef _MITK_VECTOR_IMAGE_VTK_MAPPER_3D__H
#define _MITK_VECTOR_IMAGE_VTK_MAPPER_3D__H

#include "MitkDiffusionImagingExports.h"

#include "mitkBaseVtkMapper3D.h"
#include "mitkImage.h"

class vtkMaskedGlyph3D;
class vtkActor;
class vtkPolyDataMapper;
class vtkMaskPoints;


namespace mitk
{

class MitkDiffusionImaging_EXPORT VectorImageVtkGlyphMapper3D : public BaseVtkMapper3D
{
public:
    mitkClassMacro( VectorImageVtkGlyphMapper3D, BaseVtkMapper3D );

    itkNewMacro( Self );

    enum GlyphType {LineGlyph, ArrowGlyph};
    
    itkSetMacro(MaximumNumberOfPoints, unsigned int);
    itkGetMacro(MaximumNumberOfPoints, unsigned int);
    
    itkSetMacro(UseMaskPoints, bool);
    itkGetMacro(UseMaskPoints, bool);
    itkBooleanMacro(UseMaskPoints);
    
    itkSetMacro(RandomMode, bool);
    itkGetMacro(RandomMode, bool);
    itkBooleanMacro(RandomMode);

    virtual vtkProp* GetVtkProp(mitk::BaseRenderer* renderer);

protected:
    /**
     * Constructor. Doesn't do anything...
     */
    VectorImageVtkGlyphMapper3D();

    /**
     * Destructor
     */
    virtual ~VectorImageVtkGlyphMapper3D();

    /**
     * Generate a vtkPolyData by creating vectors as glyphs
     */
    virtual void GenerateData();

    /**
     * This method is called, each time a specific renderer is updated.
     */
    virtual void GenerateData( mitk::BaseRenderer* renderer );

    /**
     * Returns the input data object of the given filter. In this
     * case, a mitk::Image is returned.
     */
    Image* GetInput();
    
    vtkMaskedGlyph3D* m_Glyph3DGenerator;
    vtkActor* m_Glyph3DActor;
    vtkPolyDataMapper* m_Glyph3DMapper;
    
    GlyphType m_GlyphType;
    unsigned int m_MaximumNumberOfPoints;
    bool m_UseMaskPoints;
    bool m_RandomMode;
    
};


} //end of namespace mitk

#endif



