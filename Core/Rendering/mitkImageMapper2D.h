/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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


#ifndef MITKIMAGEMAPPER2D_H_HEADER_INCLUDED_C10E906E
#define MITKIMAGEMAPPER2D_H_HEADER_INCLUDED_C10E906E

#include "mitkCommon.h"
#include "mitkGeometry3D.h"
#include "mitkLevelWindow.h"
#include "mitkDataTreeNode.h"
#include "mitkMapper2D.h"
#include "mitkImageSliceSelector.h"

#include <map>
#include <assert.h>
#include "mitkGLMapper2D.h"
#include "mitkImageChannelSelector.h"
#include "mitkBaseRenderer.h"

#include <itkCommand.h>
#include <itkEventObject.h>


class iil4mitkPicImage;
class Vtk2itk;
class vtkImageReslice;
class vtkLookupTable;
class vtkGeneralTransform;

namespace mitk {

class RenderWindow;
class BaseRenderer;

//##ModelId=3D74B23E0176
//##Documentation
//## @brief OpenGL-texture-based mapper to display slices of an image
//##
//## Currently implemented for mapping on PlaneGeometry and AbstractTransformGeometry.
//## This results in a flipped version when used for texture mapping. Furthermore,
//## not the complete rectangular area described by the Geometry2D from the renderer
//## is resampled, @em if the Geometry2D is larger than the image dimension in the requested
//## direction. This results in a stretched version when used for texture mapping.
//## @ingroup Mapper
class ImageMapper2D : public GLMapper2D
{
  friend class MapperFactory;
public:
  /** Standard class typedefs. */
  mitkClassMacro(ImageMapper2D,GLMapper2D);
  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Some convenient typedefs. */
  //##ModelId=3E3D834A018D
  typedef mitk::Image                    InputImageType;
  //##ModelId=3E3D834A01AB
  typedef InputImageType::Pointer        InputImagePointer;
  //##ModelId=3E3D834A01B5
  typedef InputImageType::ConstPointer   InputImageConstPointer;
  //##ModelId=3E3D834A01C9
  typedef SlicedData::RegionType         InputImageRegionType;

  //##ModelId=3E3D834B0008
  //##Documentation
  //## @brief Get the Image to map
  const InputImageType * GetInput(void);

  //##ModelId=3ED94AAD0315
  virtual void GenerateAllData();

  //##ModelId=3E0B14DC0086
  virtual void Paint(mitk::BaseRenderer * renderer);

  virtual void Update(mitk::BaseRenderer * renderer);

  virtual void ApplyProperties(mitk::BaseRenderer* renderer);

  //##ModelId=3E6423D20047
  //##Documentation
  //## @brief internal storage class for data needed for rendering into a renderer
  class RendererInfo {
    //##ModelId=3E6E83AB0346
    //##Documentation
    //## @brief internal id of the renderer the data is stored for
    int m_RendererId;
    
    //##ModelId=3E6423D203E1
    //##Documentation
    //## @brief stored iil4mitkPicImage containing the texture to display
    iil4mitkPicImage* m_iil4mitkImage;
    mitk::BaseRenderer* m_Renderer;
    unsigned long m_ObserverId;

    void RendererDeleted();
  public:
    //##ModelId=3E6423D203E0
    //##Documentation
    //## @brief timestamp of last update of stored data
    itk::TimeStamp m_LastUpdateTime;
    //##ModelId=3E6E83AB02CD
    //##Documentation
    //## @brief stored data as a ipPicDescriptor
    ipPicDescriptor *m_Pic;
    //##Documentation
    //## @brief number of pixels per mm in x- and y-direction of the resampled image m_Pic
    Vector2D m_PixelsPerMM;
   
