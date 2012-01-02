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

#ifndef QmitDicomLocalStorageWidget_h
#define QmitDicomLocalStorageWidgett_h

// #include <QmitkFunctionality.h>
#include "ui_QmitDicomLocalStorageWidgetControls.h"
#include <QWidget>


/*!
\brief QmitDicomLocalStorageWidget 

\warning  This application module is not yet documented. Use "svn blame/praise/annotate" and ask the author to provide basic documentation.

\sa QmitkFunctionality
\ingroup Functionalities
*/
class MITK_DICOMUI_EXPORT QmitDicomLocalStorageWidget : public QWidget
{  
   // this is needed for all Qt objects that should have a Qt meta-object
   // (everything that derives from QObject and wants to have signal/slots)
   Q_OBJECT

public:  

   static const std::string Widget_ID;

   QmitDicomLocalStorageWidget(QWidget *parent);
   virtual ~QmitDicomLocalStorageWidget();

   virtual void CreateQtPartControl(QWidget *parent);

   /* @brief   Initializes the widget. This method has to be called before widget can start. 
   * @param dataStorage The data storage the widget should work with.
   * @param multiWidget The corresponding multiwidget were the ct Image is displayed and the user should define his path.
   * @param imageNode  The image node which will be the base of mitral processing
   */
   void Initialize();



   protected slots:


protected:


   Ui::QmitDicomLocalStorageWidget* m_Controls;

   // Performs a starbust on inputimage, which results in outputimage. Only workds with 3D and 4D Ultrasound images (char Pixeltype)
   //void PerformStarburst(mitk::Image::Pointer inputImage, mitk::Image::Pointer &outputImage, mitk::Point3D startPoint, bool doubleStarburst, bool thinStarburst);



};



#endif // _QmitDicomLocalStorageWidget_H_INCLUDED

