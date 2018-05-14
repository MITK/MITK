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

#ifndef FiberBundleMAPPER2D_H_HEADER_INCLUDED
#define FiberBundleMAPPER2D_H_HEADER_INCLUDED

#include <mitkCommon.h>
#include <mitkBaseRenderer.h>
#include <mitkVtkMapper.h>
#include <mitkFiberBundle.h>
#include <vtkSmartPointer.h>

#define MITKFIBERBUNDLEMAPPER2D_POLYDATAMAPPER vtkOpenGLPolyDataMapper

class vtkActor;
class mitkBaseRenderer;
class MITKFIBERBUNDLEMAPPER2D_POLYDATAMAPPER;
class vtkCutter;
class vtkPlane;
class vtkPolyData;

namespace mitk {

struct IShaderRepository;

class FiberBundleMapper2D : public VtkMapper
{

public:
  mitkClassMacro(FiberBundleMapper2D, VtkMapper);
  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

  mitk::FiberBundle* GetInput();
  void Update(mitk::BaseRenderer * renderer) override;
  static void SetDefaultProperties(DataNode* node, BaseRenderer* renderer = nullptr, bool overwrite = false );
  vtkProp* GetVtkProp(mitk::BaseRenderer* renderer) override;

  class  FBXLocalStorage : public mitk::Mapper::BaseLocalStorage
  {
  public:
    vtkSmartPointer<vtkActor> m_Actor;
    vtkSmartPointer<MITKFIBERBUNDLEMAPPER2D_POLYDATAMAPPER> m_Mapper;
    itk::TimeStamp m_LastUpdateTime;
    FBXLocalStorage();

    ~FBXLocalStorage() override
    {
    }
  };

  /** \brief This member holds all three LocalStorages for the three 2D render windows. */
  mitk::LocalStorageHandler<FBXLocalStorage> m_LocalStorageHandler;

protected:
  FiberBundleMapper2D();
  ~FiberBundleMapper2D() override;

  /** Does the actual resampling, without rendering. */
  void GenerateDataForRenderer(mitk::BaseRenderer*) override;

  void UpdateShaderParameter(mitk::BaseRenderer*);

private:
  vtkSmartPointer<vtkLookupTable> m_lut;

  int     m_LineWidth;
};


}//end namespace

#endif
