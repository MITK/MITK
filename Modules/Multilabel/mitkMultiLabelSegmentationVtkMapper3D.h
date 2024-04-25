/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkMultiLabelSegmentationVtkMapper3D_h
#define mitkMultiLabelSegmentationVtkMapper3D_h

// MITK
#include "MitkMultilabelExports.h"
#include "mitkCommon.h"

// MITK Rendering
#include "mitkBaseRenderer.h"
#include "mitkExtractSliceFilter.h"
#include "mitkLabelSetImage.h"
#include "mitkVtkMapper.h"

// VTK
#include <vtkSmartPointer.h>

class vtkPolyDataMapper;
class vtkImageData;
class vtkLookupTable;
class vtkVolumeProperty;
class vtkVolume;
class vtkSmartVolumeMapper;

namespace mitk
{

  /** \brief Mapper to resample and display 2D slices of a 3D labelset image.
   *
   * Properties that can be set for labelset images and influence this mapper are:
   *
   *   - \b "labelset.contour.active": (BoolProperty) whether to show only the active label as a contour or not
   *   - \b "labelset.contour.width": (FloatProperty) line width of the contour

   * The default properties are:

   *   - \b "labelset.contour.active", mitk::BoolProperty::New( true ), renderer, overwrite )
   *   - \b "labelset.contour.width", mitk::FloatProperty::New( 2.0 ), renderer, overwrite )

   * \ingroup Mapper
   */
  class MITKMULTILABEL_EXPORT MultiLabelSegmentationVtkMapper3D : public VtkMapper
  {
  public:
    /** Standard class typedefs. */
    mitkClassMacro(MultiLabelSegmentationVtkMapper3D, VtkMapper);

    /** Method for creation through the object factory. */
    itkNewMacro(Self);

    /** \brief Checks whether this mapper needs to update itself and generate
     * data. */
    void Update(mitk::BaseRenderer *renderer) override;

    //### methods of MITK-VTK rendering pipeline
    vtkProp *GetVtkProp(mitk::BaseRenderer *renderer) override;
    //### end of methods of MITK-VTK rendering pipeline

    /** \brief Internal class holding the mapper, actor, etc. for each of the 3 2D render windows */
    /**
       * To render axial, coronal, and sagittal, the mapper is called three times.
       * For performance reasons, the corresponding data for each view is saved in the
       * internal helper class LocalStorage. This allows rendering n views with just
       * 1 mitkMapper using n vtkMapper.
       * */
    class MITKMULTILABEL_EXPORT LocalStorage : public mitk::Mapper::BaseLocalStorage
    {
    public:
      vtkSmartPointer<vtkPropAssembly> m_Actors;

      std::vector<vtkSmartPointer<vtkSmartVolumeMapper>> m_LayerVolumeMappers;
      std::vector<vtkSmartPointer<vtkImageData>> m_LayerImages;
      std::vector<vtkSmartPointer<vtkVolume>> m_LayerVolumes;

      std::vector <vtkSmartPointer<vtkColorTransferFunction> > m_TransferFunctions;
      std::vector <vtkSmartPointer<vtkPiecewiseFunction> > m_OpacityTransferFunctions;

      /** Vector containing the pointer of the currently used group images.
       * IMPORTANT: This member must not be used to access any data.
       * Its purpose is to allow checking if the order of the groups has changed
       * in order to adapt the pipe line accordingly*/
      std::vector<const Image*> m_GroupImageIDs;

      /** \brief Timestamp of last update of stored data. */
      itk::TimeStamp m_LastDataUpdateTime;
      /** \brief Timestamp of last update of a property. */
      itk::TimeStamp m_LastPropertyUpdateTime;

      /** look up table for label colors. */
      mitk::LookupTable::Pointer m_LabelLookupTable;

      unsigned int m_NumberOfGroups;

      /** \brief Default constructor of the local storage. */
      LocalStorage();
      /** \brief Default destructor of the local storage. */
      ~LocalStorage() override;
    };

    /** \brief The LocalStorageHandler holds all (three) LocalStorages for the three 2D render windows. */
    mitk::LocalStorageHandler<LocalStorage> m_LSH;

    /** \brief Get the LocalStorage corresponding to the current renderer. */
    LocalStorage *GetLocalStorage(mitk::BaseRenderer *renderer);

    /** \brief Set the default properties for general image rendering. */
    static void SetDefaultProperties(mitk::DataNode *node, mitk::BaseRenderer *renderer = nullptr, bool overwrite = false);

  protected:
    /** Default constructor */
    MultiLabelSegmentationVtkMapper3D();
    /** Default deconstructor */
    ~MultiLabelSegmentationVtkMapper3D() override;

    /** \brief Does the actual resampling, without rendering the image yet.
      * All the data is generated inside this method. The vtkProp (or Actor)
      * is filled with content (i.e. the resliced image).
      *
      * After generation, a 4x4 transformation matrix(t) of the current slice is obtained
      * from the vtkResliceImage object via GetReslicesAxis(). This matrix is
      * applied to each textured plane (actor->SetUserTransform(t)) to transform everything
      * to the actual 3D position (cf. the following image).
      *
      * \image html cameraPositioning3D.png
      *
      */
    void GenerateDataForRenderer(mitk::BaseRenderer *renderer) override;

    bool GenerateVolumeMapping(mitk::BaseRenderer* renderer, const std::vector<mitk::LabelSetImage::GroupIndexType>& outdatedGroupIDs);

    /** \brief Generates the look up table that should be used.
      */
    void GenerateLookupTable(mitk::BaseRenderer* renderer);
  };

} // namespace mitk

#endif
