/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkContourModelSetMapper3D_h
#define mitkContourModelSetMapper3D_h

#include "mitkCommon.h"
#include <MitkContourModelExports.h>

#include "mitkBaseRenderer.h"
#include "mitkVtkMapper.h"

#include "mitkContourModel.h"
#include "mitkContourModelSet.h"
#include "mitkContourModelToSurfaceFilter.h"

#include <vtkActor.h>
#include <vtkAssembly.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProp.h>
#include <vtkSmartPointer.h>
#include <vtkTubeFilter.h>

namespace mitk
{
  class MITKCONTOURMODEL_EXPORT ContourModelSetMapper3D : public VtkMapper
  {
  public:
    /** Standard class typedefs. */
    mitkClassMacro(ContourModelSetMapper3D, VtkMapper);

    /** Method for creation through the object factory. */
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

      const mitk::ContourModelSet *GetInput(void);

    /** \brief Checks whether this mapper needs to update itself and generate
   * data. */
    void Update(mitk::BaseRenderer *renderer) override;

    /*+++ methods of MITK-VTK rendering pipeline +++*/
    vtkProp *GetVtkProp(mitk::BaseRenderer *renderer) override;

    /*+++ END methods of MITK-VTK rendering pipeline +++*/

    class MITKCONTOURMODEL_EXPORT LocalStorage : public mitk::Mapper::BaseLocalStorage
    {
    public:
      /** \brief Assembly of contours. */
      vtkSmartPointer<vtkAssembly> m_Assembly;

      mitk::ContourModelToSurfaceFilter::Pointer m_contourToPolyData;

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
    ContourModelSetMapper3D();
    ~ContourModelSetMapper3D() override;

    void GenerateDataForRenderer(mitk::BaseRenderer *renderer) override;

    virtual vtkSmartPointer<vtkPolyData> CreateVtkPolyDataFromContour(mitk::ContourModel *inputContour,
                                                                      mitk::BaseRenderer *renderer);

    virtual void ApplyContourProperties(mitk::BaseRenderer *renderer);
    virtual void ApplyContourModelSetProperties(BaseRenderer *renderer);
  };
}
#endif
