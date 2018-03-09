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


#ifndef FiberBundleMapper3D_H_HEADER_INCLUDED
#define FiberBundleMapper3D_H_HEADER_INCLUDED

#include <mitkVtkMapper.h>
#include <mitkFiberBundle.h>

#include <vtkOpenGLPolyDataMapper.h>
#include <vtkSmartPointer.h>

//#define MITKFIBERBUNDLEMAPPER3D_POLYDATAMAPPER vtkOpenGLPolyDataMapper

//class MITKFIBERBUNDLEMAPPER3D_POLYDATAMAPPER;
class vtkPropAssembly;
class vtkPolyDataMapper;
class vtkLookupTable;
class vtkOpenGLActor;

namespace mitk {

//##Documentation
//## @brief Mapper for FiberBundle
//## @ingroup Mapper

class FiberBundleMapper3D : public VtkMapper
{
public:

    mitkClassMacro(FiberBundleMapper3D, VtkMapper)
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    //========== essential implementation for 3D mapper ========
    const FiberBundle* GetInput();
    vtkProp *GetVtkProp(mitk::BaseRenderer *renderer) override; //looks like depricated.. should be replaced bz GetViewProp()
    static void SetDefaultProperties(DataNode* node, BaseRenderer* renderer = nullptr, bool overwrite = false );
    static void SetVtkMapperImmediateModeRendering(vtkMapper *mapper);
    void GenerateDataForRenderer(mitk::BaseRenderer* renderer) override;
    //=========================================================

    class  FBXLocalStorage3D : public mitk::Mapper::BaseLocalStorage
    {
    public:
        /** \brief Point Actor of a 3D render window. */
        vtkSmartPointer<vtkActor> m_FiberActor;
        /** \brief Point Mapper of a 3D render window. */
        vtkSmartPointer<vtkOpenGLPolyDataMapper> m_FiberMapper;

        vtkSmartPointer<vtkPropAssembly> m_FiberAssembly;

        /** \brief Timestamp of last update of stored data. */
        itk::TimeStamp m_LastUpdateTime;
        /** \brief Constructor of the local storage. Do as much actions as possible in here to avoid double executions. */
        FBXLocalStorage3D(); //if u copy&paste from this 2Dmapper, be aware that the implementation of this constructor is in the cpp file

        ~FBXLocalStorage3D() override
        {
        }
    };

    /** \brief This member holds all three LocalStorages for the 3D render window(s). */
    mitk::LocalStorageHandler<FBXLocalStorage3D> m_LocalStorageHandler;


protected:

    FiberBundleMapper3D();
    ~FiberBundleMapper3D() override;
    void InternalGenerateData(mitk::BaseRenderer *renderer);

    void UpdateShaderParameter(mitk::BaseRenderer*);

private:
    vtkSmartPointer<vtkLookupTable> m_lut;
    float   m_TubeRadius;
    int     m_TubeSides;
    int     m_LineWidth;
    float   m_RibbonWidth;
    bool    m_Lighting;
    vtkSmartPointer<vtkPolyData> m_FiberPolyData;
    mitk::FiberBundle* m_FiberBundle;
};

} // end namespace mitk




#endif /* FiberBundleMapper3D_H_HEADER_INCLUDED */

