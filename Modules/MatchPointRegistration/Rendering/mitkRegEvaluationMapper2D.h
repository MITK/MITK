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

#ifndef MITK_REG_EVALUATION_MAPPER_2D_H
#define MITK_REG_EVALUATION_MAPPER_2D_H

//MatchPoint
#include <mapRegistration.h>
#include "mitkRegEvaluationObject.h"

//MITK
#include <mitkCommon.h>

//MITK Rendering
#include "mitkBaseRenderer.h"
#include "mitkVtkMapper.h"
#include "mitkExtractSliceFilter.h"

//VTK
#include <vtkSmartPointer.h>
#include <vtkPropAssembly.h>

//MITK
#include "MitkMatchPointRegistrationExports.h"

class vtkActor;
class vtkPolyDataMapper;
class vtkPlaneSource;
class vtkImageData;
class vtkLookupTable;
class vtkImageExtractComponents;
class vtkImageReslice;
class vtkImageChangeInformation;
class vtkPoints;
class vtkMitkThickSlicesFilter;
class vtkPolyData;
class vtkMitkApplyLevelWindowToRGBFilter;
class vtkMitkLevelWindowFilter;

namespace mitk {

/** \brief Mapper to resample and display 2D slices of registration evaluation visualization.
 * \ingroup Mapper
 */
class MITKMATCHPOINTREGISTRATION_EXPORT RegEvaluationMapper2D : public VtkMapper
{

public:
  /** Standard class typedefs. */
  mitkClassMacro( RegEvaluationMapper2D,VtkMapper );

  /** Method for creation through the object factory. */
  itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

  const mitk::DataNode* GetTargetNode(void);
  const mitk::DataNode* GetMovingNode(void);
  /** \brief Get the target image to map */
  const mitk::Image *GetTargetImage(void);
  /** \brief Get the moving image to map */
  const mitk::Image *GetMovingImage(void);
  /** \brief Get the target image to map */
  const mitk::MAPRegistrationWrapper *GetRegistration(void);

  /** \brief Checks whether this mapper needs to update itself and generate
   * data. */
  void Update(mitk::BaseRenderer * renderer) override;

  //### methods of MITK-VTK rendering pipeline
  vtkProp* GetVtkProp(mitk::BaseRenderer* renderer) override;
  //### end of methods of MITK-VTK rendering pipeline

  /** \brief Internal class holding the mapper, actor, etc. for each of the 3 2D render windows */
  /**
     * To render transveral, coronal, and sagittal, the mapper is called three times.
     * For performance reasons, the corresponding data for each view is saved in the
     * internal helper class LocalStorage. This allows rendering n views with just
     * 1 mitkMapper using n vtkMapper.
     * */
  class MITKMATCHPOINTREGISTRATION_EXPORT LocalStorage : public mitk::Mapper::BaseLocalStorage
  {
  public:
    /** \brief Actor of a 2D render window. */
    vtkSmartPointer<vtkActor> m_Actor;

    vtkSmartPointer<vtkPropAssembly> m_Actors;
    /** \brief Mapper of a 2D render window. */
    vtkSmartPointer<vtkPolyDataMapper> m_Mapper;
    /** \brief Current slice of a 2D render window.*/
    vtkSmartPointer<vtkImageData> m_EvaluationImage;

    /** \brief Empty vtkPolyData that is set when rendering geometry does not
      *   intersect the image geometry.
      *   \warning This member variable is set to nullptr,
      *   if no image geometry is inside the plane geometry
      *   of the respective render window. Any user of this
      *   slice has to check whether it is set to nullptr!
      */
    vtkSmartPointer<vtkPolyData> m_EmptyPolyData;
    /** \brief Plane on which the slice is rendered as texture. */
    vtkSmartPointer<vtkPlaneSource> m_Plane;
    /** \brief The texture which is used to render the current slice. */
    vtkSmartPointer<vtkTexture> m_Texture;
    /** \brief The lookuptables for colors and level window */
    vtkSmartPointer<vtkLookupTable> m_ColorLookupTable;
    vtkSmartPointer<vtkLookupTable> m_DefaultLookupTable;
    /** \brief The actual reslicer (one per renderer) */
    mitk::ExtractSliceFilter::Pointer m_Reslicer;

