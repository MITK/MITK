/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkContourModelSetMapper3D_h
#define mitkContourModelSetMapper3D_h

#include <mitkCommon.h>
#include <MitkContourModelExports.h>

#include <mitkBaseRenderer.h>
#include <mitkVtkMapper.h>

#include <mitkContourModel.h>
#include <mitkContourModelSet.h>
#include <mitkContourModelToSurfaceFilter.h>

#include <vtkActor.h>
#include <vtkAssembly.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProp.h>
#include <vtkSmartPointer.h>
#include <vtkTubeFilter.h>

namespace mitk
{
  /** \brief VTK-based mapper to display a mitk::ContourModelSet in a 3D render window.
   *
   * Creates VTK polydata from each ContourModel in the set and assembles them
   * into a single vtkAssembly for 3D rendering with tube filtering.
   *
   * \sa ContourModelSet, ContourModelMapper3D, ContourModelSetMapper2D
   * \ingroup MitkContourModelModule
   */
  class MITKCONTOURMODEL_EXPORT ContourModelSetMapper3D : public VtkMapper
  {
  public:
    mitkClassMacro(ContourModelSetMapper3D, VtkMapper);

    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

      /** \brief Return the input ContourModelSet data object.
       * \return Const pointer to the ContourModelSet associated with this mapper's data node.
       */
      const mitk::ContourModelSet *GetInput(void);

    /** \brief Check whether this mapper needs to regenerate its rendering data.
     * \param[in] renderer The renderer to check for update necessity.
     */
    void Update(mitk::BaseRenderer *renderer) override;

    /** \brief Return the top-level VTK prop for the given renderer.
     * \param[in] renderer The renderer for which to retrieve the VTK prop.
     * \return The vtkAssembly representing all contours in the 3D render window.
     */
    vtkProp *GetVtkProp(mitk::BaseRenderer *renderer) override;

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

    /** \brief Handler that manages per-renderer LocalStorage instances. */
    mitk::LocalStorageHandler<LocalStorage> m_LSH;

    /** \brief Retrieve the LocalStorage for the given renderer.
     * \param[in] renderer The renderer whose local storage is requested.
     * \return Pointer to the corresponding LocalStorage instance.
     */
    LocalStorage *GetLocalStorage(mitk::BaseRenderer *renderer);

    /** \brief Set default rendering properties for ContourModelSet 3D display.
     * \param[in] node The data node to configure.
     * \param[in] renderer The renderer context. If nullptr, properties are set globally.
     * \param[in] overwrite If true, existing properties are overwritten.
     */
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
