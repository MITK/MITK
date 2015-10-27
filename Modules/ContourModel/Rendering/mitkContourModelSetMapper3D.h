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

#ifndef _MITK_CONTOURMODELSET_MAPPER_3D_H_
#define _MITK_CONTOURMODELSET_MAPPER_3D_H_


#include "mitkCommon.h"
#include <MitkContourModelExports.h>

#include "mitkBaseRenderer.h"
#include "mitkVtkMapper.h"

#include "mitkContourModel.h"
#include "mitkContourModelSet.h"
#include "mitkContourModelToSurfaceFilter.h"

#include <vtkSmartPointer.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkAssembly.h>
#include <vtkProp.h>
#include <vtkPolyData.h>
#include <vtkTubeFilter.h>


namespace mitk {

  class MITKCONTOURMODEL_EXPORT ContourModelSetMapper3D : public VtkMapper
  {
      public:
    /** Standard class typedefs. */
    mitkClassMacro( ContourModelSetMapper3D,VtkMapper );

    /** Method for creation through the object factory. */
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    const mitk::ContourModelSet* GetInput(void);

    /** \brief Checks whether this mapper needs to update itself and generate
   * data. */
    virtual void Update(mitk::BaseRenderer * renderer) override;


        /*+++ methods of MITK-VTK rendering pipeline +++*/
    virtual vtkProp* GetVtkProp(mitk::BaseRenderer* renderer) override;

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
      ~LocalStorage()
      {
      }
    };


    /** \brief The LocalStorageHandler holds all (three) LocalStorages for the three 2D render windows. */
    mitk::LocalStorageHandler<LocalStorage> m_LSH;

    /** \brief Get the LocalStorage corresponding to the current renderer. */
    LocalStorage* GetLocalStorage(mitk::BaseRenderer* renderer);

        /** \brief Set the default properties for general image rendering. */
    static void SetDefaultProperties(mitk::DataNode* node, mitk::BaseRenderer* renderer = NULL, bool overwrite = false);


  protected:
    ContourModelSetMapper3D();
    virtual ~ContourModelSetMapper3D();

    void GenerateDataForRenderer( mitk::BaseRenderer *renderer ) override;

    virtual vtkSmartPointer<vtkPolyData> CreateVtkPolyDataFromContour(mitk::ContourModel* inputContour,mitk::BaseRenderer* renderer);

    virtual void ApplyContourProperties(mitk::BaseRenderer* renderer);
    virtual void ApplyContourModelSetProperties(BaseRenderer *renderer);
  };
}
#endif
