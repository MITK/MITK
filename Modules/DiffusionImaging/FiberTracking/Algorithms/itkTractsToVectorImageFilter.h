#ifndef __itkTractsToVectorImageFilter_h__
#define __itkTractsToVectorImageFilter_h__

// MITK
#include <mitkFiberBundleX.h>

// ITK
#include <itkImageSource.h>
#include <itkVectorImage.h>

// VTK
#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include <vtkCellArray.h>
#include <vtkPoints.h>
#include <vtkPolyLine.h>

using namespace mitk;

namespace itk{

/**
* \brief Extracts the voxel-wise main directions of the input fiber bundle.   */

template< class PixelType >
class TractsToVectorImageFilter : public ImageSource< VectorImage< float, 3 > >
{

public:
    typedef TractsToVectorImageFilter Self;
    typedef ProcessObject Superclass;
    typedef SmartPointer< Self > Pointer;
    typedef SmartPointer< const Self > ConstPointer;

    typedef itk::Vector<float,3>                    OutputVectorType;
    typedef itk::Image<OutputVectorType, 3>         OutputImageType;
    typedef std::vector< OutputImageType::Pointer > OutputImageContainerType;

    typedef vnl_vector_fixed< double, 3 >                                       DirectionType;
    typedef VectorContainer< unsigned int, DirectionType >                      DirectionContainerType;
    typedef VectorContainer< unsigned int, DirectionContainerType::Pointer >    ContainerType;
    typedef Image< Vector< float, 3 >, 3>                                       ItkDirectionImageType;
    typedef VectorContainer< unsigned int, ItkDirectionImageType::Pointer >     DirectionImageContainerType;
    typedef itk::Image<unsigned char, 3>                                        ItkUcharImgType;
    typedef itk::Image<double, 3>                                               ItkDoubleImgType;

    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)
    itkTypeMacro( TractsToVectorImageFilter, ImageSource )

    itkSetMacro( SizeThreshold, float)
    itkGetMacro( SizeThreshold, float)
    itkSetMacro( AngularThreshold, float)                               ///< cluster directions that are closer together than the specified threshold
    itkGetMacro( AngularThreshold, float)                               ///< cluster directions that are closer together than the specified threshold
    itkSetMacro( NormalizeVectors, bool)                                ///< Normalize vectors to length 1
    itkGetMacro( NormalizeVectors, bool)                                ///< Normalize vectors to length 1
    itkSetMacro( UseWorkingCopy, bool)                                  ///< Do not modify input fiber bundle. Use a copy.
    itkGetMacro( UseWorkingCopy, bool)                                  ///< Do not modify input fiber bundle. Use a copy.
    itkSetMacro( MaxNumDirections, unsigned long)                       ///< If more directions are extracted, only the largest are kept.
    itkGetMacro( MaxNumDirections, unsigned long)                       ///< If more directions are extracted, only the largest are kept.
    itkSetMacro( MaskImage, ItkUcharImgType::Pointer)                   ///< only process voxels inside mask
    itkSetMacro( FiberBundle, FiberBundleX::Pointer)                    ///< input fiber bundle
    itkGetMacro( ClusteredDirectionsContainer, ContainerType::Pointer)  ///< output directions
    itkGetMacro( NumDirectionsImage, ItkUcharImgType::Pointer)          ///< number of directions per voxel
    itkGetMacro( OutputFiberBundle, FiberBundleX::Pointer)              ///< vector field for visualization purposes
    itkGetMacro( DirectionImageContainer, DirectionImageContainerType::Pointer) ///< output directions
    itkSetMacro( CreateDirectionImages, bool)

    void GenerateData();

protected:

    DirectionContainerType::Pointer FastClustering(DirectionContainerType::Pointer inDirs, std::vector< double > lengths);  ///< cluster fiber directions
//    std::vector< DirectionType > Clustering(std::vector< DirectionType >& inDirs);
//    DirectionContainerType::Pointer MeanShiftClustering(DirectionContainerType::Pointer dirCont);
//    vnl_vector_fixed<double, 3> ClusterStep(DirectionContainerType::Pointer dirCont, vnl_vector_fixed<double, 3> currentMean);

    vnl_vector_fixed<double, 3> GetVnlVector(double point[3]);
    itk::Point<double, 3> GetItkPoint(double point[3]);


    TractsToVectorImageFilter();
    virtual ~TractsToVectorImageFilter();

    FiberBundleX::Pointer               m_FiberBundle;                      ///< input fiber bundle
    float                               m_AngularThreshold;                 ///< cluster directions that are closer together than the specified threshold
    float                               m_Epsilon;                          ///< epsilon for vector equality check
    ItkUcharImgType::Pointer            m_MaskImage;                        ///< only voxels inside the binary mask are processed
    bool                                m_NormalizeVectors;                 ///< normalize vectors to length 1
    itk::Vector<float>                  m_OutImageSpacing;                  ///< spacing of output image
    ContainerType::Pointer              m_DirectionsContainer;              ///< container for fiber directions
    bool                                m_UseWorkingCopy;                   ///< do not modify input fiber bundle but work on copy
    unsigned long                       m_MaxNumDirections;                 ///< if more directions per voxel are extracted, only the largest are kept
    float                               m_SizeThreshold;
    bool                                m_CreateDirectionImages;

    // output datastructures
    ContainerType::Pointer                  m_ClusteredDirectionsContainer; ///< contains direction vectors for each voxel
    ItkUcharImgType::Pointer                m_NumDirectionsImage;           ///< shows number of fibers per voxel
    DirectionImageContainerType::Pointer    m_DirectionImageContainer;      ///< contains images that contain the output directions
    FiberBundleX::Pointer                   m_OutputFiberBundle;            ///< vector field for visualization purposes
};

}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkTractsToVectorImageFilter.cpp"
#endif

#endif // __itkTractsToVectorImageFilter_h__
