set(CPP_FILES

  # DataStructures -> Tbss
  IODataStructures/TbssImages/mitkTbssImageSource.cpp
  IODataStructures/TbssImages/mitkTbssRoiImageSource.cpp
  IODataStructures/TbssImages/mitkNrrdTbssImageReader.cpp
  IODataStructures/TbssImages/mitkNrrdTbssImageIOFactory.cpp
  IODataStructures/TbssImages/mitkNrrdTbssRoiImageReader.cpp
  IODataStructures/TbssImages/mitkNrrdTbssRoiImageIOFactory.cpp
  IODataStructures/TbssImages/mitkTbssImage.cpp
  IODataStructures/TbssImages/mitkTbssRoiImage.cpp
  IODataStructures/TbssImages/mitkNrrdTbssImageWriter.cpp
  IODataStructures/TbssImages/mitkNrrdTbssImageWriterFactory.cpp
  IODataStructures/TbssImages/mitkNrrdTbssRoiImageWriter.cpp
  IODataStructures/TbssImages/mitkNrrdTbssRoiImageWriterFactory.cpp
  IODataStructures/TbssImages/mitkTbssImporter.cpp

  # DataStructures
  IODataStructures/mitkQuantificationObjectFactory.cpp

  # Rendering
  Rendering/mitkTbssImageMapper.cpp
)

set(H_FILES
  # DataStructures
  IODataStructures/TbssImages/mitkTbssImporter.h

  # Rendering
  Rendering/mitkTbssImageMapper.h

  Algorithms/itkRegularizedIVIMLocalVariationImageFilter.h
  Algorithms/itkRegularizedIVIMReconstructionFilter.h
  Algorithms/itkRegularizedIVIMReconstructionSingleIteration.h
)