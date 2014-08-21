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


#ifndef FiberBundleXMapper3D_H_HEADER_INCLUDED
#define FiberBundleXMapper3D_H_HEADER_INCLUDED

//#include <mitkBaseData.h> //?? necessary
#include <mitkVtkMapper.h>
#include <mitkFiberBundleX.h>

#include <vtkSmartPointer.h>
#include <vtkOpenGLPolyDataMapper.h>
#include <vtkOpenGLActor.h>
#include <vtkLookupTable.h>



class vtkPropAssembly;




namespace mitk {

//##Documentation
//## @brief Mapper for FiberBundleX
//## @ingroup Mapper

class FiberBundleXMapper3D : public VtkMapper
{
public:

    mitkClassMacro(FiberBundleXMapper3D, VtkMapper)
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    //========== essential implementation for 3D mapper ========
    const FiberBundleX* GetInput();
    virtual vtkProp *GetVtkProp(mitk::BaseRenderer *renderer); //looks like depricated.. should be replaced bz GetViewProp()
    static void SetDefaultProperties(DataNode* node, BaseRenderer* renderer = NULL, bool overwrite = false );
    static void SetVtkMapperImmediateModeRendering(vtkMapper *mapper);
    virtual void GenerateDataForRenderer(mitk::BaseRenderer* renderer);
    //=========================================================

    class  FBXLocalStorage3D : public mitk::Mapper::BaseLocalStorage
    {
    public:
        /** \brief Point Actor of a 3D render window. */
        vtkSmartPointer<vtkActor> m_FiberActor;
        /** \brief Point Mapper of a 3D render window. */
        vtkSmartPointer<vtkPolyDataMapper> m_FiberMapper;

        vtkSmartPointer<vtkPropAssembly> m_FiberAssembly;

        /** \brief Timestamp of last update of stored data. */
        itk::TimeStamp m_LastUpdateTime;
        /** \brief Constructor of the local storage. Do as much actions as possible in here to avoid double executions. */
        FBXLocalStorage3D(); //if u copy&paste from this 2Dmapper, be aware that the implementation of this constructor is in the cpp file

        ~FBXLocalStorage3D()
        {
        }
    };

    /** \brief This member holds all three LocalStorages for the 3D render window(s). */
    mitk::LocalStorageHandler<FBXLocalStorage3D> m_LocalStorageHandler;


protected:

    FiberBundleXMapper3D();
    virtual ~FiberBundleXMapper3D();
    void InternalGenerateData(mitk::BaseRenderer *renderer);

    void UpdateVtkObjects(); //??

private:
    vtkSmartPointer<vtkLookupTable> m_lut;


};

} // end namespace mitk




#endif /* FiberBundleXMapper3D_H_HEADER_INCLUDED */

