/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date: 2009-05-05 11:31:02 +0200 (Di, 05 Mai 2009) $
Version:   $Revision: 10185 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef QmitkDiffusionDicomImportView_H__INCLUDED
#define QmitkDiffusionDicomImportView_H__INCLUDED

#include "QmitkFunctionality.h"
#include "ui_QmitkDiffusionDicomImportViewControls.h"


/*!
\brief QmitkDiffusionDicomImport 

\sa QmitkFunctionality
\ingroup Functionalities
*/
class QmitkDiffusionDicomImport : public QmitkFunctionality
{  
  Q_OBJECT

public:  

  static const std::string VIEW_ID;

  /*! 
  \  Convenient typedefs
  */
  typedef mitk::DataStorage::SetOfObjects  ConstVector;
  typedef ConstVector::ConstPointer        ConstVectorPointer;
  typedef ConstVector::ConstIterator       ConstVectorIterator;

  /*!  
  \brief default constructor  
  */  
  QmitkDiffusionDicomImport(QObject *parent=0, const char *name=0);
  QmitkDiffusionDicomImport(const QmitkDiffusionDicomImport& other);

  /*!  
  \brief default destructor  
  */  
  virtual ~QmitkDiffusionDicomImport();

  /*!  
  \brief method for creating the widget containing the application   controls, like sliders, buttons etc.  
  */  
  virtual void CreateQtPartControl(QWidget *parent);

  /*!  
  \brief method for creating the connections of main and control widget  
  */  
  virtual void CreateConnections();

  virtual void Activated();

  void SetDwiNodeProperties(mitk::DataNode::Pointer node, std::string name);

protected slots:

  void DicomLoadAddFolderNames();

  void DicomLoadDeleteFolderNames();

  void DicomLoadStartLoad() ;

  void AverageClicked();

  void OutputSet();
  void OutputClear();

  void AdvancedCheckboxClicked();

  void Remove();

protected: 

  void Status(QString status);
  void Status(std::string status);
  void Status(const char* status);
  void Error(QString status);
  void Error(std::string status);
  void Error(const char* status);

  void PrintMemoryUsage();
  std::string FormatMemorySize( size_t size );
  std::string FormatPercentage( double val );
  std::string GetMemoryDescription( size_t processSize, float percentage );

  /*!  
  * controls containing sliders for scrolling through the slices  
  */  
  Ui::QmitkDiffusionDicomImportControls *m_Controls;

  QmitkStdMultiWidget* m_MultiWidget;

  QWidget *m_Parent;

  QString m_OutputFolderName;
  bool m_OutputFolderNameSet;

};

#endif // !defined(QmitkDiffusionDicomImport_H__INCLUDED)