    /** part of the target image that is relevant for the rendering*/
    mitk::Image::Pointer m_slicedTargetImage;
    /** part of the moving image mapped into the slicedTargetImage
     geometry*/
    mitk::Image::Pointer m_slicedMappedImage;

    /** \brief Timestamp of last update of stored data. */
    itk::TimeStamp m_LastUpdateTime;

    /** \brief mmPerPixel relation between pixel and mm. (World spacing).*/
    mitk::ScalarType* m_mmPerPixel;

    /** \brief This filter is used to apply the level window to target image. */
    vtkSmartPointer<vtkMitkLevelWindowFilter> m_TargetLevelWindowFilter;

    /** \brief This filter is used to apply the level window to moving image. */
    vtkSmartPointer<vtkMitkLevelWindowFilter> m_MappedLevelWindowFilter;

    vtkSmartPointer<vtkImageExtractComponents> m_TargetExtractFilter;
    vtkSmartPointer<vtkImageExtractComponents> m_MappedExtractFilter;

    /** \brief Default constructor of the local storage. */
    LocalStorage();
    /** \brief Default deconstructor of the local storage. */
    ~LocalStorage() override;
  };

  /** \brief The LocalStorageHandler holds all (three) LocalStorages for the three 2D render windows. */
  mitk::LocalStorageHandler<LocalStorage> m_LSH;

  /** \brief Get the LocalStorage corresponding to the current renderer. */
  LocalStorage* GetLocalStorage(mitk::BaseRenderer* renderer);

  /** \brief Set the default properties for general image rendering. */
  static void SetDefaultProperties(mitk::DataNode* node, mitk::BaseRenderer* renderer = nullptr, bool overwrite = false);

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

  /** Default constructor */
  RegEvaluationMapper2D();
  /** Default deconstructor */
  ~RegEvaluationMapper2D() override;

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

  void PrepareContour( mitk::DataNode* datanode, LocalStorage * localStorage );

  void PrepareDifference( LocalStorage * localStorage );

  void PrepareWipe(mitk::DataNode* datanode, LocalStorage * localStorage, const Point2D& currentIndex2D);

  void PrepareCheckerBoard( mitk::DataNode* datanode, LocalStorage * localStorage );

  void PrepareColorBlend( LocalStorage * localStorage );

  void PrepareBlend( mitk::DataNode* datanode, LocalStorage * localStorage );

  /** \brief This method uses the vtkCamera clipping range and the layer property
    * to calcualte the depth of the object (e.g. image or contour). The depth is used
    * to keep the correct order for the final VTK rendering.*/
  float CalculateLayerDepth(mitk::BaseRenderer* renderer);

  /** \brief This method applies (or modifies) the lookuptable for all types of images.
   * \warning To use the lookup table, the property 'Lookup Table' must be set and a 'Image Rendering.Mode'
   * which uses the lookup table must be set.
*/
  void ApplyLookuptable(mitk::BaseRenderer* renderer, const mitk::DataNode* dataNode, vtkMitkLevelWindowFilter* levelFilter);

  /**
   * @brief ApplyLevelWindow Apply the level window for the given renderer.
   * \warning To use the level window, the property 'LevelWindow' must be set and a 'Image Rendering.Mode' which uses the level window must be set.
   * @param renderer Level window for which renderer?
   */
  void ApplyLevelWindow(mitk::BaseRenderer *renderer, const mitk::DataNode* dataNode, vtkMitkLevelWindowFilter* levelFilter);

  /** \brief Set the opacity of the actor. */
  void ApplyOpacity( mitk::BaseRenderer* renderer );

  /**
    * \brief Calculates whether the given rendering geometry intersects the
    * given SlicedGeometry3D.
    *
    * This method checks if the given PlaneGeometry intersects the given
    * SlicedGeometry3D. It calculates the distance of the PlaneGeometry to all
    * 8 cornerpoints of the SlicedGeometry3D. If all distances have the same
    * sign (all positive or all negative) there is no intersection.
    * If the distances have different sign, there is an intersection.
    **/
  bool RenderingGeometryIntersectsImage( const PlaneGeometry* renderingGeometry, SlicedGeometry3D* imageGeometry );
};

} // namespace mitk

#endif /* MITKRegEvaluationMapper2D_H_HEADER_INCLUDED_C10E906E */
