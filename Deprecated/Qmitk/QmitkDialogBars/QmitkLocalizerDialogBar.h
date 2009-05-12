/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2008-10-17 14:11:00 +0100 (Mi, 17 Okt 2008) $
Version:   $Revision: 11316 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef QMITK_LOCALIZERDIALOGBAR_H
#define QMITK_LOCALIZERDIALOGBAR_H


#include "QmitkDialogBar.h"
#include "mitkDataTreeFilter.h"

#include "QmitkRenderWindow.h"
#include <QmitkTreeNodeSelector.h>

#include <mitkEllipsoid.h>

#include <qgrid.h>



/**
 * \brief DialogBar that shows the current position in a MIP-volume of a chosen picture
 *
 *  The LoclizerDialogBar is a widget, that when enabled shows the current planes position in a small renderwindow containing 
 *  a MIP-volume of the currently chosen picture. It is connected to the geometryupdateevent of the MultiWidgets 3 major renderwindows,
 *  so when the geometry has changed, e.g. by clicking in one of the renderwindows, the LocalizerDialogBar's UpdateGeometry() is called.
 *
 * \ingroup Functionalities
 */
class QMITK_EXPORT QmitkLocalizerDialogBar : public QmitkDialogBar
{
  Q_OBJECT

public:
  /**
  \brief default constructor
  */
  QmitkLocalizerDialogBar( QObject *parent = 0, const char *name = 0,
    QmitkStdMultiWidget *multiWidget = NULL,
    mitk::DataTreeIteratorBase* dataIt = NULL);//, mitk::DataStorage *storage = NULL );

  /**
  \brief default destructor
  */
  virtual ~QmitkLocalizerDialogBar();

  virtual QAction *CreateAction( QObject *parent );

  virtual QString GetFunctionalityName();

  /**
  \brief method is called everytime the dialogbar is set visible

    This method is called everytime the dialogbar is set visible and updates the TreeNodeSelector.
    The PointNavigationController is set and the listener added, in order to establish the interaction
    that moves the pointer according to the movements of the planes.
    The RenderWindow is initialised and the node containing the pointer is added to the DataStorage if necessary.
  */
  virtual void Activated();

  /**
    \brief method is called everytime the dialogbar is set invisible

    method removes the listener and deletes the node containing the point from the DataStorage
  */
  virtual void Deactivated();

  /**
    \brief method initialises the RenderWindow 

    method also makes the planes invisible in the RenderWindow
    
  */
  void SetViewProperties(QmitkRenderWindow *renderwindow);

  /**
    \brief is called whenever the SliceNavigationController emits an geometryupdateevent and sets the Point to the right position
  */
  void UpdateGeometry(const itk::EventObject &geometryUpdateEvent);

protected:
  QmitkTreeNodeSelector* m_TreeNodeSelector;

  QmitkRenderWindow* m_LocalizerRenderWindow;

  mitk::Ellipsoid::Pointer m_Punkt;
  
  std::string m_FileUsed;

  vtkVolume *m_Volume;

  mitk::DataTreeNode::Pointer node;
  
  QmitkRenderWindow* m_RenderWindow();

  mitk::DataTreeNode::Pointer imageNode;

  mitk::Image::Pointer mitkImage;

  vtkImageData* vtkImage;
  
  bool wasActive;

  mitk::DataStorage* m_Storage;  // hier muss ne member für die datastorage und dann in der cpp ersetzen

  QGrid* grid;
  

protected slots:
  virtual QWidget *CreateDialogBar( QWidget *parent ); 

    /**
    \brief checks if a new picture is chosen and removes the old and renders the new volume if necessary
  */
  void ImageSelected(mitk::DataTreeIteratorClone imageIt);

  /**
    \brief is called when a datatreenode is removed or added, it updates the TreeNodeSelector
  */
  void OnNewNode(const mitk::DataTreeNode* n);

   /**
    \brief sets the sphere to the point where the planes meet
  */
  void SetSeeker(mitk::Ellipsoid::Pointer m_Punkt);

  /**
    \brief needed for resetting the point when a wheelevent accurs
  */
  void SetSeeker( QWheelEvent* );

  /**
    \brief  if an appropriate picture is loaded, createVolume is called and the returned volume is added to the renderWindow
  */
  void DrawVolume();

  /**
    \brief creates the vtkVolume from the given vtkImageData
  */
  vtkVolume* createVolume(vtkImageData* vtkImage);

  /**
    \brief creates and adds the node, containing the seeker-point
  */
  void CreateNode();

  /**
    \brief sets the camera to the chosen position and zooms
  */
  void SetCamera(const QString& view);

  /**
    \brief cleans up

    removes the node containing the point from the datastorage and the volume from the renderer
  */
  void MakeClean();


};

#endif

