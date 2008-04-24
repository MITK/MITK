/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#if !defined(QmitkSliceBasedSegmentation_H__INCLUDED)
#define QmitkSliceBasedSegmentation_H__INCLUDED

#include "QmitkFunctionality.h"

#include "mitkTestingConfig.h" // IMPORTANT: this defines or undefines BUILD_TESTING !

#include <string>

class QmitkStdMultiWidget;
class QmitkSliceBasedSegmentationControls;

/**
  \brief Slice based segmentation of 3D and 3D+t images.
  
  \sa QmitkToolSelectionBox  
  \sa QmitkToolReferenceDataSelectionBox   
  \sa QmitkToolWorkingDataListBox   
  \sa mitk::ToolManager

  \ingroup Functionalities
  \ingroup Reliver

  There is a separate page describing the general design of QmitkSliceBasedSegmentation: \li \ref QmitkSliceBasedSegmentationTechnicalPage

  Implemented for the ReLiver application, requirements are from this project. 
  QmitkSliceBasedSegmentation is mainly a restructured replacement for ERISBase.

  Provides a set of interactive tool for segmentation in 2D slices (slices of 3D or 3D+t images).
  Technically it is based on mitk::ToolManager and the related classes mitk::Tool, QmitkToolSelectionBox, QmitkToolDataSelectionBox.

  Very slim implementation, most of the job is done by the widgets used in the GUI.

\verbatim
  Last contributor: $Author$
\endverbatim
*/
class QmitkSliceBasedSegmentation : public QmitkFunctionality
{  
  Q_OBJECT
  
  public:  
  
    QmitkSliceBasedSegmentation(QObject *parent=0, const char *name=0, QmitkStdMultiWidget *mitkStdMultiWidget = NULL, mitk::DataTreeIteratorBase* dataIt = NULL);

    virtual ~QmitkSliceBasedSegmentation();

    /**
      \brief from QmitkFunctionality
    */
    virtual QWidget * CreateControlWidget(QWidget *parent);

    /**
      \brief from QmitkFunctionality
    */
    virtual QWidget * CreateMainWidget(QWidget * parent);

    /**
      \brief from QmitkFunctionality
    */
    virtual void CreateConnections();

    /**
      \brief from QmitkFunctionality
    */
    virtual QAction * CreateAction(QActionGroup *parent);

    /**
      \brief from QmitkFunctionality
    */
    virtual void Activated();

    /**
      \brief from QmitkFunctionality
    */
    virtual void Deactivated();

    void OnNodePropertiesChanged();
    void OnNewNodesGenerated();

#ifdef BUILD_TESTING
    /**
      \brief Testing entry point
    */
    virtual bool TestYourself();

    /**
       \brief helper method for testing
    */
    bool TestAllTools();

    /**
       \brief helper method for testing
    */
    bool TestInterpolation();
    
  protected slots:
    /**
       \brief helper method for testing
    */
    void TestOnNewSegmentationDialogFound( QWidget* widget );

  public:
#else
    // slot function is needed, because moc ignores our #ifdefs
    void TestOnNewSegmentationDialogFound( QWidget* widget ) {}
#endif

  protected slots:  

    /**
      \brief from QmitkFunctionality
    */
    void TreeChanged();

    void ReinitializeToImage();
    void CreateNewSegmentation();
    void LoadSegmentation();
    void DeleteSegmentation();
    void SaveSegmentation();
    void SetReferenceImagePixelSmoothing(bool on);

    void OnToolSelected(int id);
    void OnReferenceNodeSelected(const mitk::DataTreeNode*);

  protected:

    /** TODO bug #1342: should be more central, clearly not in functionality **/
    mitk::DataTreeNode::Pointer CreateEmptySegmentationNode( mitk::Image* image, const std::string& organType, const std::string name );
    mitk::DataTreeNode::Pointer CreateSegmentationNode( mitk::Image* image, const std::string& name, const std::string& organType );
    
    /**
      \brief Main widget showing the scene
    */
    QmitkStdMultiWidget * m_MultiWidget;
    
    /**
      \brief This functionality's GUI
    */
    QmitkSliceBasedSegmentationControls * m_Controls;
};
#endif // !defined(QmitkSliceBasedSegmentation_H__INCLUDED)

