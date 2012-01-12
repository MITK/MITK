/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-05-12 19:56:03 +0200 (Di, 12 Mai 2009) $
Version:   $Revision: 17179 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef FiberBundleXMapper3D_H_HEADER_INCLUDED
#define FiberBundleXMapper3D_H_HEADER_INCLUDED

//#include <mitkBaseData.h> //?? necessary
#include <MitkDiffusionImagingExports.h>
#include <mitkVtkMapper3D.h>
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

class MitkDiffusionImaging_EXPORT FiberBundleXMapper3D : public VtkMapper3D
{
public:

    mitkClassMacro(FiberBundleXMapper3D, VtkMapper3D);
    itkNewMacro(Self);

    //========== essential implementation for 3D mapper ========
    const FiberBundleX* GetInput();
    virtual vtkProp *GetVtkProp(mitk::BaseRenderer *renderer); //looks like depricated.. should be replaced bz GetViewProp()
    static void SetDefaultProperties(DataNode* node, BaseRenderer* renderer = NULL, bool overwrite = false );
    virtual void ApplyProperties(mitk::BaseRenderer* renderer);
    static void SetVtkMapperImmediateModeRendering(vtkMapper *mapper);
    virtual void GenerateDataForRenderer(mitk::BaseRenderer* renderer);
    //=========================================================
    virtual void GenerateData(mitk::BaseRenderer *renderer);

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
    mitk::Mapper::LocalStorageHandler<FBXLocalStorage3D> m_LSH;


protected:

    FiberBundleXMapper3D();
    virtual ~FiberBundleXMapper3D();

    void UpdateVtkObjects(); //??

private:
    vtkSmartPointer<vtkLookupTable> m_lut;
    

};

} // end namespace mitk




#endif /* FiberBundleXMapper3D_H_HEADER_INCLUDED */

