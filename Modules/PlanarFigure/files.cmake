file(GLOB_RECURSE H_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/include/*")

set(CPP_FILES
  Algorithms/mitkExtrudePlanarFigureFilter.cpp
  Algorithms/mitkImageToPlanarFigureFilter.cpp
  Algorithms/mitkPlanarFigureObjectFactory.cpp
  Algorithms/mitkPlanarFigureSource.cpp
  Algorithms/mitkPlanarFigureToPlanarFigureFilter.cpp
  DataManagement/mitkPlanarAngle.cpp
  DataManagement/mitkPlanarBezierCurve.cpp
  DataManagement/mitkPlanarCircle.cpp
  DataManagement/mitkPlanarDoubleEllipse.cpp
  DataManagement/mitkPlanarEllipse.cpp
  DataManagement/mitkPlanarCross.cpp
  DataManagement/mitkPlanarFigure.cpp
  DataManagement/mitkPlanarFourPointAngle.cpp
  DataManagement/mitkPlanarComment.cpp
  DataManagement/mitkPlanarLine.cpp
  DataManagement/mitkPlanarArrow.cpp
  DataManagement/mitkPlanarPolygon.cpp
  DataManagement/mitkPlanarSubdivisionPolygon.cpp
  DataManagement/mitkPlanarRectangle.cpp
  DataManagement/mitkPlanarFigureControlPointStyleProperty.cpp
  IO/mitkPlanarFigureIOFactory.cpp
  IO/mitkPlanarFigureReader.cpp
  IO/mitkPlanarFigureWriter.cpp
  IO/mitkPlanarFigureWriterFactory.cpp
  IO/mitkPlanarFigureSerializer.cpp
  IO/mitkPlanarFigureSubclassesSerializer.cpp
  Interactions/mitkPlanarFigureInteractor.cpp
  Rendering/mitkPlanarFigureMapper2D.cpp
  Rendering/mitkPlanarFigureVtkMapper3D.cpp
)

set(RESOURCE_FILES
  Interactions/PlanarFigureConfig.xml
  Interactions/PlanarFigureInteraction.xml
)
