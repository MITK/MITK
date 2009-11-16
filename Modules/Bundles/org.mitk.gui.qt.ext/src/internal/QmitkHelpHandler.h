/*=========================================================================

 Program:   openCherry Platform
 Language:  C++
 Date:      $Date: 2009-10-23 02:14:03 +0200 (Fr, 23 Okt 2009) $
 Version:   $Revision: 19649 $

 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/

#include <QObject>
 
/** \class QmitkHelpHandler
 *  \brief An object of this class will, once initialized, catch an event if the F1-key is pressed by the user.
 *         It will open the mitk-help if it isn't already open and redirect to the help page of the currently active bundle.
 */
 class QmitkHelpHandler : public QObject
 	{
      Q_OBJECT

      public:
      QmitkHelpHandler();

 	    protected:
      bool eventFilter(QObject *obj, QEvent *event);
 	};