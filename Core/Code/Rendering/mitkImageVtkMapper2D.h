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
class vtkMitkApplyLevelWindowToRGBFilter;

namespace mitk {

  /** \brief Mapper to resample and display 2D slices of a 3D image.
 * 
 * The following image gives a brief overview of the mapping and the involved parts.
 *
 * \image html imageVtkMapper2Darchitecture.png
 *
 * First, the image is resliced by means of vtkImageReslice. The volume image
 * serves as input to the mapper in addition to spatial placement of the slice and a few other
 * properties such as thick slices. This code was already present in the old version
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
 * same transformation as used for reslicing is applied to both the camera and the
 * vtkActor. All important steps are explained in more detail below. The resulting
 * 2D image (by reslicing the underlying 3D input image appropriately) can either
 * be directly rendered in a 2D view or just be calculated to be used later by another
 * rendering entity, e.g. in texture mapping in a 3D view.
 *
 * Properties that can be set for images and influence the imageMapper2D are:
 *
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
 *   - \b "volume annotation color": (ColorProperty) color of the volume annotation, TODO has to be reimplemented
 *   - \b "volume annotation unit": (StringProperty) annotation unit as string (does not implicit convert the unit!) 
          unit is ml or cm3, TODO has to be reimplemented

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

      /** \brief Origin of the 2D geometry. */
      mitk::Point3D m_Origin;
      /** \brief Bottom end point of the y-axis of the 2D geometry. */
      mitk::Vector3D m_Bottom;
      /** \brief Right end point of the x-axis of the 2D geometry. */
      mitk::Vector3D m_Right;
      /** \brief Normal of the 2D geometry. */
      mitk::Vector3D m_Normal;
      /** \brief mmPerPixel relation between pixel and mm. (World spacing).*/
      mitk::ScalarType m_mmPerPixel[2];

      /** \brief This filter is used to apply the level window to RBG(A) images. */
      vtkMitkApplyLevelWindowToRGBFilter* m_LevelWindowToRGBFilterObject;

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
    void GeneratePlane(mitk::BaseRenderer* renderer, vtkFloatingPointType planeBounds[6]);

    /** \brief Generates a vtkPolyData object containing the outline of a given binary slice.
      \param binarySlice - The binary image slice. (Volumes are not supported.)
      \param mmPerPixel - Spacing of the binary image slice. Hence it's 2D, only in x/y-direction.
      \note This code has been taken from the deprecated library iil.
      */
    vtkSmartPointer<vtkPolyData> CreateOutlinePolyData(mitk::BaseRenderer* renderer);

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
    * applied to each textured plane (actor->SetUserTransform(t)) to transform everything
    * to the actual 3D position (cf. the following image).
    *
    * \image html cameraPositioning3D.png
    *
    */
    virtual void GenerateDataForRenderer(mitk::BaseRenderer *renderer);

    /** \brief Internal helper method for intersection testing used only in CalculateClippedPlaneBounds() */
    bool LineIntersectZero( vtkPoints *points, int p1, int p2,
                            vtkFloatingPointType *bounds );

    /** \brief Calculate the bounding box of the resliced image. This is necessary for
        arbitrarily rotated planes in an image volume. A rotated plane (e.g. in swivel mode)
        will have a new bounding box, which needs to be calculated. */
    bool CalculateClippedPlaneBounds( const Geometry3D *boundingGeometry,
                                      const PlaneGeometry *planeGeometry, vtkFloatingPointType *bounds );

    /** \brief This method uses the vtkCamera clipping range and the layer property
    * to calcualte the depth of the object (e.g. image or contour). The depth is used
    * to keep the correct order for the final VTK rendering.*/
    float CalculateLayerDepth(mitk::BaseRenderer* renderer);

    /** \brief This method applies a level window on RBG(A) images.
    * It should only be called for internally for RGB(A) images. */
    void ApplyRBGALevelWindow( mitk::BaseRenderer* renderer );

    /** \brief This method applies (or modifies) the lookuptable for all types of images. */
    void ApplyLookuptable( mitk::BaseRenderer* renderer );

    /** \brief This method applies a color transfer function, if no LookuptableProperty is set.
    Internally, a vtkColorTransferFunction is used. This is usefull for coloring continous
    images (e.g. float) */
    void ApplyColorTransferFunction(mitk::BaseRenderer* renderer);

    /** \brief Set the color of the image/polydata */
    void ApplyColor( mitk::BaseRenderer* renderer );

    /** \brief Set the opacity of the actor. */
    void ApplyOpacity( mitk::BaseRenderer* renderer );
  };

} // namespace mitk

#endif /* MITKIMAGEVTKMAPPER2D_H_HEADER_INCLUDED_C10E906E */
