

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
class QGroupBox;
class QCheckBox;
namespace mitk{
  class ManualSegmentationToSurfaceFilter;
}

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
  QmitkSurfaceCreatorComponent(QObject *parent=0, const char *parentName=0, QmitkStdMultiWidget *mitkStdMultiWidget = NULL, mitk::DataTreeIteratorBase* dataIt = NULL, bool updateSelector = true, bool showSelector = true, bool allowExpertMode = true);

  /***************        DESTRUCTOR      ***************/
  /** \brief Destructor. */
  virtual ~QmitkSurfaceCreatorComponent();

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




  ///** \brief Method to get the GUI of this component. This Method is obligatory */
  //virtual QWidget* GetGUI();

  /** \brief Method to set the Image Selector visible or invisible */
  virtual void SetSelectorVisibility(bool visibility);

  /** \brief Method to set the ExpertMode visible or invisible */
  virtual void SetExpertMode(bool visibility);


  /***************      (DE)ACTIVATED     ***************/

  ///** \brief Method to set m_Activated to true */
  virtual void Activated();

  ///** \brief Method to set m_Activated to false */
  virtual void Deactivated();

  /***************        ATTRIBUTES      ***************/

  /** \brief Vector with all added components */
  //std::vector<QmitkFunctionalityComponentContainer*> m_AddedChildList; 


public slots:  
  /***************      OHTER METHODS     ***************/

  /** \brief Slot method that will be called if TreeNodeSelector widget was activated to select the current image. */
  virtual void ImageSelected(const mitk::DataTreeFilter::Item * imageIt);

  /** \brief Slot method that will be called if the CheckBox at the Threshold-Group-Box was toggled to show the threshold image or not. */ 
  void ShowSurfaceCreatorContent(bool show = true);

  /** \brief Slot method that will be called if the CheckBox at the TreeNodeSelector-Group-Box was toggled to show the TreeNodeSelector or not. */ 
  void ShowImageContent(bool show = true);

  /** \brief Slot method that will be called if the CheckBox at the SurfaceCreatorCheckBox was toggled to show the SurfaceCreator or not. */ 
  void ShowCreateSurface(bool);

  /** \brief Slot method that will be called if the CheckBox ShowExpertMode was toggled to show the ExpertMode or not. */ 
  void ShowExpertMode(bool);

  /** \brief Slot method that will be called if the CheckBox ShowSurfaceParameter was toggled to show the Parameters or not. */ 
  void ShowSurfaceParameter(bool);

  /** \brief Method is called when the DataTree was changed. This Method is self-evident obligatory */
  virtual void TreeChanged();

  /** \brief Method to set the threshold that is used to create the surface */
  virtual void SetThreshold(int threshold);

  /** \brief Method to set the threshold that is used to create the surface as slot combiend with thresholdComponent*/
  void SetThreshold(const QString& threshold);

protected slots:

  //----------------------------------------------------
  /** \brief Method  to create a surface if the "create surface" button was pressed*/
  void CreateSurface();
  //----------------------------------------------------

  
  void SetMedian3DFlag(bool flag);
  void SetInterpolateFlag(bool flag);
  void SetSmoothFlag(bool flag);
  void SetGaussFlag(bool flag);
  void SetDecimateFlag(bool flag);

protected:

  /** \brief Method to push all parameterwidgets and expertwidgets in a parameterList-vector respectively in a expertModeList-vector and set them shown or not, addicted to the user-preferences at the GUI*/
  void InitSurfaceGUI();

  /** \brief Method to set all parameter Flags to true or false,  addicted to the user-preferences at the GUI*/
  void InitSurfaceParamterFlags();

  /** \brief Method to insert the created surface into datatree*/
  void InsertSurfaceIntoDataTree(mitk::ManualSegmentationToSurfaceFilter * ft, mitk::DataTreeIteratorClone  iT);

  /***************        ATTRIBUTES      ***************/

  /** \brief Vector with all Widgets from the ExpertMode */
  std::vector<QGroupBox *> m_ExpertModeList;

  /** \brief Vector with all Parameter-Widgets */
  std::vector<QCheckBox *> m_ParameterList;

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

  /** \brief Attribute to decide whether the ExpertMode can be visible or not */
  bool m_ShowExpertMode;

  /** \brief Attribute to decide whether the ExpertMode can be visible or not */
  bool m_AllowExpertMode;

  //Flags if to use Parameters or not:

  /** \brief Attribute to decide whether the Median3D-Paramter is checked an shall be used or not */
  bool m_Median3DFlag;

  /** \brief Attribute to decide whether the Interpolate-Paramter is checked an shall be used or not */
  bool m_InterpolateFlag;

  /** \brief Attribute to decide whether the Smooth-Paramter is checked an shall be used or not */
  bool m_SmoothFlag;

  /** \brief Attribute to decide whether the Gauss-Paramter is checked an shall be used or not */
  bool m_GaussFlag;

  /** \brief Attribute to decide whether the Decimate-Paramter is checked an shall be used or not */
  bool m_DecimateFlag;


private:


  /*!
  Returns the Median3DValue from the GUI-InputField
  */
  void GetMedian3DValue(int& x, int& y, int& z);

  /*!
  Returns the InterpolationValue from the GUI-InputField
  */
  void GetInterpolateValue(int& x, int& y, int& z);

  /*!
  Returns the SmoothIterationValues from the GUI-InputField
  */
  int GetSmoothIterationValue();

  /*!
  Returns the SmoothRelaxationValues from the GUI-InputField
  */
  float GetSmoothRelaxationValue();

  /*!
  Returns the GaussValue from the GUI-InputField
  */
  float GetGaussValue();

  /*!
  Returns the DecimateValue from the GUI-InputField
  */
  float GetDecimateValue();

  /***************        ATTRIBUTES      ***************/

  /** \brief The created GUI from the .ui-File. This Attribute is	obligatory*/
  QmitkSurfaceCreatorComponentGUI * m_SurfaceCreatorComponentGUI;

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

  /*!
  *  variable to set the red-value of the surface 
  */
  float m_r;
  /*!
  *  variable to set the greed-value of the surface 
  */
  float m_g;
  /*!
  *  variable to set the blue-value of the surface 
  */
  float m_b;

  /*!
  *  variable to set the color of the surface 
  */
  mitk::Color m_Color;
  /*!
  *  variable to get the next color 
  */
  mitk::ColorSequence* m_RainbowColor;

  /*!
  *  variable to hold the threshold that is used to create the surface 
  */
  int m_Threshold;

  /** \brief Spacer added at the end of the component */
  QSpacerItem* m_Spacer;


};

#endif

