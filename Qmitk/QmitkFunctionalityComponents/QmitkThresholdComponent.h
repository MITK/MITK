

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
#ifndef MITK_THRESHOLDCOMPONENT_H
#define MITK_THRESHOLDCOMPONENT_H

#include "QmitkFunctionalityComponentContainer.h"
class QmitkThresholdComponentGUI;

/**
* \brief ComponentClass to find an adequate threshold for a selected image
* \ingroup QmitkFunctionalityComponent
* 
* This class inherits from
* - \ref QmitkFunctionalityComponentContainer

* 
* \section QmitkThresholdComponent Overview
* 
* The ThresholdComponent is a class to easy find an adequate threshold. On two ways the user can insert 
* a possible threshold: as a number in a textfield or by moving a scrollbar. The selected threshold will 
* be shown on the image as an new binary layer that contains those areas above the theshold.
* Like all other componentes the QmitkThresholdComponent inherits from QmitkFunctionalityComponentContainer.
* 
*/


class QmitkThresholdComponent : public QmitkFunctionalityComponentContainer
{

  Q_OBJECT

public:
  /***************       CONSTRUCTOR      ***************/
  /** \brief Constructor. */
  QmitkThresholdComponent(QObject *parent=0, const char *name=0, mitk::DataTreeIteratorBase* dataIt = NULL);

  /***************       CONSTRUCTOR      ***************/
  /** \brief Constructor to set some preferences. */
  QmitkThresholdComponent(QObject *parent, const char *name, mitk::DataTreeIteratorBase* dataIt, bool updateSelector, bool showSelector);

  /***************        DESTRUCTOR      ***************/
  /** \brief Destructor. */
  ~QmitkThresholdComponent();

  /***************        CREATE          ***************/
  QWidget* CreateContainerWidget(QWidget* parent);
  virtual void CreateConnections();
  virtual void TreeChanged();
  virtual void TreeChanged(mitk::DataTreeIteratorBase* it);



  //Set and Get
  void SetDataTreeIterator(mitk::DataTreeIteratorBase* it);
  void SetFunctionalityName(QString name);
  mitk::DataTreeIteratorBase* GetDataTreeIterator();
  virtual QString GetFunctionalityComponentName();
  virtual QString GetFunctionalityName();
  QWidget* GetGUI();

  /***************        ATTRIBUTES      ***************/
  /** \brief Vector with all added components */
  std::vector<QmitkFunctionalityComponentContainer*> m_AddedChildList; 

public slots:  
  /***************      OHTER METHODS     ***************/
  /** \brief Slot method that will be called if TreeNodeSelector widget was activated. */
  virtual void ImageSelected(const mitk::DataTreeFilter::Item * imageIt);
  void ShowThreshold();
  void ThresholdSliderChanged();
  void ThresholdValueChanged( );

protected:
  /***************        ATTRIBUTES      ***************/
  QString m_Name;
  /*!
  a reference to a data tree iterator object
  */
  mitk::DataTreeIteratorClone m_DataTreeIteratorClone;
  mitk::DataTreeNode::Pointer m_Node;
  mitk::DataTreeNode::Pointer m_ThresholdImageNode;
  /** \brief Attribute to decide whether the selector shall be updated when a parent-Selector is updatet or not */
  bool m_UpdateSelector;
  /** \brief Attribute to decide whether the selector shall be shown or not */
  bool m_ShowSelector;


private:
  QmitkThresholdComponentGUI * m_GUI;
  const mitk::DataTreeFilter::Item * m_SelectedImage; 
  void CreateThresholdImageNode();


};

#endif

