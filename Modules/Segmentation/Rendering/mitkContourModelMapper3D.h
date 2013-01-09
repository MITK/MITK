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

#ifndef _MITK_CONTOURMODEL_MAPPER_3D_H_
#define _MITK_CONTOURMODEL_MAPPER_3D_H_


#include "mitkCommon.h"
#include "SegmentationExports.h"

#include "mitkBaseRenderer.h"
#include "mitkVtkMapper2D.h"

#include "mitkContourModel.h"
//#include "mitkContourModelToVtkPolyDataFilter.h"

#include <vtkSmartPointer.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkProp.h>
#include <vtkPolyData.h>
#include <vtkTubeFilter.h>


namespace mitk {

  class Segmentation_EXPORT ContourModelMapper3D : public VtkMapper2D
  {
      public:
    /** Standard class typedefs. */
    mitkClassMacro( ContourModelMapper3D,VtkMapper2D );

    /** Method for creation through the object factory. */
    itkNewMacro(Self);

    const mitk::ContourModel* GetInput(void);

    /** \brief Checks whether this mapper needs to update itself and generate
   * data. */
    virtual void Update(mitk::BaseRenderer * renderer);


        /*+++ methods of MITK-VTK rendering pipeline +++*/
    virtual vtkProp* GetVtkProp(mitk::BaseRenderer* renderer);

    virtual void MitkRenderOverlay(BaseRenderer* renderer);
    virtual void MitkRenderOpaqueGeometry(BaseRenderer* renderer);
    virtual void MitkRenderTranslucentGeometry(BaseRenderer* renderer);
    virtual void MitkRenderVolumetricGeometry(BaseRenderer* renderer);
    /*+++ END methods of MITK-VTK rendering pipeline +++*/


    class Segmentation_EXPORT LocalStorage : public mitk::Mapper::BaseLocalStorage
    {
    public:

      /** \brief Actor of a 2D render window. */
      vtkSmartPointer<vtkActor> m_Actor;
      /** \brief Mapper of a 2D render window. */
      vtkSmartPointer<vtkPolyDataMapper> m_Mapper;

      vtkSmartPointer<vtkTubeFilter> m_TubeFilter;

      //mitk::ContourModelToVtkPolyDataFilter::Pointer m_contourToPolyData;

      vtkSmartPointer<vtkPolyData> m_OutlinePolyData;

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
    mitk::Mapper::LocalStorageHandler<LocalStorage> m_LSH;

    /** \brief Get the LocalStorage corresponding to the current renderer. */
    LocalStorage* GetLocalStorage(mitk::BaseRenderer* renderer);

        /** \brief Set the default properties for general image rendering. */
    static void SetDefaultProperties(mitk::DataNode* node, mitk::BaseRenderer* renderer = NULL, bool overwrite = false);


  protected:
    ContourModelMapper3D();
    virtual ~ContourModelMapper3D();

    void GenerateDataForRenderer( mitk::BaseRenderer *renderer );

    virtual vtkSmartPointer<vtkPolyData> CreateVtkPolyDataFromContour(mitk::ContourModel* inputContour);

    virtual void ApplyContourProperties(mitk::BaseRenderer* renderer);
  };
}
#endif
