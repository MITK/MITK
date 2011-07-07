/*=========================================================================
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef MITKIMAGEVTKMAPPER2D_H_HEADER_INCLUDED_C10E906E
#define MITKIMAGEVTKMAPPER2D_H_HEADER_INCLUDED_C10E906E

//MITK
#include <mitkCommon.h>

//MITK Rendering
#include "mitkBaseRenderer.h"
#include "mitkVtkMapper2D.h"

//VTK
#include <vtkSmartPointer.h>

class vtkActor;
class vtkPolyDataMapper;
class vtkPlaneSource;
class vtkImageData;
class vtkLookupTable;
class vtkImageReslice;
class vtkImageChangeInformation;
class vtkPoints;
class vtkMitkThickSlicesFilter;
class vtkPolyData;

namespace mitk {

  /** \brief Mapper to resample and display 2D slices of a 3D image.
 * 
 * The following image gives a brief overview of the mapping and the involved parts.
 *
 * \image html imageVtkMapper2Darchitecture.png
 *
 * At first, the image is resliced by means of vtkImageReslice. The volume image
 * serves as input in addition to spatial placement of the slice and a few other
 * properties like thick slices. This code was already present in the old version
 * (mitkImageMapperGL2D).
 *
 * Next, the obtained slice (m_ReslicedImage) is used to create a texture
 * (m_Texture) and a plane onto which the texture is rendered (m_Plane). For
 * mapping purposes, a vtkPolyDataMapper (m_Mapper) is utilized. Orthographic
 * projection is applied to create the effect of a 2D image. The mapper and the
 * texture are assigned to the actor (m_Actor) which is passed to the VTK rendering
 * pipeline via the method GetVtkProp().
 *
 * In order to transform the textured plane to the correct position in space, the
 * same transformation as used for reslicing is applied to both, the camera and the
 * vtkActor. The camera position is also influenced by the mitkDisplayGeometry
 * parameters to facilitate zooming and panning. All important steps are explained
 * in following more detailedly. The resulting 2D image (by reslicing the
 * underlying 3D input image appropriately) can either be directly rendered
 * in a 2D view or just be calculated to be used later on by another
 * rendering entity, e.g. in texture mapping in a 3D view.
 *
 * Properties that can be set for images and influence the imageMapper2D are:
 *
 *   - \b "modality": (mitkModalityProperty) Modality of the image
 *   - \b "opacity": (FloatProperty) Opacity of the image
 *   - \b "color": (ColorProperty) Color of the image
 *   - \b "use color": (BoolProperty) Use the color of the image or not
 *   - \b "binary": (BoolProperty) is the image a binary image or not
 *   - \b "outline binary": (BoolProperty) show outline of the image or not
 *   - \b "texture interpolation": (BoolProperty) texture interpolation of the image
 *   - \b "reslice interpolation": (VtkResliceInterpolationProperty) reslice interpolation of the image
 *   - \b "in plane resample extent by geometry": (BoolProperty) Do it or not
 *   - \b "bounding box": (BoolProperty) Is the Bounding Box of the image shown or not
 *   - \b "layer": (IntProperty) Layer of the image
 *   - \b "volume annotation color": (ColorProperty) color of the volume annotation
 *   - \b "volume annotation unit": (StringProperty) annotation unit as string (does not implicit convert the unit!) 
          unit is ml/cm3

 * The default properties are:

 *   - \b "opacity", mitk::FloatProperty::New(0.3f), renderer, overwrite )
 *   - \b "color", ColorProperty::New(1.0,0.0,0.0), renderer, overwrite )
 *   - \b "use color", mitk::BoolProperty::New( true ), renderer, overwrite )
 *   - \b "binary", mitk::BoolProperty::New( true ), renderer, overwrite )
 *   - \b "outline binary", mitk::BoolProperty::New( false ), renderer, overwrite )
 *   - \b "texture interpolation", mitk::BoolProperty::New( mitk::DataNodeFactory::m_TextureInterpolationActive ) )
 *   - \b "reslice interpolation", mitk::VtkResliceInterpolationProperty::New() )
 *   - \b "in plane resample extent by geometry", mitk::BoolProperty::New( false ) )
 *   - \b "bounding box", mitk::BoolProperty::New( false ) )
 *   - \b "layer", mitk::IntProperty::New(10), renderer, overwrite)

 * If the modality-property is set for an image, the mapper uses modality-specific default properties,
 * e.g. color maps, if they are defined.

 * \ingroup Mapper
 */
  class MITK_CORE_EXPORT ImageVtkMapper2D : public VtkMapper2D
  {

  public:

    /** Standard class typedefs. */
    mitkClassMacro( ImageVtkMapper2D,VtkMapper2D );

    /** Method for creation through the object factory. */
    itkNewMacro(Self);

    /** \brief Get the Image to map */
    const mitk::Image *GetInput(void);

    /** \brief Checks whether this mapper needs to update itself and generate
   * data. */
    virtual void Update(mitk::BaseRenderer * renderer);

    /** \brief Apply all properties to the vtkActor (e.g. color, opacity, binary image handling, etc.).*/
    virtual void ApplyProperties(mitk::BaseRenderer* renderer, ScalarType mmPerPixel[2]);

    //### methods of MITK-VTK rendering pipeline
    virtual vtkProp* GetVtkProp(mitk::BaseRenderer* renderer);

    virtual void MitkRenderOverlay(BaseRenderer* renderer);
    virtual void MitkRenderOpaqueGeometry(BaseRenderer* renderer);
    virtual void MitkRenderTranslucentGeometry(BaseRenderer* renderer);
    virtual void MitkRenderVolumetricGeometry(BaseRenderer* renderer);
    //### end of methods of MITK-VTK rendering pipeline


    /** \brief Internal class holding the mapper, actor, etc. for each of the 3 2D render windows */
    /**
     * To render transveral, coronal, and sagittal, the mapper is called three times.
     * For performance reasons, the corresponding data for each view is saved in the
     * internal helper class LocalStorage. This allows rendering n views with just
     * 1 mitkMapper using n vtkMapper.
     * */
    class MITK_CORE_EXPORT LocalStorage : public mitk::Mapper::BaseLocalStorage
    {
    public:
      /** \brief Actor of a 2D render window. */
      vtkSmartPointer<vtkActor> m_Actor;
      /** \brief Mapper of a 2D render window. */
      vtkSmartPointer<vtkPolyDataMapper> m_Mapper;
      /** \brief Current slice of a 2D render window. */
      vtkSmartPointer<vtkImageData> m_ReslicedImage;
      /** \brief Plane on which the slice is rendered as texture. */
      vtkSmartPointer<vtkPlaneSource> m_Plane;
      /** \brief The texture which is used to render the current slice. */
      vtkSmartPointer<vtkTexture> m_Texture;
      /** \brief The lookuptable for colors and level window */
      vtkSmartPointer<vtkLookupTable> m_LookupTable;
      /** \brief The actual reslicer (one per renderer) */
      vtkSmartPointer<vtkImageReslice> m_Reslicer;
      /** \brief Thickslices post filtering.  */
      vtkSmartPointer<vtkMitkThickSlicesFilter> m_TSFilter;
      /** \brief Using unit spacing for resampling makes life easier TODO improve docu ...*/
      vtkSmartPointer<vtkImageChangeInformation> m_UnitSpacingImageFilter;      
      /** \brief PolyData object containg all lines/points needed for outlining the contour.
          This container is used to save a computed contour for the next rendering execution.
          For instance, if you zoom or pann, there is no need to recompute the contour. */
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

    /** \brief This member holds all (three) LocalStorages for the three 2D render windows. */
    mitk::Mapper::LocalStorageHandler<LocalStorage> m_LSH;

    /** \brief Set the default properties for general image rendering. */
    static void SetDefaultProperties(mitk::DataNode* node, mitk::BaseRenderer* renderer = NULL, bool overwrite = false);

  protected:
    /** \brief Generates a plane according to the size of the resliced image in mm.
    *
    * \image html texturedPlane.png
    *
    * In VTK a vtkPlaneSource is defined through three points. The origin and two
    * points defining the axes of the plane (see VTK documentation). The origin is
    * set to (xMin; yMin; 0), where xMin and yMin are the minimal bounds of the
    * resliced image in space. The center of the plane (C) is also the center of
    * the view plane (cf. the image above).
    *
    * \note: For the standard MITK view with three 2D render windows showing three
    * different slices, three such planes are generated. All these planes are generated
    * in the XY-plane (even if they depict a YZ-slice of the volume).
    *
    */
    void GeneratePlane(mitk::BaseRenderer* renderer, vtkFloatingPointType planeBounds[6]);

    /** \brief This method sets up the camera on the actor/image.
    * The view is centralized, zooming and panning of VTK are called inside.
    * TODO: this method should be moved to VtkMitkRenderProp or to the mitkVtkPropRenderer.
    *
    * \image html ImageMapperdisplayGeometry.png
    *
    * Similar to the textured plane (cf. void GeneratePlane(mitk::BaseRenderer* renderer,
    * vtkFloatingPointType planeBounds[6])), the mitkDisplayGeometry defines a view plane (or
    * projection plane). This plane is used to set the camera parameters. The view plane
    * center (VC) is important for camera positioning (cf. the image above).
    *
    * The following figure shows the combination of the textured plane and the view plane.
    *
    * \image html cameraPositioning.png
    *
    * The view plane center (VC) is the center of the textured plane (C) and the focal point
    * (FP) at the same time. The FP defines into which direction the camera is viewing. Since
    * the textured plane is always in the XY-plane and orthographic projection is applied, the
    * distance between camera and plane is theoretically irrelevant (because in the orthographic
    * projection the center of projection is at infinity and the size of objects depends only on
    * a scaling parameter).As a consequence, the direction of projection (DOP) is (0; 0; -1).
    * The camera up vector is always defined as (0; 1; 0).
    *
    * \warning Due to a VTK clipping bug the distance between textured plane and camera is really huge.
    * Otherwise, VTK would clip off some slices. Same applies for the clipping range size.
    *
    * \note The camera position is defined through the mitkDisplayGeometry and not through
    * the center of the textured plane. This facilitates zooming and panning, because the display
    * geometry changes and the textured plane does not.
    *
    * \image html scaling.png
    *
    * The textured plane is scaled to fill the render window via
    * camera->SetParallelScale( imageHeightInMM / 2). In the orthographic projection all extends,
    * angles and sizes should be preserved. Therefore, the image is scaled with one parameter in
    * X and Y direction. This parameter defines the size of the rendered image. A higher value will
    * result in smaller images. In order to render just the whole image, the scale is set to half of
    * the image height in wolrdcoordinates (cf. the picture above).
    *
    * For zooming purposes, a factor is computed as follows:
    * factor = image height / display height (in worldcoordinates).
    * When the display geometry gets smaller (zoom in), the factor becoomes bigger. When the display
    * geometry gets bigger (zoom out), the factor becoomes smaller. The used VTK method
    * camera->Zoom( factor ) also works with an inverse scale.
    */
    void AdjustCamera(mitk::BaseRenderer* renderer);

    /** \brief Generates a vtkPolyData object containing the outline of a given binary slice.
      \param binarySlice - The binary image slice. (Volumes are not supported.)
      \param mmPerPixel - Spacing of the binary image slice. Hence it's 2D, only in x/y-direction.
      \note This code has been taken from the deprecated library iil.
      */
    vtkSmartPointer<vtkPolyData> CreateOutlinePolyData(vtkSmartPointer<vtkImageData> binarySlice, ScalarType mmPerPixel[2]);

    /** Default constructor */
    ImageVtkMapper2D();
    /** Default deconstructor */
    virtual ~ImageVtkMapper2D();

    /** \brief Does the actual resampling, without rendering the image yet.
    * All the data is generated inside this method. The vtkProp (or Actor)
    * is filled with content (i.e. the resliced image).
    *
    * After generation, a 4x4 transformation matrix(t) of the current slice is obtained
    * from the vtkResliceImage object via GetReslicesAxis(). This matrix is
    * applied to each camera (cam->ApplyTransformation(t)) and to each textured
    * plane (actor->SetUserTransform(t)) to transform everything
    * to the actual 3D position (cf. the following image).
    *
    * \image html cameraPositioning3D.png
    *
    */
    virtual void GenerateData(mitk::BaseRenderer *renderer);

    /** \brief Internal helper method for intersection testing used only in CalculateClippedPlaneBounds() */
    bool LineIntersectZero( vtkPoints *points, int p1, int p2,
                            vtkFloatingPointType *bounds );

    /** \brief Calculate the bounding box of the resliced image. This is necessary for
        arbitrary rotated planes in an image volume. A rotated plane (e.g. in swivil mode)
        will have a new bounding box, which needs to be calculated. */
    bool CalculateClippedPlaneBounds( const Geometry3D *boundingGeometry,
                                      const PlaneGeometry *planeGeometry, vtkFloatingPointType *bounds );
  };

} // namespace mitk

#endif /* MITKIMAGEVTKMAPPER2D_H_HEADER_INCLUDED_C10E906E */
