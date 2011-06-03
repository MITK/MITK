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

//VTK TODO use "class" instead?
#include <vtkSmartPointer.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkPlaneSource.h>
#include <vtkImageData.h>
#include <vtkLookupTable.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkMatrix4x4.h>

class Vtk2itk;
class vtkImageReslice;
class vtkLookupTable;
class vtkGeneralTransform;
class vtkImageChangeInformation;
class vtkPoints;
class vtkMitkThickSlicesFilter;

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

    virtual void ApplyProperties(mitk::BaseRenderer* renderer);

    virtual vtkProp* GetVtkProp(mitk::BaseRenderer* renderer);

    virtual void MitkRenderOverlay(BaseRenderer* renderer);
    virtual void MitkRenderOpaqueGeometry(BaseRenderer* renderer);
    virtual void MitkRenderTranslucentGeometry(BaseRenderer* renderer);
    virtual void MitkRenderVolumetricGeometry(BaseRenderer* renderer);

    /** \brief Internal class holding the mapper, actor, etc. for each of the 3 2D render windows */
    class LocalStorage : public mitk::Mapper::BaseLocalStorage
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

      /** \brief Constructor of the local storage. Do as much actions as possible in here to avoid double executions. */
      LocalStorage()
      {
        m_ReslicedImage = vtkSmartPointer<vtkImageData>::New();
        m_Plane = vtkSmartPointer<vtkPlaneSource>::New();
        m_Texture = vtkSmartPointer<vtkTexture>::New();
        m_LookupTable = vtkSmartPointer<vtkLookupTable>::New();
        m_Mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
        m_Actor = vtkSmartPointer<vtkActor>::New();
        m_TransformFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();

        //the following actions are always the same and thus can be performed
        //in the constructor for each image (i.e. the image-corresponding local storage)

        //built a default lookuptable
        m_LookupTable->SetSaturationRange( 0.0, 0.0 );
        m_LookupTable->SetHueRange( 0.0, 0.0 );
        m_LookupTable->SetValueRange( 0.0, 1.0 );
        m_LookupTable->Build();
        //map all black values to transparent
        m_LookupTable->SetTableValue(0, 0.0, 0.0, 0.0, 0.0);

        //set the mapper for the actor
        m_Actor->SetMapper(m_Mapper);
        //set the texture for the actor
        m_Actor->SetTexture(m_Texture);
      }

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
      /** \brief timestamp of last update of stored data */
      itk::TimeStamp m_LastUpdateTime;

      /** \brief number of pixels per mm in x- and y-direction of the resampled */
      Vector2D m_PixelsPerMM;

      /** \brief Extent (in pixels) of the image */
      Vector2D m_Extent;

      /** \brief Overlap (in pixels) to ensure coverage of rotated images also */
      Vector2D m_Overlap;

      /** \brief Using unit spacing for resampling makes life easier */
      vtkImageChangeInformation *m_UnitSpacingImageFilter;

      /** \brief The actual reslicer (one per renderer) */
      vtkImageReslice *m_Reslicer;

      /** \brief Thickslices post filtering */
      vtkMitkThickSlicesFilter *m_TSFilter;

      /** \brief Extracted image for 3D rendering */
      vtkImageData *m_Image;

      /** \brief Reference geometry associated with the world geometry */
      const Geometry3D *m_ReferenceGeometry;

      bool m_TextureInterpolation;

      /** \brief stores the id of the observer for delete event of renderer */
      unsigned long m_ObserverID;

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

      void Squeeze();
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
    void GeneratePlane(mitk::BaseRenderer* renderer, mitk::ScalarType spacing[2]);

    //set the camera to view the textured plane
    void AdjustCamera(mitk::BaseRenderer* renderer);

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
