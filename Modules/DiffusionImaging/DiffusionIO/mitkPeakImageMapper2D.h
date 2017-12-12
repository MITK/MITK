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

#ifndef PeakImageMapper2D_H_HEADER_INCLUDED
#define PeakImageMapper2D_H_HEADER_INCLUDED

//MITK Rendering
#include <mitkCommon.h>
#include <mitkBaseRenderer.h>
#include <mitkVtkMapper.h>
#include <mitkPeakImage.h>
#include <vtkSmartPointer.h>

#define MITKPeakImageMapper2D_POLYDATAMAPPER vtkOpenGLPolyDataMapper

class vtkActor;
class mitkBaseRenderer;
class MITKPeakImageMapper2D_POLYDATAMAPPER;
class vtkCutter;
class vtkPlane;
class vtkPolyData;



namespace mitk {

struct IShaderRepository;

class PeakImageMapper2D : public VtkMapper
{

public:
    mitkClassMacro(PeakImageMapper2D, VtkMapper)
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)
    mitk::PeakImage* GetInput();


    /** \brief Checks whether this mapper needs to update itself and generate data. */
    virtual void Update(mitk::BaseRenderer * renderer) override;


    static void SetDefaultProperties(DataNode* node, BaseRenderer* renderer = nullptr, bool overwrite = false );

    virtual void UpdateVtkTransform(mitk::BaseRenderer *renderer) override;

    //### methods of MITK-VTK rendering pipeline
    virtual vtkProp* GetVtkProp(mitk::BaseRenderer* renderer) override;
    //### end of methods of MITK-VTK rendering pipeline


    class  FBXLocalStorage : public mitk::Mapper::BaseLocalStorage
    {
    public:
        /** \brief Point Actor of a 2D render window. */
        vtkSmartPointer<vtkActor> m_PointActor;
        /** \brief Point Mapper of a 2D render window. */
        vtkSmartPointer<MITKPeakImageMapper2D_POLYDATAMAPPER> m_Mapper;
        vtkSmartPointer<vtkPlane> m_SlicingPlane;  //needed later when optimized 2D mapper
        vtkSmartPointer<vtkPolyData> m_SlicedResult; //might be depricated in optimized 2D mapper

        /** \brief Timestamp of last update of stored data. */
        itk::TimeStamp m_LastUpdateTime;
        /** \brief Constructor of the local storage. Do as much actions as possible in here to avoid double executions. */
        FBXLocalStorage(); //if u copy&paste from this 2Dmapper, be aware that the implementation of this constructor is in the cpp file

        ~FBXLocalStorage()
        {
        }
    };

    /** \brief This member holds all three LocalStorages for the three 2D render windows. */
    mitk::LocalStorageHandler<FBXLocalStorage> m_LocalStorageHandler;



protected:
    PeakImageMapper2D();
    virtual ~PeakImageMapper2D();

    /** Does the actual resampling, without rendering. */
    virtual void GenerateDataForRenderer(mitk::BaseRenderer*) override;

    void UpdateShaderParameter(mitk::BaseRenderer*);

private:
    vtkSmartPointer<vtkLookupTable> m_lut;
};


}//end namespace

#endif
