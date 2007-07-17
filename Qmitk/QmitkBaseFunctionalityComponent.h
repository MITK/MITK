#ifndef QMITK_BASEFUNCTIONALITYCOMPONENT_H
#define QMITK_BASEFUNCTIONALITYCOMPONENT_H

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


/**
* \brief Base class for hierarchical functionality components. This class contains the DataTreeIterator, access by the usual set-and get-methods, and the name-attribut of a component
* \ingroup QmitkFunctionalityComponent
* 
* 
* 

* 
* \section BaseFunctionalityComponent Overview
* 
* The BaseFunctionalityComponent is the root-class for all component-based functionalities.
* As common ground for all inherit classes it contains the DataTreeIterator, i.e an iterator
* on the current node of the data tree, and the name-attribute of the component. 
* Inherit classes are:
* 
* - \ref QmitkAlgorithmFunctionalityComponent
* - \ref QmitkInteractionFunctionalityComponent
* - \ref QmitkFunctionalityComponentContainer
* 
*/

#include <qobject.h>
#include "QmitkBaseComponent.h"
#include "mitkDataTreeFilter.h"

class QmitkBaseFunctionalityComponent : public QmitkBaseComponent
{
  Q_OBJECT

public:
  /***************       CONSTRUCTOR      ***************/
  /** 
  * \brief Constructor. 
  */
  QmitkBaseFunctionalityComponent(QObject *parent=0, const char *name=0, mitk::DataTreeIteratorBase* dataIt = NULL);

  /***************        DESTRUCTOR      ***************/
  /** 
  * @brief Destructor. 
  */
  virtual  ~QmitkBaseFunctionalityComponent();

  /***************      OHTER METHODS     ***************/
  /** 
  * @brief Slot method that will be called if TreeNodeSelector widget was activated. 
  */
  virtual void ImageSelected(const mitk::DataTreeFilter::Item * imageIt);

  /***************        SET AND GET     ***************/
  /*!
  * @brief setter method for data tree attribute
  * @param it the reference to a data tree ieterator object
  */
  virtual void SetDataTreeIterator(mitk::DataTreeIteratorBase* it);

  /** 
  * @brief Method to set the the iterator of the selected image 
  */
  virtual void SetMitkImageIterator(mitk::DataTreeIteratorClone mitkImageIterator);

  /** 
  * @brief Method to set the DataTreeIterator
  */
  virtual void SetTreeIterator(mitk::DataTreeIteratorClone dataIt);
  
  /*!
  * @brief getter for dataTree attribute. It returns the reference to a data tree iterator object
  */
  mitk::DataTreeIteratorBase* GetDataTreeIterator();

  /*!
  * @brief A reference to a data tree iterator object.
  */
  mitk::DataTreeIteratorClone m_DataTreeIterator;

  /*!
  * @brief Image selected in MainSelector
  */
  mitk::Image* m_ParentMitkImage;

  /*!
  * @brief Image selected in MainSelector
  */
  mitk::Image* m_MitkImage;

  /*!
  * @brief Iterator on current image
  */
  mitk::DataTreeIteratorClone m_MitkImageIterator;
};
#endif

