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


#ifndef ODFVTKMAPPER2D_H_HEADER_INCLUDED
#define ODFVTKMAPPER2D_H_HEADER_INCLUDED

#include "mitkVtkMapper.h"
#include "vtkPropAssembly.h"
#include "vtkAppendPolyData.h"
#include "vtkActor.h"
#include "vtkPolyDataMapper.h"
#include "vtkPlane.h"
#include "vtkCutter.h"
#include "vtkClipPolyData.h"
#include "vtkTransform.h"
#include "vtkDataArrayTemplate.h"
#include "vtkSmartPointer.h"
#include "vtkOdfSource.h"
#include "vtkThickPlane.h"
#include <mitkDiffusionFunctionCollection.h>

namespace mitk {

//##Documentation
//## @brief Mapper for spherical object densitiy function representations
//##
template<class TPixelType, int NrOdfDirections>
class OdfVtkMapper2D : public VtkMapper
{
    struct OdfDisplayGeometry {
        double vp[ 3 ], vnormal[ 3 ];
        Vector3D normal;
        double d, d1, d2;
        mitk::Point3D M3D, L3D, O3D;

        double vp_original[ 3 ], vnormal_original[ 3 ];
        mitk::Vector2D size, origin;

        bool Equals(OdfDisplayGeometry other)
        {
            return other.vp_original[0] == vp[0] &&
                    other.vp_original[1] == vp[1] &&
                    other.vp_original[2] == vp[2] &&
                    other.vnormal_original[0] == vnormal[0] &&
                    other.vnormal_original[1] == vnormal[1] &&
                    other.vnormal_original[2] == vnormal[2] &&
                    other.size[0] == size[0] &&
                    other.size[1] == size[1] &&
                    other.origin[0] == origin[0] &&
                    other.origin[1] == origin[1];
        }
    };

public:

    mitkClassMacro(OdfVtkMapper2D,VtkMapper)
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    vtkProp* GetVtkProp(mitk::BaseRenderer* renderer) override;
    bool IsVisibleOdfs(mitk::BaseRenderer* renderer);
    void MitkRenderOverlay(mitk::BaseRenderer* renderer) override;
    void MitkRenderOpaqueGeometry(mitk::BaseRenderer* renderer) override;
    void MitkRenderTranslucentGeometry(mitk::BaseRenderer* renderer) override;
    void MitkRenderVolumetricGeometry(mitk::BaseRenderer*  /*renderer*/) override{}

    OdfDisplayGeometry MeasureDisplayedGeometry(mitk::BaseRenderer* renderer);
    double GetMinImageSpacing( int index );
    void ApplyPropertySettings();
    virtual void Slice(mitk::BaseRenderer* renderer, OdfDisplayGeometry dispGeo);
    virtual int GetIndex(mitk::BaseRenderer* renderer);
    static void SetDefaultProperties(DataNode* node, BaseRenderer* renderer = nullptr, bool overwrite = false);
    void Update(mitk::BaseRenderer * renderer) override;
    void GenerateDataForRenderer(mitk::BaseRenderer* renderer) override;
    bool IsLODEnabled( BaseRenderer * /*renderer*/ ) const override { return true; }

    class LocalStorage : public mitk::Mapper::BaseLocalStorage
    {
    public:

        std::vector< vtkSmartPointer<vtkPropAssembly> >       m_PropAssemblies;
        std::vector< vtkSmartPointer<vtkAppendPolyData> >     m_OdfsPlanes;
        std::vector< vtkSmartPointer<vtkActor> >              m_OdfsActors;
        std::vector< vtkSmartPointer<vtkPolyDataMapper> >     m_OdfsMappers;
        vtkSmartPointer< vtkPolyData >                        m_TemplateOdf;

        itk::TimeStamp                      m_LastUpdateTime;

        /** \brief Default constructor of the local storage. */
        LocalStorage();
        /** \brief Default deconstructor of the local storage. */
        ~LocalStorage() override
        {
        }
    };

protected:

    OdfVtkMapper2D();
    ~OdfVtkMapper2D() override;

    static void GlyphMethod(void *arg);
    bool IsPlaneRotated(mitk::BaseRenderer* renderer);
    static bool m_ToggleTensorEllipsoidView;
    static bool m_ToggleColourisationMode;
    static bool m_ToggleGlyphPlacementMode;


    typedef vnl_matrix_fixed<double, 3, 3> DirectionsType;


private:

    mitk::Image* GetInput();

    static vtkSmartPointer<vtkTransform>              m_OdfTransform;
    static vtkSmartPointer<vtkOdfSource>              m_OdfSource;
    static float                                      m_Scaling;
    static int                                        m_Normalization;
    static int                                        m_ScaleBy;
    static float                                      m_IndexParam1;
    static float                                      m_IndexParam2;
    static vtkSmartPointer<vtkDoubleArray>            m_ColourScalars;
    int                                               m_ShowMaxNumber;
    std::vector< vtkSmartPointer<vtkPlane> >          m_Planes;
    std::vector< vtkSmartPointer<vtkCutter> >         m_Cutters;
    std::vector< vtkSmartPointer<vtkThickPlane> >     m_ThickPlanes1;
    std::vector< vtkSmartPointer<vtkClipPolyData> >   m_Clippers1;
    std::vector< vtkSmartPointer<vtkThickPlane> >     m_ThickPlanes2;
    std::vector< vtkSmartPointer<vtkClipPolyData> >   m_Clippers2;
    vtkImageData*                                     m_VtkImage ;
    std::vector< OdfDisplayGeometry >                 m_LastDisplayGeometry;
    mitk::LocalStorageHandler<LocalStorage>           m_LSH;

    static vnl_matrix<float>                          m_Sh2Basis;
    static vnl_matrix<float>                          m_Sh4Basis;
    static vnl_matrix<float>                          m_Sh6Basis;
    static vnl_matrix<float>                          m_Sh8Basis;
    static vnl_matrix<float>                          m_Sh10Basis;
    static vnl_matrix<float>                          m_Sh12Basis;
};

} // namespace mitk

#include "mitkOdfVtkMapper2D.txx"

#endif /* ODFVTKMAPPER2D_H_HEADER_INCLUDED */
