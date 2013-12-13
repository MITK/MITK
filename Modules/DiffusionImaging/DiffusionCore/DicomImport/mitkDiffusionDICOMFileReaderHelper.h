#ifndef MITKDIFFUSIONDICOMFILEREADERHELPER_H
#define MITKDIFFUSIONDICOMFILEREADERHELPER_H

#include "mitkDiffusionImage.h"

#include "itkImageSeriesReader.h"
#include "itkVectorImage.h"

namespace mitk
{

class DiffusionDICOMFileReaderHelper
{
public:
  typedef std::vector< std::string > StringContainer;

  typedef std::vector< StringContainer > VolumeFileNamesContainer;



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
};


}

#endif // MITKDIFFUSIONDICOMFILEREADERHELPER_H
