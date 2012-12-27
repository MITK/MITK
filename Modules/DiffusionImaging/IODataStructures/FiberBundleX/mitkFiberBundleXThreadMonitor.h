
/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef _MITK_FiberBundleXThreadMonitor_H
#define _MITK_FiberBundleXThreadMonitor_H

//includes for MITK datastructure
#include "mitkBaseData.h"
#include "MitkDiffusionImagingExports.h"
#include <QString>

#include <QMutex>


namespace mitk {

  /**
   * \brief Base Class for Fiber Bundles;   */
  class  MitkDiffusionImaging_EXPORT FiberBundleXThreadMonitor : public BaseData
  {

  public:

    // ======virtual methods must have======
    virtual void UpdateOutputInformation();
    virtual void SetRequestedRegionToLargestPossibleRegion();
    virtual bool RequestedRegionIsOutsideOfTheBufferedRegion();
    virtual bool VerifyRequestedRegion();
    virtual void SetRequestedRegion( itk::DataObject *data );
    //=======================================

    mitkClassMacro( FiberBundleXThreadMonitor, BaseData );
    itkNewMacro( Self );


    void setTextL1(QString);
    QString getTextL1();

    QString         getBracketOpen();
    mitk::Point2D   getBracketOpenPosition();
    void            setBracketOpenPosition(mitk::Point2D);
    int             getBracketOpenOpacity(); // range 0 - 10,  multiplicationfactor 0.1 (in mapper)

    QString         getBracketClose();
    mitk::Point2D   getBracketClosePosition();
    void            setBracketClosePosition(mitk::Point2D);
    int             getBracketCloseOpacity(); // range 0 - 10,  multiplicationfactor 0.1 (in mapper)

    QString         getHeading();
    mitk::Point2D   getHeadingPosition();
    void            setHeadingPosition(mitk::Point2D);
    int             getHeadingOpacity(); // range 0 - 10,  multiplicationfactor 0.1 (in mapper)
    void            setHeadingOpacity(int);

    QString         getMask();
    mitk::Point2D   getMaskPosition();
    void            setMaskPosition(mitk::Point2D);
    int             getMaskOpacity(); // multiplicationfactor 0.1 (in mapper)
    void            setMaskOpacity(int);

    QString         getStatus();
    void            setStatus(QString);
    mitk::Point2D   getStatusPosition();
    void            setStatusPosition(mitk::Point2D);
    int             getStatusOpacity(); // multiplicationfactor 0.1 (in mapper)
    void            setStatusOpacity(int);

    int             getStarted();
    void            setStarted(int);
    mitk::Point2D   getStartedPosition();
    void            setStartedPosition(mitk::Point2D);
    int             getStartedOpacity(); // multiplicationfactor 0.1 (in mapper)
    void            setStartedOpacity(int);

    int             getFinished();
    void            setFinished(int);
    mitk::Point2D   getFinishedPosition();
    void            setFinishedPosition(mitk::Point2D);
    int             getFinishedOpacity(); // multiplicationfactor 0.1 (in mapper)
    void            setFinishedOpacity(int);

    int             getTerminated();
    void            setTerminated(int);
    mitk::Point2D   getTerminatedPosition();
    void            setTerminatedPosition(mitk::Point2D);
    int             getTerminatedOpacity(); // multiplicationfactor 0.1 (in mapper)
    void            setTerminatedOpacity(int);


  protected:
    FiberBundleXThreadMonitor();
    virtual ~FiberBundleXThreadMonitor();


  private:
    QString m_monitorBracketOpen;
    mitk::Point2D m_monitorBracketOpenPosition;

    QString m_monitorBracketClose;
    mitk::Point2D m_monitorBracketClosePosition;

    QString m_monitorHeading;
    mitk::Point2D m_monitorHeadingPosition;
    int m_monitorHeadingOpacity;

    QString m_monitorMask;
    mitk::Point2D m_monitorMaskPosition;
    int m_monitorMaskOpacity;

    QString m_monitorStatus;
    mitk::Point2D m_monitorStatusPosition;
    int m_monitorStatusOpacity;

    int m_monitorStarted;
    mitk::Point2D m_monitorStartedPosition;
    int m_monitorStartedOpacity;

    int m_monitorFinished;
    mitk::Point2D m_monitorFinishedPosition;
    int m_monitorFinishedOpacity;

    int m_monitorTerminated;
    mitk::Point2D m_monitorTerminatedPosition;
    int m_monitorTerminatedOpacity;

    QMutex m_startedMutex;
    QMutex m_finishedMutex;
    QMutex m_terminatedMutex;
    QMutex m_statusMutex;





  };

} // namespace mitk

#endif /*  _MITK_FiberBundleX_H */
