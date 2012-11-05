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

class TractsToVectorImageFilter : public ImageSource< VectorImage< float, 3 > >
{

public:
    typedef TractsToVectorImageFilter Self;
    typedef ProcessObject Superclass;
    typedef SmartPointer< Self > Pointer;
    typedef SmartPointer< const Self > ConstPointer;

    typedef itk::Vector<float,3>                    OutputVectorType;
    typedef itk::Image<OutputVectorType, 3>     OutputImageType;
    typedef std::vector< OutputImageType::Pointer > OutputImageContainerType;

    typedef vnl_vector_fixed< double, 3 >                               DirectionType;
    typedef VectorContainer< int, DirectionType >                       DirectionContainerType;
    typedef VectorContainer< int, DirectionContainerType::Pointer >     ContainerType;
    typedef Image< Vector< float, 3 >, 3>                               ItkDirectionImageType;
    typedef VectorContainer< int, ItkDirectionImageType::Pointer >      DirectionImageContainerType;

    typedef itk::Image<unsigned char, 3>  ItkUcharImgType;

    itkNewMacro(Self)
    itkTypeMacro( TractsToVectorImageFilter, ImageSource )

    itkSetMacro( AngularThreshold, float)
    itkGetMacro( AngularThreshold, float)

    itkSetMacro( FiberSampling, int)
    itkGetMacro( FiberSampling, int)

    itkSetMacro( UseFastClustering, bool)
    itkGetMacro( UseFastClustering, bool)

    itkSetMacro( NormalizeVectors, bool)
    itkGetMacro( NormalizeVectors, bool)

    itkSetMacro( UseWorkingCopy, bool)
    itkGetMacro( UseWorkingCopy, bool)

    itkSetMacro( MaxNumDirections, int)
    itkGetMacro( MaxNumDirections, int)

    itkGetMacro( ClusteredDirectionsContainer, ContainerType::Pointer)
    itkGetMacro( NumDirectionsImage, ItkUcharImgType::Pointer)
    itkGetMacro( CrossingsImage, ItkUcharImgType::Pointer)

    itkSetMacro( MaskImage, ItkUcharImgType::Pointer)
    itkSetMacro( FiberBundle, FiberBundleX::Pointer)
    itkGetMacro( OutputFiberBundle, FiberBundleX::Pointer)
    itkGetMacro( DirectionImageContainer, DirectionImageContainerType::Pointer)

    void GenerateData();

protected:


    std::vector< DirectionType > Clustering(std::vector< DirectionType >& inDirs);
    std::vector< DirectionType > FastClustering(std::vector< DirectionType >& inDirs);
    DirectionContainerType::Pointer MeanShiftClustering(DirectionContainerType::Pointer dirCont);
    vnl_vector_fixed<double, 3> ClusterStep(DirectionContainerType::Pointer dirCont, vnl_vector_fixed<double, 3> currentMean);

    vnl_vector_fixed<double, 3> GetVnlVector(double point[3]);
    itk::Point<float, 3> GetItkPoint(double point[3]);

    float GaussWeighting(float distance);
    float   m_GaussFac1;
    float   m_GaussFac2;
    float   m_Thres;

    TractsToVectorImageFilter();
    virtual ~TractsToVectorImageFilter();

    FiberBundleX::Pointer               m_FiberBundle;
    float                               m_AngularThreshold;
    float                               m_Epsilon;
    ItkUcharImgType::Pointer            m_MaskImage;
    bool                                m_NormalizeVectors;
    mitk::Vector3D                      m_OutImageSpacing;
    ContainerType::Pointer              m_DirectionsContainer;
    bool                                m_UseWorkingCopy;
    bool                                m_UseTrilinearInterpolation;
    int                                 m_MaxNumDirections;
    bool                                m_UseFastClustering;
    int                                 m_FiberSampling;

    // output datastructures
    ContainerType::Pointer                  m_ClusteredDirectionsContainer; // contains direction vectors for each voxel
    ItkUcharImgType::Pointer                m_NumDirectionsImage; // shows number of fibers per voxel
    ItkUcharImgType::Pointer                m_CrossingsImage; // shows voxels containing more than one fiber
    DirectionImageContainerType::Pointer    m_DirectionImageContainer;
    FiberBundleX::Pointer                   m_OutputFiberBundle;
};

}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkTractsToVectorImageFilter.cpp"
#endif

#endif // __itkTractsToVectorImageFilter_h__
