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

#if !defined(QmitkInteractiveSegmentation_H__INCLUDED)
#define QmitkInteractiveSegmentation_H__INCLUDED

#include "QmitkFunctionality.h"

#include "mitkTestingConfig.h" // IMPORTANT: this defines or undefines BUILD_TESTING !

#include <string>

class QmitkStdMultiWidget;
class QmitkInteractiveSegmentationControls;

/**
  \brief Slice based segmentation of 3D and 3D+t images.
  
  \sa QmitkToolSelectionBox  
  \sa QmitkToolReferenceDataSelectionBox   
  \sa QmitkToolWorkingDataSelectionBox   
  \sa mitk::ToolManager

  \ingroup Functionalities
  \ingroup Reliver

  There is a separate page describing the general design of QmitkInteractiveSegmentation: \li \ref QmitkInteractiveSegmentationTechnicalPage

  Implemented for the ReLiver application, requirements are from this project. 
  QmitkInteractiveSegmentation is mainly a restructured replacement for ERISBase.

  Provides a set of interactive tool for segmentation in 2D slices (slices of 3D or 3D+t images).
  Technically it is based on mitk::ToolManager and the related classes mitk::Tool, QmitkToolSelectionBox, QmitkToolDataSelectionBox.

  Very slim implementation, most of the job is done by the widgets used in the GUI.

\verbatim
  Last contributor: $Author$
\endverbatim
*/
class QmitkInteractiveSegmentation : public QmitkFunctionality
{  
  Q_OBJECT
  
  public:  
  
    QmitkInteractiveSegmentation(QObject *parent=0, const char *name=0, QmitkStdMultiWidget *mitkStdMultiWidget = NULL, mitk::DataTreeIteratorBase* dataIt = NULL);

    virtual ~QmitkInteractiveSegmentation();

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
    virtual int TestYourself();

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
    void LoadSegmentationPACS();
    void DeleteSegmentation();
    void SaveSegmentation();
    void SaveSegmentationPACS();
    void SetReferenceImagePixelSmoothing(bool on);

    void OnToolSelected(int id);
    void OnReferenceNodeSelected(const mitk::DataTreeNode*);

  protected:

    void CheckImageAlignment(mitk::Image* image);
    
    /**
      \brief Main widget showing the scene
    */
    QmitkStdMultiWidget * m_MultiWidget;
    
    /**
      \brief This functionality's GUI
    */
    QmitkInteractiveSegmentationControls * m_Controls;
};
#endif // !defined(QmitkInteractiveSegmentation_H__INCLUDED)

