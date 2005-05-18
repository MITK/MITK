#include "mitkConfig.h"
#ifdef MBI_INTERNAL
  #include "mitkVesselGraphData.h"
  #include "mitkVesselGraphFileWriter.h"
#endif
#define QMITKCOMMONFUNCTIONALITYIMPLEMENTATION
#include "QmitkCommonFunctionality.h"

#ifdef MBI_INTERNAL
  /**
   * Saves the given directed vessel graph to a file. If no name is provided, the
   * user is prompted for a file name.
   */
  void CommonFunctionality::SaveDirectedVesselGraph( mitk::DirectedVesselGraphData* graph, std::string name )
  {
    if(graph == NULL)
    {
      std::cout << "Warning in file " << __FILE__<< " line " << __LINE__ <<": vessel graph is NULL!" << std::endl;
      return;
    }
    QString fileName = name.c_str();
    if (fileName == "")
    {
        fileName = QFileDialog::getSaveFileName(QString("VesselGraph.dvg"),"MITK VesselGraph (*.dvg)");
    }
    if (fileName != NULL ) 
    {
      mitk::VesselGraphFileWriter<Directed>::Pointer writer = mitk::VesselGraphFileWriter<Directed>::New();
      writer->SetInput( graph );
      writer->SetFileName( fileName.ascii() );
      writer->Update();
    }
  }

  /**
   * Saves the given undirected vessel graph to a file. If no name is provided, the
   * user is prompted for a file name.
   */
  void CommonFunctionality::SaveUndirectedVesselGraph( mitk::UndirectedVesselGraphData* graph, std::string name)
  {
    if(graph == NULL)
    {
      std::cout << "Warning in file " << __FILE__<< " line " << __LINE__ <<": vessel graph is NULL!" << std::endl;
      return;
    }
    QString fileName = name.c_str();
    if (fileName == "")
    {
        fileName = QFileDialog::getSaveFileName(QString("VesselGraph.uvg"),"MITK VesselGraph (*.uvg)");
    }
    if (fileName != NULL ) 
    {
      mitk::VesselGraphFileWriter<Undirected>::Pointer writer = mitk::VesselGraphFileWriter<Undirected>::New();
      writer->SetInput( graph );
      writer->SetFileName( fileName.ascii() );
      writer->Update();
    }
  }
#endif

  /**
   * Saves the given mitk::BaseData to a file. The user is prompted to
   * enter a file name. Currently only mitk::Image, mitk::Surface, mitk::PointSet and
   * mitk::VesselGraphData are supported. This function is deprecated
   * until the save-problem is solved by means of a Save-Factory or any
   * other "nice" mechanism
   */
  void CommonFunctionality::SaveBaseData( mitk::BaseData* data, std::string name )
  {
    if (data != NULL)
    {
      mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(data);
      if(image.IsNotNull())
      {
        typedef itk::Image<int,3> ImageType;
        CommonFunctionality::SaveImage< ImageType >(image);
      }

      mitk::PointSet::Pointer pointset = dynamic_cast<mitk::PointSet*>(data);
      if(pointset.IsNotNull())
      {
        name = itksys::SystemTools::GetFilenameWithoutExtension(name);
        name += ".mps";
        QString fileName = QFileDialog::getSaveFileName(QString(name.c_str()),"MITK Point-Sets (*.mps)");
        if (fileName != NULL )
        {
          mitk::PointSetWriter::Pointer writer = mitk::PointSetWriter::New();
          writer->SetInput( pointset );
          writer->SetFileName( fileName.ascii() );
          writer->Update();
        }
      }

      mitk::Surface::Pointer surface = dynamic_cast<mitk::Surface*>(data);
      if(surface.IsNotNull())
      {
        CommonFunctionality::SaveSurface(surface, "SurfaceModel.stl");
      }

    #ifdef MBI_INTERNAL
      mitk::UndirectedVesselGraphData::Pointer uvg = dynamic_cast<mitk::UndirectedVesselGraphData*>(data);
      if (uvg.IsNotNull())
      {
        CommonFunctionality::SaveUndirectedVesselGraph(uvg.GetPointer());
      }

      mitk::DirectedVesselGraphData::Pointer dvg = dynamic_cast<mitk::DirectedVesselGraphData*>(data);
      if (dvg.IsNotNull())
      {
        CommonFunctionality::SaveDirectedVesselGraph(dvg.GetPointer());
      }
    #endif
    }
  }
