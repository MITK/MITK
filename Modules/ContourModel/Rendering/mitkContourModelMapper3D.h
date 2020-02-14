/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef _MITK_CONTOURMODEL_MAPPER_3D_H_
#define _MITK_CONTOURMODEL_MAPPER_3D_H_

#include "mitkCommon.h"
#include <MitkContourModelExports.h>

#include "mitkBaseRenderer.h"
#include "mitkVtkMapper.h"

#include "mitkContourModel.h"
//#include "mitkContourModelToVtkPolyDataFilter.h"

#include <vtkActor.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProp.h>
#include <vtkSmartPointer.h>
#include <vtkTubeFilter.h>

namespace mitk
{
  class MITKCONTOURMODEL_EXPORT ContourModelMapper3D : public VtkMapper
  {
  public:
    /** Standard class typedefs. */
    mitkClassMacro(ContourModelMapper3D, VtkMapper);

    /** Method for creation through the object factory. */
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

      const mitk::ContourModel *GetInput(void);

    /** \brief Checks whether this mapper needs to update itself and generate
   * data. */
    void Update(mitk::BaseRenderer *renderer) override;

    /*+++ methods of MITK-VTK rendering pipeline +++*/
    vtkProp *GetVtkProp(mitk::BaseRenderer *renderer) override;

    /*+++ END methods of MITK-VTK rendering pipeline +++*/

    class MITKCONTOURMODEL_EXPORT LocalStorage : public mitk::Mapper::BaseLocalStorage
    {
    public:
      /** \brief Actor of a 2D render window. */
      vtkSmartPointer<vtkActor> m_Actor;
      /** \brief Mapper of a 2D render window. */
      vtkSmartPointer<vtkPolyDataMapper> m_Mapper;

      vtkSmartPointer<vtkTubeFilter> m_TubeFilter;

      // mitk::ContourModelToVtkPolyDataFilter::Pointer m_contourToPolyData;

      vtkSmartPointer<vtkPolyData> m_OutlinePolyData;

      /** \brief Timestamp of last update of stored data. */
      itk::TimeStamp m_LastUpdateTime;

      /** \brief Default constructor of the local storage. */
      LocalStorage();
      /** \brief Default deconstructor of the local storage. */
      ~LocalStorage() override {}
    };

    /** \brief The LocalStorageHandler holds all (three) LocalStorages for the three 2D render windows. */
    mitk::LocalStorageHandler<LocalStorage> m_LSH;

    /** \brief Get the LocalStorage corresponding to the current renderer. */
    LocalStorage *GetLocalStorage(mitk::BaseRenderer *renderer);

    /** \brief Set the default properties for general image rendering. */
    static void SetDefaultProperties(mitk::DataNode *node, mitk::BaseRenderer *renderer = nullptr, bool overwrite = false);

  protected:
    ContourModelMapper3D();
    ~ContourModelMapper3D() override;

    void GenerateDataForRenderer(mitk::BaseRenderer *renderer) override;

    virtual vtkSmartPointer<vtkPolyData> CreateVtkPolyDataFromContour(mitk::ContourModel *inputContour);

    virtual void ApplyContourProperties(mitk::BaseRenderer *renderer);
  };
}
#endif
