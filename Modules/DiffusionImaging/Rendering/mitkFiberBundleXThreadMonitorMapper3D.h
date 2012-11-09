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


#ifndef FiberBundleXThreadMonitorMapper3D_H_HEADER_INCLUDED
#define FiberBundleXThreadMonitorMapper3D_H_HEADER_INCLUDED

//#include <mitkBaseData.h> //?? necessary
#include <MitkDiffusionImagingExports.h>
#include <mitkVtkMapper3D.h>
#include <mitkFiberBundleXThreadMonitor.h>

#include <vtkSmartPointer.h>
#include <vtkPolyDataMapper.h>
#include <vtkTextActor.h>



class vtkPropAssembly;




namespace mitk {

  //##Documentation
  //## @brief Mapper for FiberBundleX
  //## @ingroup Mapper

  class MitkDiffusionImaging_EXPORT FiberBundleXThreadMonitorMapper3D : public VtkMapper3D
  {
  public:

    mitkClassMacro(FiberBundleXThreadMonitorMapper3D, VtkMapper3D);
    itkNewMacro(Self);

    //========== essential implementation for 3D mapper ========
    const FiberBundleXThreadMonitor* GetInput();
    virtual vtkProp *GetVtkProp(mitk::BaseRenderer *renderer); //looks like depricated.. should be replaced bz GetViewProp()
    static void SetDefaultProperties(DataNode* node, BaseRenderer* renderer = NULL, bool overwrite = false );
    virtual void ApplyProperties(mitk::BaseRenderer* renderer);
    static void SetVtkMapperImmediateModeRendering(vtkMapper *mapper);
    virtual void GenerateDataForRenderer(mitk::BaseRenderer* renderer);
    virtual void GenerateData();
    //=========================================================

  protected:

    FiberBundleXThreadMonitorMapper3D();
    virtual ~FiberBundleXThreadMonitorMapper3D();

    void UpdateVtkObjects(); //??


    vtkSmartPointer<vtkPolyDataMapper> m_FiberMonitorMapper;
    vtkSmartPointer<vtkTextActor> m_TextActorClose;
    vtkSmartPointer<vtkTextActor> m_TextActorOpen;
    vtkSmartPointer<vtkTextActor> m_TextActorHeading;
    vtkSmartPointer<vtkTextActor> m_TextActorMask;
    vtkSmartPointer<vtkTextActor> m_TextActorStatus;
    vtkSmartPointer<vtkTextActor> m_TextActorStarted;
    vtkSmartPointer<vtkTextActor> m_TextActorFinished;
    vtkSmartPointer<vtkTextActor> m_TextActorTerminated;
    vtkPropAssembly* m_FiberAssembly;

  private:
    double m_lastModifiedMonitorNodeTime;

  };

} // end namespace mitk




#endif /* FiberBundleXMapper3D_H_HEADER_INCLUDED */

