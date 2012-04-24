
/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision: 11989 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef _MITK_ConnectomicsDegreeHistogram_H
#define _MITK_ConnectomicsDegreeHistogram_H

#include<mitkConnectomicsHistogramBase.h>

namespace mitk {

    /**
  * \brief A class to calculate and store the degree of each node   */
  class ConnectomicsDegreeHistogram : public mitk::ConnectomicsHistogramBase
  {

  public:

    ConnectomicsDegreeHistogram();
    virtual ~ConnectomicsDegreeHistogram();



  protected:

    /** @brief Creates a new histogram from the network source. */
    virtual void ComputeFromConnectomicsNetwork( ConnectomicsNetwork* source );

  };

}

#endif /* _MITK_ConnectomicsDegreeHistogram_H */