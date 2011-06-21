/*=========================================================================
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef MITKIMAGEVTKMAPPER2D_H_HEADER_INCLUDED_C10E906E
#define MITKIMAGEVTKMAPPER2D_H_HEADER_INCLUDED_C10E906E

//MITK Rendering
#include "mitkCommon.h"
#include "mitkBaseRenderer.h"
#include "mitkVtkMapper2D.h"

//VTK
#include <vtkSmartPointer.h>

class vtkActor;
class vtkPolyDataMapper;
class vtkPlaneSource;
class vtkImageData;
class vtkTransformPolyDataFilter;
class vtkLookupTable;
class vtkImageReslice;
class vtkImageChangeInformation;
class vtkPoints;
class vtkMitkThickSlicesFilter;
class vtkPolyData;

namespace mitk {

  /** \brief Mapper to resample and display 2D slices of a 3D image.
 *
 * Currently implemented for mapping on PlaneGeometry and
 * AbstractTransformGeometry. The resulting 2D image (by reslicing the
 * underlying 3D input image appropriately) can either be directly rendered
 * in a 2D view or just be calculated to be used later on by another
 * rendering entity, e.g. in texture mapping in a 3D view.
 *
 * This results in a flipped version when used for texture mapping. Furthermore,
 * not the complete rectangular area described by the Geometry2D from the renderer
 * is resampled, @em if the Geometry2D is larger than the image dimension in the
 * requested direction. This results in a stretched version when used for texture
 * mapping.
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

    /** \brief Calls Update() for all associated renderers. */
    virtual void GenerateAllData();

    /** \brief Checks whether this mapper needs to update itself and generate
   * data. */
    virtual void Update(mitk::BaseRenderer * renderer);

    virtual void ApplyProperties(mitk::BaseRenderer* renderer, vtkSmartPointer<vtkTransform> transform, ScalarType mmPerPixel[2]);

    virtual vtkProp* GetVtkProp(mitk::BaseRenderer* renderer);

    virtual void MitkRenderOverlay(BaseRenderer* renderer);
    virtual void MitkRenderOpaqueGeometry(BaseRenderer* renderer);
    virtual void MitkRenderTranslucentGeometry(BaseRenderer* renderer);
    virtual void MitkRenderVolumetricGeometry(BaseRenderer* renderer);

    /** \brief Internal class holding the mapper, actor, etc. for each of the 3 2D render windows */
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
      /** \brief transform the plane */
      vtkSmartPointer<vtkTransformPolyDataFilter> m_TransformFilter;
      /** \brief The actual reslicer (one per renderer) */
      vtkSmartPointer<vtkImageReslice> m_Reslicer;
      /** \brief Thickslices post filtering */
      vtkSmartPointer<vtkMitkThickSlicesFilter> m_TSFilter;
      /** \brief Using unit spacing for resampling makes life easier TODO improve docu ...*/
      vtkSmartPointer<vtkImageChangeInformation> m_UnitSpacingImageFilter;      
      /** \brief PolyData object containg all lines/points needed for outlining the contour.*/
      vtkSmartPointer<vtkPolyData> m_OutlinePolyData;

      /** \brief timestamp of last update of stored data */
      itk::TimeStamp m_LastUpdateTime;

      /** \brief Constructor of the local storage. Do as much actions as possible in here to avoid double executions. */
      LocalStorage();

      ~LocalStorage()
      {
      }
    };

    /** \brief This member holds all three LocalStorages for the three 2D render windows. */
    mitk::Mapper::LocalStorageHandler<LocalStorage> m_LSH;

    /** \brief Internal storage class for data needed for rendering into a
   * renderer
   */
    class MITK_CORE_EXPORT RendererInfo
    {
      /** \brief internal id of the renderer the data is stored for */
      int m_RendererID;

      mitk::BaseRenderer* m_Renderer;

    public:
      /** \brief stores the id of the observer for delete event of renderer */
      unsigned long m_ObserverID;

      vtkImageData* m_Image;

      RendererInfo();

      ~RendererInfo();

      inline bool IsInitialized() const
      {
        return m_RendererID >= 0;
      }

      void Initialize( int rendererID, mitk::BaseRenderer *renderer,
                       unsigned long observerID );

      inline int GetRendererID() const
      {
        return m_RendererID;
      }

      void RemoveObserver();

    }; // RendererInfo

    /** \brief Get the RendererInfo for \a renderer */
    const RendererInfo *GetRendererInfo( mitk::BaseRenderer *renderer )
    {
      return &this->AccessRendererInfo(renderer);
    }

    /** \brief Release memory allocated for buffering */
    virtual void Clear();

    static void SetDefaultProperties(mitk::DataNode* node, mitk::BaseRenderer* renderer = NULL, bool overwrite = false);

  protected:
    //Generate a plane with size of the image in mm
    void GeneratePlane(mitk::BaseRenderer* renderer, vtkFloatingPointType planeBounds[6]);

    //set the camera to view the textured plane
    void AdjustCamera(mitk::BaseRenderer* renderer);

    vtkSmartPointer<vtkPolyData> CreateOutlinePolyData(vtkSmartPointer<vtkImageData> binarySlice, ScalarType mmPerPixel[2]);

    ImageVtkMapper2D();

    virtual ~ImageVtkMapper2D();

    /** Does the actual resampling, without rendering the image yet. */
    virtual void GenerateData(mitk::BaseRenderer *renderer);

    /** \brief Get the RendererInfo for @a renderer */
    inline RendererInfo & AccessRendererInfo( mitk::BaseRenderer* renderer )
    {
      RendererInfo& rendererInfo = m_RendererInfo[renderer];
      if(rendererInfo.IsInitialized()==false)
      {
        // Add observer for renderer reset events (RendererInfo will
        // automatically be removed from list when a Renderer is deleted)
        //
        // Note: observer ID is passed to rendererInfo, which will take
        // responsiblity to remove the observer upon its destruction
        typedef itk::MemberCommand< ImageVtkMapper2D > MemberCommandType;
        MemberCommandType::Pointer deleteRendererCommand =
            MemberCommandType::New();

        deleteRendererCommand->SetCallbackFunction(
            this, &ImageVtkMapper2D::DeleteRendererCallback );

        unsigned long observerID = renderer->AddObserver(
            BaseRenderer::RendererResetEvent(), deleteRendererCommand );

        // Initialize RendererInfo
        rendererInfo.Initialize( ImageVtkMapper2D::numRenderer++, renderer, observerID );
      }

      return rendererInfo;
    }

    void DeleteRendererCallback( itk::Object *object, const itk::EventObject & );

    bool LineIntersectZero( vtkPoints *points, int p1, int p2,
                            vtkFloatingPointType *bounds );

    bool CalculateClippedPlaneBounds( const Geometry3D *boundingGeometry,
                                      const PlaneGeometry *planeGeometry, vtkFloatingPointType *bounds );

    /** \brief Number of renderers data is stored for
   * \todo General concept for keeping data for rendering required
   * \todo static?
   */
    static int numRenderer;

  protected:
    typedef std::map<BaseRenderer*,RendererInfo> RendererInfoMap;

    /** \brief Map of instances of RendererInfo
   * \sa RendererInfo
   */
    RendererInfoMap m_RendererInfo;

  };

} // namespace mitk

#endif /* MITKIMAGEVTKMAPPER2D_H_HEADER_INCLUDED_C10E906E */
