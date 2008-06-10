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

#if !defined(@FUNCTIONALITY_NAME@_H__INCLUDED)
#define @FUNCTIONALITY_NAME@_H__INCLUDED

#include "QmitkFunctionality.h"
#include "mitkTestingConfig.h"

class QmitkStdMultiWidget;
class @FUNCTIONALITY_NAME@Controls;

/*!
\brief @FUNCTIONALITY_NAME@ 

One needs to reimplement the methods CreateControlWidget(..), CreateMainWidget(..) 
and CreateAction(..) from QmitkFunctionality. 

\sa QmitkFunctionality
\ingroup Functionalities
*/
class @FUNCTIONALITY_NAME@ : public QmitkFunctionality
{  
  Q_OBJECT
  
  public:  
  /*!  
  \brief default constructor  
  */  
  @FUNCTIONALITY_NAME@(QObject *parent=0, const char *name=0, QmitkStdMultiWidget *mitkStdMultiWidget = NULL, mitk::DataTreeIteratorBase* dataIt = NULL);

  /*!  
  \brief default destructor  
  */  
  virtual ~@FUNCTIONALITY_NAME@();

  /*!  
  \brief method for creating the widget containing the application controls, like sliders, buttons etc.  
  */  
  virtual QWidget * CreateControlWidget(QWidget *parent);

  /*!  
  \brief method for creating the applications main widget  
  */  
  virtual QWidget * CreateMainWidget(QWidget * parent);

  /*!  
  \brief method for creating the connections of main and control widget  
  */  
  virtual void CreateConnections();

  /*!  
  \brief method for creating an QAction object, i.e. button & menu entry  @param parent the parent QWidget  
  */  
  virtual QAction * CreateAction(QActionGroup *parent);

  virtual void Activated();

#ifdef BUILD_TESTING
  /**
  \brief This method performs an automated functionality test. 
  
  Will be called by testing subsystem. Do not call it yourself!
  The method should be implemented in its own cpp file 'QmitkMyFunctionalityTesting.cpp'.
  */
  virtual int TestYourself();
#endif

protected slots:  
  void TreeChanged();
  /*
   * just an example slot for the example TreeNodeSelector widget
   */
  void ImageSelected(mitk::DataTreeIteratorClone imageIt);
  
  /*
   * just an example slot for the example "Start!" button
   */
   void StartButtonClicked();

protected:  
  /*!  
  * default main widget containing 4 windows showing 3   
  * orthogonal slices of the volume and a 3d render window  
  */  
  QmitkStdMultiWidget * m_MultiWidget;

  /*!  
  * controls for the functionality. Sliders, Buttons, TreenodeSelectors,...  
  */  
  @FUNCTIONALITY_NAME@Controls * m_Controls;

};
#endif // !defined(@FUNCTIONALITY_NAME@_H__INCLUDED)
