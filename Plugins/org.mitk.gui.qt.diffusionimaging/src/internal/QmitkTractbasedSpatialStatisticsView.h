/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2010-03-31 16:40:27 +0200 (Mi, 31 Mrz 2010) $
Version:   $Revision: 21975 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef QmitkTractbasedSpatialStatisticsView_h
#define QmitkTractbasedSpatialStatisticsView_h


#include <berryISelectionListener.h>
#include <berryIPartListener.h>
#include <berryIStructuredSelection.h>


#include <QmitkFunctionality.h>

#include "ui_QmitkTractbasedSpatialStatisticsViewControls.h"
#include <QListWidgetItem>

#include <mitkDiffusionImage.h>
#include <string>

#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>
#include <mitkDataNodeFactory.h>

#include <itkDiffusionTensor3D.h>

#include <mitkTbssImage.h>
#include <mitkTbssRoiImage.h>

#include "QmitkTbssTableModel.h"
#include "QmitkTbssMetaTableModel.h"




typedef short DiffusionPixelType;
typedef itk::Image<char, 3> CharImageType;
typedef itk::Image<unsigned char, 3> UCharImageType;
typedef itk::Image<float, 4> Float4DImageType;
typedef itk::Image<float, 3> FloatImageType;
typedef itk::Vector<int, 3> IntVectorType;
//typedef itk::VectorImage<int, 3> DirectionImageType;
typedef itk::VectorImage<float, 3> VectorImageType;


typedef itk::ImageFileReader< CharImageType > CharReaderType;
typedef itk::ImageFileReader< UCharImageType > UCharReaderType;
typedef itk::ImageFileWriter< CharImageType > CharWriterType;
typedef itk::ImageFileReader< FloatImageType > FloatReaderType;
typedef itk::ImageFileWriter< FloatImageType > FloatWriterType;
typedef itk::ImageFileReader< Float4DImageType > Float4DReaderType;
typedef itk::ImageFileWriter< Float4DImageType > Float4DWriterType;


struct TbssSelListener;



/*!
  \brief QmitkTractbasedSpatialStatisticsView

  \warning  This application module is not yet documented. Use "svn blame/praise/annotate" and ask the author to provide basic documentation.

  \sa QmitkFunctionalitymitkTbssWorkspaceManager
  \ingroup Functionalities
*/
class QmitkTractbasedSpatialStatisticsView : public QmitkFunctionality
{


  friend struct TbssSelListener;



  // this is needed for all Qt objesetupUicts that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

  public:

    static const std::string VIEW_ID;

    QmitkTractbasedSpatialStatisticsView();
    virtual ~QmitkTractbasedSpatialStatisticsView();

    virtual void CreateQtPartControl(QWidget *parent);

    /// \brief Creation of the connections of main and control widget
    virtual void CreateConnections();

    virtual void StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget);
    virtual void StdMultiWidgetNotAvailable();

    /// \brief Called when the functionality is activated
    virtual void Activated();

    virtual void Deactivated();


  protected slots:


    //void OutputValues();



   // void InitializeGridByVectorImage();

    void Masking();



    void CreateRoi();


    void Clustering();

    void AdjustPlotMeasure(const QString & text);

    void Clicked(const QwtDoublePoint& pos);

    void TbssImport();


    void AddGroup();
    void RemoveGroup();

    void CopyToClipboard();



  protected:

    /// \brief called by QmitkFunctionality when DataManager's selection has changed
    virtual void OnSelectionChanged( std::vector<mitk::DataNode*> nodes );

    void Plot(mitk::TbssImage*, mitk::TbssRoiImage*);

    void InitPointsets();

    void SetDefaultNodeProperties(mitk::DataNode::Pointer node, std::string name);

    berry::ISelectionListener::Pointer m_SelListener;
    berry::IStructuredSelection::ConstPointer m_CurrentSelection;

    bool m_IsInitialized;

    mitk::PointSet::Pointer m_PointSetNode;
    mitk::DataNode::Pointer m_P1;

    Ui::QmitkTractbasedSpatialStatisticsViewControls* m_Controls;

    QmitkStdMultiWidget* m_MultiWidget;


    std::vector<CharImageType::IndexType> SortPoints(CharImageType::Pointer roi, CharImageType::IndexType currentPoint);

    bool PointVisited(std::vector<CharImageType::IndexType> points, CharImageType::IndexType point);

    // Modifies the current point by reference and returns true if no more points need to be visited
    CharImageType::IndexType FindNextPoint(std::vector<CharImageType::IndexType> pointsVisited,
      CharImageType::IndexType currentPoint, CharImageType::Pointer roi, bool &ready);




    //void DoInitializeGridByVectorImage(FloatVectorImageType::Pointer vectorpic, CharImageType::Pointer roi ,std::string name);



    // Tokenizer needed for the roi files
    void Tokenize(const std::string& str,
                  std::vector<std::string>& tokens,
                  const std::string& delimiters = " ")
    {
      // Skip delimiters at beginning.
      std::string::size_type lastPos = str.find_first_not_of(delimiters, 0);
      // Find first "non-delimiter".
      std::string::size_type pos     = str.find_first_of(delimiters, lastPos);

      while (std::string::npos != pos || std::string::npos != lastPos)
      {
          // Found a token, add it to the vector.
          tokens.push_back(str.substr(lastPos, pos - lastPos));
          // Skip delimiters.  Note the "not_of"
          lastPos = str.find_first_not_of(delimiters, pos);
          // Find next "non-delimiter"
          pos = str.find_first_of(delimiters, lastPos);
      }
    }

    mitk::DataNode::Pointer readNode(std::string f)
    {
      mitk::DataNode::Pointer node;
      mitk::DataNodeFactory::Pointer nodeReader = mitk::DataNodeFactory::New();
      try
      {
        nodeReader->SetFileName(f);
        nodeReader->Update();
        node = nodeReader->GetOutput();
      }
      catch(...) {
        MITK_ERROR << "Could not read file";
        return NULL;
      }

      return node;
    }

    /*template < typename TPixel, unsigned int VImageDimension >
    void ToITK4D( itk::Image<TPixel, VImageDimension>* inputImage, Float4DImageType::Pointer& outputImage );*/


    std::string ReadFile(std::string whatfile);


    std::vector< itk::Index<3> > m_Roi;

    std::string m_CurrentStructure;

    mitk::Geometry3D* m_CurrentGeometry;

    QmitkTbssTableModel* m_GroupModel;



    void AddTbssToDataStorage(mitk::Image* image, std::string name);

    mitk::TbssImage::Pointer m_CurrentTbssMetaImage;

    VectorImageType::Pointer ConvertToVectorImage(mitk::Image::Pointer mitkImg);

};



#endif // _QMITKTRACTBASEDSPATIALSTATISTICSVIEW_H_INCLUDED

