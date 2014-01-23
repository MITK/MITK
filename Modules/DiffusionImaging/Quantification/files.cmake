set(CPP_FILES

  # DataStructures -> Tbss
  IODataStructures/TbssImages/mitkTbssImageSource.cpp
  IODataStructures/TbssImages/mitkTbssRoiImageSource.cpp
  IODataStructures/TbssImages/mitkNrrdTbssImageReader.cpp
  IODataStructures/TbssImages/mitkNrrdTbssRoiImageReader.cpp
  IODataStructures/TbssImages/mitkTbssImage.cpp
  IODataStructures/TbssImages/mitkTbssRoiImage.cpp
  IODataStructures/TbssImages/mitkNrrdTbssImageWriter.cpp
  IODataStructures/TbssImages/mitkNrrdTbssRoiImageWriter.cpp
  IODataStructures/TbssImages/mitkTbssImporter.cpp

  # Rendering
  Rendering/mitkTbssImageMapper.cpp

  #Algorithms
  Algorithms/mitkTractAnalyzer.cpp
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
