#ifndef MITKIMAGEMAPPER2D_H_HEADER_INCLUDED_C10E906E
#define MITKIMAGEMAPPER2D_H_HEADER_INCLUDED_C10E906E

#include "mitkCommon.h"
#include "Geometry3D.h"
#include "LevelWindow.h"
#include "DataTreeNode.h"
#include "Mapper2D.h"
#include "ImageSliceSelector.h"

#include <map>
#include "mitkGLMapper2D.h"
#include "ImageChannelSelector.h"
#include "BaseRenderer.h"

class iilPicImage;
class Vtk2itk;
class vtkImageReslice;

namespace mitk {

  class RenderWindow;
  class BaseRenderer;

  //##ModelId=3D74B23E0176
//##Documentation
//## @brief OpenGL-texture-based mapper to display slices of an image
//## @ingroup Mapper
//## @todo the resampling is still left-handed, the flipping is done by OpenGL.
//## This results in a flipped version when used for texture mapping. Furthermore,
//## not the complete rectangular area described by the Geometry2D from the renderer
//## is resampled, @em if the Geometry2D is larger than the image dimension in the requested
//## direction. This results in a stretched version when used for texture mapping.
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

    //##ModelId=3E3D834B003A
    virtual void GenerateData();
    //##ModelId=3ED94AAD0315
    virtual void GenerateAllData();

    //##ModelId=3E0B14DC0086
    virtual void Paint(mitk::BaseRenderer * renderer);

    //##ModelId=3E6423D20047
    //##Documentation
    //## @brief internal storage class for data needed for rendering into a renderer
    class RendererInfo {
    public:
      //##ModelId=3E6423D203E0
      //##Documentation
      //## @brief timestamp of last update of stored data
      unsigned long int m_LastUpdateTime;
      //##ModelId=3E6423D203E1
      //##Documentation
      //## @brief stored iilPicImage containing the texture to display
      iilPicImage* m_iilImage; 
      //##ModelId=3E6E83AB02CD
      //##Documentation
      //## @brief stored data as a ipPicDescriptor
      ipPicDescriptor *m_Pic;
      //##ModelId=3E6E83AB0346
      //##Documentation
      //## @brief internal id of the renderer the data is stored for
      int m_RendererId;
      //##ModelId=3E6423D30002
      RendererInfo() : m_LastUpdateTime(0), m_iilImage(NULL), m_Pic(NULL), m_RendererId(-1)
      {
      
      };
        //##ModelId=3E6423D30003
      ~RendererInfo()
      {
    //         delete m_iilImage; //FIXME: diese Zeile wird nie erreicht, s. Kommentar im desctuctor von ImageMapper2D
      }
    };
	
    //##ModelId=3E6E83B00343
    //##Documentation
    //## @brief Get the internal id of the renderer
    //## @sa RendererInfo
    virtual int GetAssociatedChannelNr(mitk::BaseRenderer *renderer);
    //##ModelId=3ED932B00140
    //##Documentation
    //## @brief Generate the data needed for rendering into @a renderer
    virtual void GenerateData(mitk::BaseRenderer *renderer);
    //##ModelId=3ED94AAD0326
    //##Documentation
    //## @brief Get the RendererInfo for @a renderer
	const RendererInfo * GetRendererInfo(mitk::BaseRenderer* renderer)
	{
		return &m_RendererInfo[renderer];
	}

  protected:
    //##ModelId=3E3BD98600F4
    ImageMapper2D();

    //##ModelId=3E32DCF60043
    virtual ~ImageMapper2D();

    //##ModelId=3E8607D20380
    virtual void GenerateOutputInformation();

    //##ModelId=3E6E83AB0347
    //##Documentation
    //## @brief Number of renderers data is stored for
    //## @todo General concept for keeping data for rendering required
    //## @todo static?
    static int numRenderer;

  protected:
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
    std::map<mitk::BaseRenderer*,RendererInfo> m_RendererInfo;
  };
  
} // namespace mitk



#endif /* MITKIMAGEMAPPER2D_H_HEADER_INCLUDED_C10E906E */
