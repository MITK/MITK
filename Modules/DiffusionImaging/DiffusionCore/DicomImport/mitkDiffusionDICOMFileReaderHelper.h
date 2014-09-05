#ifndef MITKDIFFUSIONDICOMFILEREADERHELPER_H
#define MITKDIFFUSIONDICOMFILEREADERHELPER_H

#include "mitkDiffusionImage.h"

#include "itkImageSeriesReader.h"
#include "itkVectorImage.h"

#include "itkImageRegionIteratorWithIndex.h"

namespace mitk
{

/**
 * @brief The MosaicDescriptor struct is a help struct holding the necessary information for
 *        loading a mosaic DICOM file into an MITK file with correct geometry information
 */
struct MosaicDescriptor
{
  unsigned int nimages;
  bool slicenormalup;

  itk::ImageBase<3>::SpacingType spacing;
  itk::ImageBase<3>::DirectionType direction;
  float origin[3];
};

class DiffusionDICOMFileReaderHelper
{
public:
  typedef std::vector< std::string > StringContainer;

  typedef std::vector< StringContainer > VolumeFileNamesContainer;

  std::string GetOutputName(const VolumeFileNamesContainer& filenames)
  {
    typedef itk::Image< short, 3> InputImageType;
    typedef itk::ImageSeriesReader< InputImageType > SeriesReaderType;

    SeriesReaderType::Pointer probe_reader = SeriesReaderType::New();
    probe_reader->SetFileNames( filenames.at(0) );
    probe_reader->GenerateOutputInformation();
    probe_reader->Update();

    std::string seriesDescTag, seriesNumberTag, patientName;
    SeriesReaderType::DictionaryArrayRawPointer inputDict = probe_reader->GetMetaDataDictionaryArray();

    if( ! itk::ExposeMetaData< std::string > ( *(*inputDict)[0], "0008|103e", seriesDescTag ) )
      seriesDescTag = "UNSeries";

    if( ! itk::ExposeMetaData< std::string > ( *(*inputDict)[0], "0020|0011", seriesNumberTag ) )
      seriesNumberTag = "00000";

    if( ! itk::ExposeMetaData< std::string > ( *(*inputDict)[0], "0010|0010", patientName ) )
      patientName = "UnknownName";

    std::stringstream ss;
    ss << seriesDescTag << "_" << seriesNumberTag << "_" << patientName;

    return ss.str();

  }

  template< typename PixelType, unsigned int VecImageDimension>
  typename itk::VectorImage< PixelType, VecImageDimension >::Pointer LoadToVector(
      const VolumeFileNamesContainer& filenames
      //const itk::ImageBase<3>::RegionType requestedRegion
      )
  {
    typedef itk::Image< PixelType, 3> InputImageType;
    typedef itk::ImageSeriesReader< InputImageType > SeriesReaderType;

    typename SeriesReaderType::Pointer probe_reader = SeriesReaderType::New();
    probe_reader->SetFileNames( filenames.at(0) );
    probe_reader->GenerateOutputInformation();
    const itk::ImageBase<3>::RegionType requestedRegion = probe_reader->GetOutput()->GetLargestPossibleRegion();

    MITK_INFO << " --- Probe reader : \n" <<
                 " Retrieved LPR " << requestedRegion;

    typedef itk::VectorImage< PixelType, 3 > VectorImageType;

    typename VectorImageType::Pointer output_image = VectorImageType::New();
    output_image->SetNumberOfComponentsPerPixel( filenames.size() );
    output_image->SetSpacing( probe_reader->GetOutput()->GetSpacing() );
    output_image->SetOrigin( probe_reader->GetOutput()->GetOrigin() );
    output_image->SetDirection( probe_reader->GetOutput()->GetDirection() );
    output_image->SetLargestPossibleRegion( probe_reader->GetOutput()->GetLargestPossibleRegion() );
    output_image->SetBufferedRegion( requestedRegion );
    output_image->Allocate();

    itk::ImageRegionIterator< VectorImageType > vecIter(
          output_image, requestedRegion );

    VolumeFileNamesContainer::const_iterator volumesFileNamesIter = filenames.begin();

    // iterate over the given volumes
    unsigned int component = 0;
    while( volumesFileNamesIter != filenames.end() )
    {

      MITK_INFO << " ======== Loading volume " << component+1 << " of " << filenames.size();

      typename SeriesReaderType::Pointer volume_reader = SeriesReaderType::New();
      volume_reader->SetFileNames( *volumesFileNamesIter );

      try
      {
        volume_reader->UpdateLargestPossibleRegion();
      }
      catch( const itk::ExceptionObject &e)
      {
        mitkThrow() << " ITK Series reader failed : "<< e.what();
      }

      itk::ImageRegionConstIterator< InputImageType > iRCIter (
            volume_reader->GetOutput(),
            volume_reader->GetOutput()->GetLargestPossibleRegion() );

      // transfer to vector image
      iRCIter.GoToBegin();
      vecIter.GoToBegin();



      while( !iRCIter.IsAtEnd() )
      {
        typename VectorImageType::PixelType vector_pixel = vecIter.Get();
        vector_pixel.SetElement( component, iRCIter.Get() );
        vecIter.Set( vector_pixel );

        ++vecIter;
        ++iRCIter;
      }

      ++volumesFileNamesIter;
      component++;
    }

    return output_image;

  }


