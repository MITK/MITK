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

#ifndef __mitkLabelSetImageVtkMapper2D_H_
#define __mitkLabelSetImageVtkMapper2D_H_

//MITK
#include "mitkCommon.h"
#include "MitkMultilabelExports.h"

//MITK Rendering
#include "mitkBaseRenderer.h"
#include "mitkVtkMapper.h"
#include "mitkLabelSetImage.h"
#include "mitkExtractSliceFilter.h"

//VTK
#include <vtkSmartPointer.h>

class vtkActor;
class vtkPolyDataMapper;
class vtkPlaneSource;
class vtkImageData;
class vtkLookupTable;
class vtkImageReslice;
class vtkPoints;
class vtkMitkThickSlicesFilter;
class vtkPolyData;
class vtkMitkLevelWindowFilter;
class vtkNeverTranslucentTexture;

namespace mitk {

/** \brief Mapper to resample and display 2D slices of a 3D labelset image.
 *
 * Properties that can be set for labelset images and influence this mapper are:
 *
 *   - \b "labelset.contour.all": (BoolProperty) whether to show all labels as contours or not
 *   - \b "labelset.contour.active": (BoolProperty) whether to show only the active label as a contour or not
 *   - \b "labelset.contour.width": (FloatProperty) line width of the contour

 * The default properties are:

 *   - \b "labelset.contour.all", mitk::BoolProperty::New( false ), renderer, overwrite )
 *   - \b "labelset.contour.active", mitk::BoolProperty::New( false ), renderer, overwrite )
 *   - \b "labelset.contour.width", mitk::FloatProperty::New( 2.0 ), renderer, overwrite )

 * \ingroup Mapper
 */
class MITKMULTILABEL_EXPORT LabelSetImageVtkMapper2D : public VtkMapper
{

public:
  /** Standard class typedefs. */
  mitkClassMacro( LabelSetImageVtkMapper2D,VtkMapper );

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** \brief Get the Image to map */
  const mitk::Image *GetInput(void);

  /** \brief Checks whether this mapper needs to update itself and generate
   * data. */
  virtual void Update(mitk::BaseRenderer * renderer) override;

  //### methods of MITK-VTK rendering pipeline
  virtual vtkProp* GetVtkProp(mitk::BaseRenderer* renderer) override;
  //### end of methods of MITK-VTK rendering pipeline

  /** \brief Internal class holding the mapper, actor, etc. for each of the 3 2D render windows */
  /**
     * To render transveral, coronal, and sagittal, the mapper is called three times.
     * For performance reasons, the corresponding data for each view is saved in the
     * internal helper class LocalStorage. This allows rendering n views with just
     * 1 mitkMapper using n vtkMapper.
     * */
  class MITKMULTILABEL_EXPORT LocalStorage : public mitk::Mapper::BaseLocalStorage
  {
  public:

    vtkSmartPointer<vtkPropAssembly> m_Actors;

    std::vector< vtkSmartPointer<vtkActor> > m_LayerActorVector;
    std::vector< vtkSmartPointer<vtkPolyDataMapper> > m_LayerMapperVector;
    std::vector< vtkSmartPointer<vtkImageData> > m_ReslicedImageVector;
    std::vector< vtkSmartPointer<vtkNeverTranslucentTexture> > m_LayerTextureVector;

    vtkSmartPointer<vtkPolyData> m_EmptyPolyData;
    vtkSmartPointer<vtkPlaneSource> m_Plane;

    std::vector< mitk::ExtractSliceFilter::Pointer > m_ReslicerVector;

    vtkSmartPointer<vtkPolyData> m_OutlinePolyData;
    /** \brief An actor for the outline */
    vtkSmartPointer<vtkActor> m_OutlineActor;
    /** \brief An actor for the outline shadow*/
    vtkSmartPointer<vtkActor> m_OutlineShadowActor;
    /** \brief A mapper for the outline */
    vtkSmartPointer<vtkPolyDataMapper> m_OutlineMapper;

    /** \brief Timestamp of last update of stored data. */
    itk::TimeStamp m_LastDataUpdateTime;

    /** \brief Timestamp of last update of a property. */
    itk::TimeStamp m_LastPropertyUpdateTime;

    /** \brief mmPerPixel relation between pixel and mm. (World spacing).*/
    mitk::ScalarType* m_mmPerPixel;

    int m_NumberOfLayers;

    /** \brief This filter is used to apply the level window to Grayvalue and RBG(A) images. */
    //vtkSmartPointer<vtkMitkLevelWindowFilter> m_LevelWindowFilter;
    std::vector< vtkSmartPointer<vtkMitkLevelWindowFilter> > m_LevelWindowFilterVector;

    /** \brief Default constructor of the local storage. */
    LocalStorage();
    /** \brief Default deconstructor of the local storage. */
    ~LocalStorage();
  };

