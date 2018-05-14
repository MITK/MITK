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


#ifndef FiberBundleThreadMonitorMapper3D_H_HEADER_INCLUDED
#define FiberBundleThreadMonitorMapper3D_H_HEADER_INCLUDED

//#include <mitkBaseData.h> //?? necessary
#include <MitkFiberTrackingExports.h>
#include <mitkVtkMapper.h>
#include <mitkFiberBundleThreadMonitor.h>

#include <vtkSmartPointer.h>
#include <vtkPolyDataMapper.h>
#include <vtkTextActor.h>



class vtkPropAssembly;




namespace mitk {

  //##Documentation
  //## @brief Mapper for FiberBundle
  //## @ingroup Mapper

  class MITKFIBERTRACKING_EXPORT FiberBundleThreadMonitorMapper3D : public VtkMapper
  {
  public:

    mitkClassMacro(FiberBundleThreadMonitorMapper3D, VtkMapper);
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    //========== essential implementation for 3D mapper ========
    const FiberBundleThreadMonitor* GetInput();
    virtual vtkProp *GetVtkProp(mitk::BaseRenderer *renderer); //looks like depricated.. should be replaced bz GetViewProp()
    static void SetDefaultProperties(DataNode* node, BaseRenderer* renderer = nullptr, bool overwrite = false );
    virtual void ApplyProperties(mitk::BaseRenderer* renderer);
    virtual void GenerateDataForRenderer(mitk::BaseRenderer* renderer);
    //=========================================================

  protected:

    FiberBundleThreadMonitorMapper3D();
    virtual ~FiberBundleThreadMonitorMapper3D();

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




#endif /* FiberBundleMapper3D_H_HEADER_INCLUDED */

