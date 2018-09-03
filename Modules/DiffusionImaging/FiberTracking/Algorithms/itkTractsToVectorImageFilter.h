#ifndef __itkTractsToVectorImageFilter_h__
#define __itkTractsToVectorImageFilter_h__

// MITK
#include <mitkFiberBundle.h>

// ITK
#include <itkImageSource.h>
#include <itkVectorImage.h>

// VTK
#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include <vtkCellArray.h>
#include <vtkPoints.h>
#include <vtkPolyLine.h>

namespace itk{

/**
* \brief Extracts the voxel-wise main directions of the input fiber bundle.   */

template< class PixelType >
class TractsToVectorImageFilter : public ImageSource< Image< PixelType, 4 > >
{

public:

  enum NormalizationMethods {
      GLOBAL_MAX,         ///< global maximum normalization
      SINGLE_VEC_NORM,    ///< normalize the single peaks to length 1
      MAX_VEC_NORM        ///< normalize all peaks according to their length in comparison to the largest peak in the voxel (0-1)
  };

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
  typedef Image< PixelType, 4 >                                               ItkDirectionImageType;
  typedef itk::Image<unsigned char, 3>                                        ItkUcharImgType;
  typedef itk::Image<double, 3>                                               ItkDoubleImgType;

  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)
  itkTypeMacro( TractsToVectorImageFilter, ImageSource )

  itkSetMacro( SizeThreshold, float)
  itkGetMacro( SizeThreshold, float)
  itkSetMacro( AngularThreshold, float)                               ///< cluster directions that are closer together than the specified threshold
  itkGetMacro( AngularThreshold, float)                               ///< cluster directions that are closer together than the specified threshold
  itkSetMacro( NormalizationMethod, NormalizationMethods)             ///< normalization method of peaks
  itkSetMacro( MaxNumDirections, unsigned long)                       ///< If more directions are extracted, only the largest are kept.
  itkGetMacro( MaxNumDirections, unsigned long)                       ///< If more directions are extracted, only the largest are kept.
  itkSetMacro( MaskImage, ItkUcharImgType::Pointer)                   ///< only process voxels inside mask
  itkSetMacro( FiberBundle, mitk::FiberBundle::Pointer)               ///< input fiber bundle
  itkSetMacro( OnlyUseMaskGeometry, bool)                             ///< don't use content of mask image, only use it's geometry
  itkGetMacro( ClusteredDirectionsContainer, ContainerType::Pointer)  ///< output directions
  itkGetMacro( NumDirectionsImage, ItkUcharImgType::Pointer)          ///< number of directions per voxel
  itkGetMacro( DirectionImage, typename ItkDirectionImageType::Pointer)        ///< output directions

  void GenerateData() override;

protected:

  DirectionContainerType::Pointer FastClustering(DirectionContainerType::Pointer inDirs, std::vector< double > lengths);  ///< cluster fiber directions

  vnl_vector_fixed<double, 3> GetVnlVector(double point[3]);


  TractsToVectorImageFilter();
  ~TractsToVectorImageFilter() override;

  NormalizationMethods                m_NormalizationMethod;              ///< normalization method of peaks
  mitk::FiberBundle::Pointer          m_FiberBundle;                      ///< input fiber bundle
  float                               m_AngularThreshold;                 ///< cluster directions that are closer together than the specified threshold
  float                               m_Epsilon;                          ///< epsilon for vector equality check
  ItkUcharImgType::Pointer            m_MaskImage;                        ///< only voxels inside the binary mask are processed
  itk::Vector<float>                  m_OutImageSpacing;                  ///< spacing of output image
  ContainerType::Pointer              m_DirectionsContainer;              ///< container for fiber directions
  unsigned long                       m_MaxNumDirections;                 ///< if more directions per voxel are extracted, only the largest are kept
  float                               m_SizeThreshold;
  bool                                m_OnlyUseMaskGeometry;

  // output datastructures
  typename ItkDirectionImageType::Pointer m_DirectionImage;
  ContainerType::Pointer                  m_ClusteredDirectionsContainer; ///< contains direction vectors for each voxel
  ItkUcharImgType::Pointer                m_NumDirectionsImage;           ///< shows number of fibers per voxel
};

}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkTractsToVectorImageFilter.cpp"
#endif

#endif // __itkTractsToVectorImageFilter_h__