  /**
   * Create the vector image for the resulting diffusion image from a mosaic DICOM image set,
   * The method needs to be provided with the MosaicDescriptor struct to be able to compute the
   * correct index and to set the geometry information of the image itself.
   */
  template< typename PixelType, unsigned int VecImageDimension>
  typename itk::VectorImage< PixelType, VecImageDimension >::Pointer LoadMosaicToVector(
      const VolumeFileNamesContainer& filenames,
      const MosaicDescriptor& mosaicInfo )
  {
    typedef itk::Image< PixelType, 3> MosaicImageType;
    typedef itk::ImageFileReader< MosaicImageType > SingleImageReaderType;

    // generate output
    typedef itk::VectorImage< PixelType, 3 > VectorImageType;

        VolumeFileNamesContainer::const_iterator volumesFileNamesIter = filenames.begin();

    // probe the first file to retrieve the size of the 2d image
    // we need this information to compute the index relation between mosaic and resulting 3d position
    // but we need it only once
    typename SingleImageReaderType::Pointer mosaic_probe = SingleImageReaderType::New();
    mosaic_probe->SetFileName( (*volumesFileNamesIter).at(0) );
    try
    {
      mosaic_probe->UpdateLargestPossibleRegion();
    }
    catch( const itk::ExceptionObject &e)
    {
      mitkThrow() << " ITK Image file reader failed : "<< e.what();
    }

    typename MosaicImageType::RegionType mosaic_lpr = mosaic_probe->GetOutput()->GetLargestPossibleRegion();
    MITK_INFO << " == MOSAIC: " << mosaic_lpr;

    itk::ImageBase<3>::SizeValueType images_per_row = ceil( sqrt( (float) mosaicInfo.nimages ) );

    itk::ImageBase<3>::RegionType requestedRegion;
    requestedRegion.SetSize( 0, mosaic_lpr.GetSize()[0]/images_per_row);
    requestedRegion.SetSize( 1, mosaic_lpr.GetSize()[1]/images_per_row);
    requestedRegion.SetSize( 2, mosaicInfo.nimages);

    typename VectorImageType::Pointer output_image = VectorImageType::New();
    output_image->SetNumberOfComponentsPerPixel( filenames.size() );

    typename VectorImageType::DirectionType dmatrix;
    dmatrix.SetIdentity();

    std::vector<double> dirx = mosaic_probe->GetImageIO()->GetDirection(0);
    std::vector<double> diry = mosaic_probe->GetImageIO()->GetDirection(1);
    std::vector<double> dirz = mosaic_probe->GetImageIO()->GetDirection(2);

    dmatrix.GetVnlMatrix().set_column( 0,  &dirx[0] );
    dmatrix.GetVnlMatrix().set_column( 1,  &diry[0] );
    dmatrix.GetVnlMatrix().set_column( 2,  &dirz[0] );


    /*
     FIXME!!! The struct currently does not provide the geometry information
              the loading works as required*/
    output_image->SetSpacing( mosaicInfo.spacing );
    output_image->SetOrigin( mosaic_probe->GetOutput()->GetOrigin() );
    output_image->SetDirection( dmatrix );
    output_image->SetLargestPossibleRegion( requestedRegion );
    output_image->SetBufferedRegion( requestedRegion );
    output_image->Allocate();

    itk::ImageRegionIteratorWithIndex< VectorImageType > vecIter(
          output_image, requestedRegion );

    // hold the image sizes in an extra variable ( used very often )
    typename MosaicImageType::SizeValueType dx = requestedRegion.GetSize()[0];
    typename MosaicImageType::SizeValueType dy = requestedRegion.GetSize()[1];

    // iterate over the given volumes
    unsigned int component = 0;
    while( volumesFileNamesIter != filenames.end() )
    {

      MITK_INFO << " ======== Loading volume " << component+1 << " of " << filenames.size();

      typename SingleImageReaderType::Pointer mosaic_reader = SingleImageReaderType::New();
      mosaic_reader->SetFileName( (*volumesFileNamesIter).at(0) );

      try
      {
        mosaic_reader->UpdateLargestPossibleRegion();
      }
      catch( const itk::ExceptionObject &e)
      {
        mitkThrow() << " ITK Image file reader failed : "<< e.what();
      }

      typename MosaicImageType::Pointer current_mosaic = mosaic_reader->GetOutput();

      vecIter.GoToBegin();
      while( !vecIter.IsAtEnd() )
      {
        typename VectorImageType::PixelType vector_pixel = vecIter.Get();
        typename VectorImageType::IndexType threeD_index = vecIter.GetIndex();

        typename MosaicImageType::IndexType mosaic_index;

        mosaic_index[2] = 1;

        // first find the corresponding tile in the mosaic
        // this is defined by the z-position of the vector (3D) image iterator
        //      in x : z_index % #images_in_grid
        //      in y : z_index / #images_in_grid
        //
        // the remaining is just computing the correct position in the mosaic, done by
        //                --------- index of (0,0,z) -----        + --- current 2d position ---
        mosaic_index[0] = (threeD_index[2] % images_per_row) * dx + threeD_index[0] + images_per_row;
        mosaic_index[1] = (threeD_index[2] / images_per_row) * dy + threeD_index[1];

        typename MosaicImageType::PixelType mosaic_pixel = current_mosaic->GetPixel( mosaic_index );

        vector_pixel.SetElement( component, mosaic_pixel );

        vecIter.Set( vector_pixel );

        ++vecIter;
      }

      ++volumesFileNamesIter;
      component++;

    }

    return output_image;

  }

};


}

#endif // MITKDIFFUSIONDICOMFILEREADERHELPER_H