    bool m_IilInterpolation;
    //##ModelId=3E6423D30002
    RendererInfo() :  m_RendererId(-1), m_iil4mitkImage(NULL), m_Renderer(NULL), m_ObserverId(0), m_Pic(NULL), m_IilInterpolation(true)
    {
      m_PixelsPerMM.Fill(0);
    };
    //##ModelId=3E6423D30003
    ~RendererInfo()
    {
      Squeeze();
      if(m_Renderer != NULL)
      {
        m_Renderer->RemoveObserver(m_ObserverId);
        m_Renderer = NULL;
      }
      //         delete m_iil4mitkImage; //@FIXME: diese Zeile wird nie erreicht, s. Kommentar im desctuctor von ImageMapper2D
    }

    inline bool IsInitialized() const
    {
      return m_RendererId >= 0;
    }
    void Initialize(int rendererId, mitk::BaseRenderer* renderer)
    {
      assert(rendererId>=0);
      assert(m_RendererId<0);

      m_RendererId = rendererId;
      m_Renderer = renderer;

      itk::SimpleMemberCommand<RendererInfo>::Pointer deleteRendererCommand;
      deleteRendererCommand = itk::SimpleMemberCommand<RendererInfo>::New();
      deleteRendererCommand->SetCallbackFunction(this, &RendererInfo::RendererDeleted);

      m_ObserverId = renderer->AddObserver(mitk::BaseRenderer::RendererResetEvent(), deleteRendererCommand);
    }

    void Set_iil4mitkImage(iil4mitkPicImage* iil4mitkImage);

    inline iil4mitkPicImage* Get_iil4mitkImage() const
    {
      return m_iil4mitkImage;
    }

    inline int GetRendererId() const
    {
      return m_RendererId;
    }

    void Squeeze();
  };

  //##ModelId=3E6E83B00343
  //##Documentation
  //## @brief Get the internal id of the renderer
  //## @sa RendererInfo
  virtual int GetAssociatedChannelNr(mitk::BaseRenderer *renderer);

  //##ModelId=3ED94AAD0326
  //##Documentation
  //## @brief Get the RendererInfo for @a renderer
  const RendererInfo * GetRendererInfo(mitk::BaseRenderer* renderer)
  {
    return &AccessRendererInfo(renderer);
  }

  //##Documentation
  //## @brief Release memory allocated for buffering
  virtual void Clear();

protected:
  //##ModelId=3E3BD98600F4
  ImageMapper2D();

  //##ModelId=3E32DCF60043
  virtual ~ImageMapper2D();

  virtual void GenerateData(mitk::BaseRenderer *renderer);
  
  //##Documentation
  //## @brief Get the RendererInfo for @a renderer
  inline RendererInfo & AccessRendererInfo(mitk::BaseRenderer* renderer)
  {
    RendererInfo& renderinfo=m_RendererInfo[renderer];
    if(renderinfo.IsInitialized()==false)
      renderinfo.Initialize(ImageMapper2D::numRenderer++, renderer);
    return renderinfo;
  }

  //##ModelId=3E6E83AB0347
  //##Documentation
  //## @brief Number of renderers data is stored for
  //## @todo General concept for keeping data for rendering required
  //## @todo static?
  static int numRenderer;

protected:
  typedef std::map<mitk::BaseRenderer*,RendererInfo> RenderInfoMap;

  //##ModelId=3EDD039F02EC
  //##Documentation
  //## @brief ImageSliceSelector for display of original slices (currently not used)
  ImageSliceSelector::Pointer m_SliceSelector;

  //##ModelId=3E6423D30004
  //##Documentation
  //## @brief vtk-filter for reslicing
  vtkImageReslice* m_Reslicer;

  //##ModelId=3E6423D30016
  //##Documentation
  //## @brief Map of instances of RendererInfo
  //## @sa RendererInfo
  RenderInfoMap m_RendererInfo;

  vtkGeneralTransform* m_ComposedResliceTransformForAbstractTransformGeometry;

private:
	int m_iil4mitkMode;
	
};

} // namespace mitk

#endif /* MITKIMAGEMAPPER2D_H_HEADER_INCLUDED_C10E906E */