  /** \brief The LocalStorageHandler holds all (three) LocalStorages for the three 2D render windows. */
  mitk::LocalStorageHandler<LocalStorage> m_LSH;

  /** \brief Get the LocalStorage corresponding to the current renderer. */
  LocalStorage* GetLocalStorage(mitk::BaseRenderer* renderer);

  /** \brief Set the default properties for general image rendering. */
  static void SetDefaultProperties(mitk::DataNode* node, mitk::BaseRenderer* renderer = NULL, bool overwrite = false);

  /** \brief This method switches between different rendering modes (e.g. use a lookup table or a transfer function).
   * Detailed documentation about the modes can be found here: \link mitk::RenderingerModeProperty \endlink
   */
  void ApplyRenderingMode(mitk::BaseRenderer *renderer);

protected:
  /** \brief Transforms the actor to the actual position in 3D.
    *   \param renderer The current renderer corresponding to the render window.
    */
  void TransformActor(mitk::BaseRenderer* renderer);

  /** \brief Generates a plane according to the size of the resliced image in milimeters.
    *
    * \image html texturedPlane.png
    *
    * In VTK a vtkPlaneSource is defined through three points. The origin and two
    * points defining the axes of the plane (see VTK documentation). The origin is
    * set to (xMin; yMin; Z), where xMin and yMin are the minimal bounds of the
    * resliced image in space. Z is relevant for blending and the layer property.
    * The center of the plane (C) is also the center of the view plane (cf. the image above).
    *
    * \note For the standard MITK view with three 2D render windows showing three
    * different slices, three such planes are generated. All these planes are generated
    * in the XY-plane (even if they depict a YZ-slice of the volume).
    *
    */
  void GeneratePlane(mitk::BaseRenderer* renderer, double planeBounds[6]);

  /** \brief Generates a vtkPolyData object containing the outline of a given binary slice.
      \param renderer: Pointer to the renderer containing the needed information
      \note This code is based on code from the iil library.
      */
  vtkSmartPointer<vtkPolyData> CreateOutlinePolyData(mitk::BaseRenderer* renderer, vtkImageData* image, int pixelValue = 1);

  /** Default constructor */
  LabelSetImageVtkMapper2D();
  /** Default deconstructor */
  virtual ~LabelSetImageVtkMapper2D();

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
  virtual void GenerateDataForRenderer(mitk::BaseRenderer *renderer) override;

  /** \brief This method uses the vtkCamera clipping range and the layer property
    * to calcualte the depth of the object (e.g. image or contour). The depth is used
    * to keep the correct order for the final VTK rendering.*/
  float CalculateLayerDepth(mitk::BaseRenderer* renderer);

  /** \brief This method applies (or modifies) the lookuptable for all types of images.
   * \warning To use the lookup table, the property 'Lookup Table' must be set and a 'Image Rendering.Mode'
   * which uses the lookup table must be set.
*/
  void ApplyLookuptable(mitk::BaseRenderer* renderer, int layer);

  /** \brief This method applies a color transfer function.
   * Internally, a vtkColorTransferFunction is used. This is usefull for coloring continous
   * images (e.g. float)
   * \warning To use the color transfer function, the property 'Image Rendering.Transfer Function' must be set and a 'Image Rendering.Mode' which uses the color transfer function must be set.
*/
  void ApplyColorTransferFunction(mitk::BaseRenderer* renderer);

  /**
   * @brief ApplyLevelWindow Apply the level window for the given renderer.
   * \warning To use the level window, the property 'LevelWindow' must be set and a 'Image Rendering.Mode' which uses the level window must be set.
   * @param renderer Level window for which renderer?
   */
  void ApplyLevelWindow(mitk::BaseRenderer* renderer);

  /** \brief Set the color of the image/polydata */
  void ApplyColor( mitk::BaseRenderer* renderer, const mitk::Color& color );

  /** \brief Set the opacity of the actor. */
  void ApplyOpacity( mitk::BaseRenderer* renderer, int layer );

  /** \brief Set the contour of the actor and its width. */
  void ApplyContour(mitk::BaseRenderer* renderer, int layer);

  /**
    * \brief Calculates whether the given rendering geometry intersects the
    * given SlicedGeometry3D.
    *
    * This method checks if the given Geometry2D intersects the given
    * SlicedGeometry3D. It calculates the distance of the Geometry2D to all
    * 8 cornerpoints of the SlicedGeometry3D. If all distances have the same
    * sign (all positive or all negative) there is no intersection.
    * If the distances have different sign, there is an intersection.
    **/
  bool RenderingGeometryIntersectsImage( const PlaneGeometry* renderingGeometry, SlicedGeometry3D* imageGeometry );
};

} // namespace mitk

#endif // __mitkLabelSetImageVtkMapper2D_H_
