/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef _MITK_VECTOR_IMAGE_VTK_MAPPER_3D__H
#define _MITK_VECTOR_IMAGE_VTK_MAPPER_3D__H

#include <MitkDiffusionCoreExports.h>

#include "mitkVtkMapper.h"
#include "mitkImage.h"

class vtkMaskedGlyph3D;
class vtkActor;
class vtkPolyDataMapper;
class vtkMaskPoints;


namespace mitk
{

class MITKDIFFUSIONCORE_EXPORT VectorImageVtkGlyphMapper3D : public VtkMapper
{
public:
    mitkClassMacro( VectorImageVtkGlyphMapper3D, VtkMapper );

    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

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

    LocalStorageHandler<BaseLocalStorage> m_LSH;

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
     * This method is called, each time a specific renderer is updated.
     */
    virtual void GenerateDataForRenderer( mitk::BaseRenderer* renderer );

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



