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


#ifndef MITKLEVELWINDOWMANAGER_H
#define MITKLEVELWINDOWMANAGER_H

#include <mitkDataTree.h>
#include <mitkLevelWindowProperty.h>

namespace mitk
{
class LevelWindowManager : public itk::Object {
public:

  mitkClassMacro(LevelWindowManager, itk::Object)
  itkNewMacro(Self); 

  void SetDataTreeIteratorClone(mitk::DataTreeIteratorClone& it);
  mitk::DataTreeIteratorClone& GetDataTreeIteratorClone();
  void OnPropertyModified(const itk::EventObject& e);
  void SetAutoTopMostImage();
  void SetLevWinProp(LevelWindowProperty::Pointer levWinProp);
  void SetLevelWindow(const mitk::LevelWindow& levWin);
  const LevelWindow& GetLevelWindow();
  LevelWindowProperty::Pointer GetLevWinProp();
  bool isAutoTopMost();

  void TreeChanged(const itk::EventObject& treeChangedEvent);

protected:

  LevelWindowManager();
  ~LevelWindowManager();

private:

  DataTreeIteratorClone m_DataTreeIteratorClone;
  bool m_AutoTopMost;
  unsigned long m_ObserverTag;
  bool m_IsObserverTagSet;
  unsigned long m_PropertyModifiedTag;
  bool m_IsPropertyModifiedTagSet;
  LevelWindowProperty::Pointer m_LevWinProp;
};
}

#endif
