

/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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
#ifndef MITK_SURFACECREATORCOMPONENT_H
#define MITK_SURFACECREATORCOMPONENT_H

#include "QmitkFunctionalityComponentContainer.h"
#include <mitkColorSequence.h>

class QmitkSurfaceCreatorComponentGUI;
class QmitkStdMultiWidget;

/**
* \brief ComponentClass to create a surface based on a threshold for a selected image
* \ingroup QmitkFunctionalityComponent
* 
* This class inherits from
* - \ref QmitkFunctionalityComponentContainer

* 
* \section QmitkSurfaceCreatorComponent Overview
* 
* The SurfaceCreatorComponent is a class to create a surface, based on a threshold. The user can input a threshold 
* (maybe found with the ThresholdComponent) and create a surface that will be added into the DataTree. The suface
* is made of polygones and can be saved as vtk or stl-Format.
* Like all other componentes the QmitkSurfaceCreatorComponent inherits from QmitkFunctionalityComponentContainer.
* 
*/


class QmitkSurfaceCreatorComponent : public QmitkFunctionalityComponentContainer
{

  Q_OBJECT

public:
  /***************       CONSTRUCTOR      ***************/
  /** \brief Constructor. */
  QmitkSurfaceCreatorComponent(QObject *parent=0, const char *name=0, QmitkStdMultiWidget *mitkStdMultiWidget = NULL, mitk::DataTreeIteratorBase* dataIt = NULL, bool updateSelector = true, bool showSelector = true);

  /***************        DESTRUCTOR      ***************/
  /** \brief Destructor. */
  ~QmitkSurfaceCreatorComponent();

  /***************        CREATE          ***************/

  /** \brief Method to create the GUI for the component from the .ui-File. This Method is obligatory */
  QWidget* CreateContainerWidget(QWidget* parent);

  /** \brief Method to create the connections for the component. This Method is obligatory even if no connections is needed*/
  virtual void CreateConnections();


  /***************      SET AND GET       ***************/

  /** \brief Method to set the Iterator to the DataTree */
  void SetDataTreeIterator(mitk::DataTreeIteratorBase* it);

  /** \brief Method to get the Iterator to the DataTree */
  mitk::DataTreeIteratorBase* GetDataTreeIterator();

  /** \brief Method to set the Name of the Functionality */
  void SetFunctionalityName(QString name);

  /** \brief Method to get the Name of the Functionality */
  virtual QString GetFunctionalityName();

  /** \brief Method to get the Name of the FunctionalityComponent */
  virtual QString GetFunctionalityComponentName();
  
  /** \brief Method to get the GUI of this component. This Method is obligatory */
  virtual QWidget* GetGUI();
   
  /** \brief Method to set the Image Selector visible or invisible */
  virtual void SetSelectorVisibility(bool visibility);

  /***************      (DE)ACTIVATED     ***************/

  ///** \brief Method to set m_Activated to true */
  virtual void Activated();

  ///** \brief Method to set m_Activated to false */
  virtual void Deactivated();

  /***************        ATTRIBUTES      ***************/

  /** \brief Vector with all added components */
  std::vector<QmitkFunctionalityComponentContainer*> m_AddedChildList; 


public slots:  
  /***************      OHTER METHODS     ***************/

  /** \brief Slot method that will be called if TreeNodeSelector widget was activated to select the current image. */
  virtual void ImageSelected(const mitk::DataTreeFilter::Item * imageIt);

  /** \brief Slot method that will be called if the CheckBox at the Threshold-Group-Box was toggled to show the threshold image or not. */ 
  void ShowSurfaceCreatorContent(bool show = true);

  /** \brief Slot method that will be called if the CheckBox at the TreeNodeSelector-Group-Box was toggled to show the TreeNodeSelector or not. */ 
  void ShowImageContent(bool show = true);

  /** \brief Method is called when the DataTree was changed. This Method is self-evident obligatory */
  virtual void TreeChanged();

  /** \brief Method to set the threshold that is used to create the surface */
  virtual void SetThreshold(int threshold);

  /** \brief Method to set the threshold that is used to create the surface */
  void SetThreshold(const QString& threshold);

protected slots:

  //----------------------------------------------------
  /** \brief Method  to create a surface if the "create surface" button was pressed*/
  void CreateSurface();
  //----------------------------------------------------

protected:
  /***************        ATTRIBUTES      ***************/

  /*!
  the name of the component
  */
  QString m_Name;

  /*!
  a reference to the MultiWidget
  */
  QmitkStdMultiWidget * m_MultiWidget;

  /*!
  a reference to a data tree iterator object
  */
  mitk::DataTreeIteratorClone m_DataTreeIteratorClone;

  /** \brief Attribute to decide whether the selector shall be updated when a parent-Selector is updatet or not */
  bool m_UpdateSelector;

  /** \brief Attribute to decide whether the selector shall be shown or not */
  bool m_ShowSelector;
 
  /** \brief Attribute to that will be set to true by the first activation of the functionality */
  bool m_Active;


private:

  /***************        ATTRIBUTES      ***************/

  /** \brief The created GUI from the .ui-File. This Attribute is	obligatory*/
  QmitkSurfaceCreatorComponentGUI * m_GUI;

  /** \brief This Attribute holds the current ImageItem from the TreeNodeSelector if his selection was changed*/
  const mitk::DataTreeFilter::Item * m_SelectedImage; 

  //----------------------------------------------------
  /*!
  * image which is used to create the surface
  */
  mitk::Image* m_MitkImage;

  /*!
  * read thresholdvalue from GUI and convert it to float
  */
  float getThreshold();

  /*!
  * iterator on current image
  */
  mitk::DataTreeIteratorClone m_MitkImageIterator;

  /*!
  *  variable to count Surfaces and hang it after the name in the DataTree 
  */
  int m_SurfaceCounter;

  //SurfaceColor
  float m_r;
  float m_g;
  float m_b;

  mitk::Color m_Color;
  mitk::ColorSequence* m_RainbowColor;
  int m_Threshold;


};

#endif

