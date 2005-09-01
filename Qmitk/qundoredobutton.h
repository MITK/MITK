#ifndef UNDOBUTTON_H_1329857WEG
#define UNDOBUTTON_H_1329857WEG

#include <qframe.h>
#include <qwidgetplugin.h>
#include "mitkCommon.h"

class QPushButton;
class QFrame;
class QHBoxLayout;
class QListBox;
class QListBoxItem;
class PopupUndoRedoListBox;

// nice undo or redo button with popup showing the last actions
class 
MITK_EXPORT
QUndoRedoButton : public QFrame
{
  Q_OBJECT;
  Q_ENUMS( Mode );
  Q_PROPERTY( Mode mode READ getMode WRITE setMode );
  Q_PROPERTY( QPixmap pixmap READ pixmap WRITE setPixmap );
  Q_PROPERTY( QString actionName READ actionName WRITE setActionName );

  public:

    enum Mode { Undo, Redo };

    QUndoRedoButton( QWidget* parent = 0, const char* name = 0 );
    virtual ~QUndoRedoButton();
    const QPixmap *pixmap() const;
    void setPixmap( const QPixmap& pm );
    const QString& actionName() const;
    void setActionName( const QString& description );
    QListBox* listbox() const;
    
    virtual void beforePopup() {};
    
    virtual void mouseReleaseEvent(QMouseEvent*);
    
    Mode getMode() const;

    // following methods are just reached through to m_popup->listbox()
    void insertStringList( const QStringList& sl, int index=-1 );
    void insertStrList( const QStrList* sl, int index=-1 );
    void insertStrList( const QStrList& sl, int index=-1 );
    void insertStrList( const char ** strs, int numStrings=-1, int index=-1 );

    void insertItem( const QListBoxItem* lbi, int index=-1 );
    void insertItem( const QListBoxItem* lbi, const QListBoxItem *after );
    void insertItem( const QString &text, int index=-1 );
    void insertItem( const QPixmap &pixmap, int index=-1 );
    void insertItem( const QPixmap &pixmap, const QString &text, int index=-1 );

    void removeItem( int index );

    void changeItem( const QListBoxItem* lbi, int index );
    void changeItem( const QString &text, int index );
    void changeItem( const QPixmap &pixmap, int index );
    void changeItem( const QPixmap &pixmap, const QString &text, int index );

  signals:
    void undoRedoLast(int); /// Emitted, when button is pressed or a number of
                            /// actions is selected

  public slots:
    void clear();
    void setMode( Mode m );

  protected:
    Mode m_Mode;

  protected slots:
    void popupItemsSelected(int num);
    void iconButtonClicked();
    void arrowButtonClicked();
    
  private:
    QPushButton* m_iconbutton;
    QPushButton* m_arrowbutton;
    QHBoxLayout* m_layout;
    PopupUndoRedoListBox* m_popup;
    QString m_actionName;
};

#endif
