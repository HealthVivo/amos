#include "MainWindow.hh"

#include <qapplication.h>
#include <qpushbutton.h>
#include <qfont.h>
#include <qlayout.h>
#include <qspinbox.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qfiledialog.h>
#include <qstatusbar.h>
#include <qlabel.h>
#include <qscrollview.h>
#include <qlineedit.h>
#include <qcheckbox.h>

#include "TilingFrame.hh"

MainWindow::MainWindow( QWidget *parent, const char *name )
           : QWidget( parent, name )
{
  // Menubar
  QMenuBar* menubar = new QMenuBar(this);
  Q_CHECK_PTR( menubar );

  QPopupMenu* file = new QPopupMenu( menubar );
  Q_CHECK_PTR( file );
  menubar->insertItem( "&File", file );
  file->insertItem( "&Open Bank...", this,  SLOT(openBank()) );
  file->insertItem( "&Quit", qApp,  SLOT(quit()) );

  // Statusbar
  QLabel * statusbar = new QLabel(this);
  statusbar->setFrameStyle(QFrame::Panel | QFrame::Sunken);
  statusbar->setText("No Bank Loaded");

  // Widgets
  m_contigid = new QSpinBox(1, 1, 1, this, "contigid");
  QSpinBox * fontsize = new QSpinBox(6, 24, 1, this, "fontsize");
  m_gindex = new QSpinBox(0,100, 1, this, "gindexspin");

  QLineEdit * dbpick = new QLineEdit("DMG", this, "dbpick");
  TilingFrame * tiling = new TilingFrame(this, "tilingframe");

  QCheckBox * stable = new QCheckBox("Stable Tiling", this, "stable");

  m_slider = new QSlider(Horizontal, this, "slider");
  m_slider->setTracking(0);

  // slider <-> tiling
  connect(m_slider, SIGNAL(valueChanged(int)),
          tiling, SLOT(setGindex(int)) );

  connect(tiling, SIGNAL(gindexChanged(int)),
          m_slider, SLOT(setValue(int)) );

  connect(m_slider, SIGNAL(sliderMoved(int)),
          tiling,   SLOT(trackGindex(int)));

  connect(m_slider, SIGNAL(sliderReleased()),
          tiling,   SLOT(trackGindexDone()) );

  // m_gindex <-> tiling
  connect(tiling, SIGNAL(gindexChanged(int)),
          m_gindex, SLOT(setValue(int)));

  connect(m_gindex, SIGNAL(valueChanged(int)),
          tiling, SLOT(setGindex(int)));

  // fontsize <-> tiling
  connect(fontsize, SIGNAL(valueChanged(int)), 
          tiling,   SLOT(setFontSize(int)));

  // stable <-> tiling
  connect(stable, SIGNAL(toggled(bool)),
          tiling, SLOT(toggleStable(bool)));

  // contigid <-> tiling
  connect(m_contigid, SIGNAL(valueChanged(int)),
          tiling,     SLOT(setContigId(int)));

  connect(tiling,     SIGNAL(contigLoaded(int)),
          m_contigid, SLOT(setValue(int)));

  // mainwindow <-> tiling
  connect(this,   SIGNAL(bankSelected(string)),
          tiling, SLOT(setBankname(string)));
  
  connect(tiling,   SIGNAL(contigRange(int, int)),
          this,     SLOT(setContigRange(int, int)));

  connect(tiling, SIGNAL(setGindexRange(int, int)),
          this,   SLOT(setGindexRange(int, int)));

  connect(this, SIGNAL(contigIdSelected(int)),
          tiling, SLOT(setContigId(int)));

  connect(this, SIGNAL(gindexChanged(int)),
          tiling, SLOT(setGindex(int)));

  // statusbar <-> tiling
  connect(tiling,    SIGNAL(setStatus(const QString &)),
          statusbar, SLOT(setText(const QString &)));

  // dbpick <-> tiling
  connect(dbpick, SIGNAL(textChanged(const QString &)),
          tiling, SLOT(setDB(const QString &)));

  QGridLayout *outergrid = new QGridLayout(NULL, 1, 2, 10, -1, "outerg"); // 1 row, 2 col
  
  // outmost
  QVBoxLayout* vbox = new QVBoxLayout(this);
  vbox->setMenuBar(menubar);
  menubar->setSeparator(QMenuBar::InWindowsStyle);
  vbox->addLayout(outergrid);
  vbox->addWidget(statusbar);
  vbox->activate();
  
  // outer
  QGridLayout *leftgrid = new QGridLayout(NULL, 12, 1, 1, -1, "leftg");
  QGridLayout *rightgrid = new QGridLayout(NULL, 2, 1, 10, -1, "rightg");
  outergrid->addMultiCellLayout(leftgrid, 0, 0, 0, 0);
  outergrid->addMultiCellLayout(rightgrid, 0, 0, 1, 1);
  outergrid->setColStretch( 1, 10 );

  //left
  QLabel * db_lbl = new QLabel(dbpick, "Database", this, "dblbl");
  leftgrid->addWidget(db_lbl,0,0);
  leftgrid->addWidget(dbpick,1,0);
  leftgrid->addRowSpacing(2,10);

  QLabel * contig_lbl = new QLabel(m_contigid, "Contig ID", this, "contiglbl");
  leftgrid->addWidget(contig_lbl,3,0);
  leftgrid->addWidget(m_contigid,4,0);
  leftgrid->addRowSpacing(5,10);

  QLabel * gindex_lbl = new QLabel(m_gindex, "Position", this, "gindexlbl");
  leftgrid->addWidget(gindex_lbl,6,0);
  leftgrid->addWidget(m_gindex,7,0);
  leftgrid->addRowSpacing(8,10);

  QLabel * fontsize_lbl = new QLabel(fontsize, "Font Size", this, "fontlbl");
  leftgrid->addWidget(fontsize_lbl,9,0);
  leftgrid->addWidget(fontsize, 10,0);
  leftgrid->addRowSpacing(11,10);

  leftgrid->addWidget(stable, 12, 0);
  leftgrid->setRowStretch(13,10);

  //right
  rightgrid->addWidget(tiling, 0, 0);
  rightgrid->addWidget(m_slider, 1, 0);
  rightgrid->setRowStretch(0, 10);

  // Set defaults
  fontsize->setValue(12);
  m_gindex->setValue(0);
  m_slider->setFocus();
}

void MainWindow::setBankname(string bankname)
{
  emit bankSelected(bankname);
}

void MainWindow::setGindex(int gindex)
{
  emit gindexChanged(gindex);
}

void MainWindow::setContigId(int contigId)
{
  emit contigIdSelected(contigId);
}

void MainWindow::openBank()
{
  QString s = QFileDialog::getExistingDirectory(
                   "/local/asmg/work/mschatz/AMOS",
                   this,
                   "get existing directory",
                   "Choose a Bank",
                   TRUE );

  if (s != "")
  {
    emit bankSelected(s.ascii());
  }
}

void MainWindow::setContigRange(int a, int b)
{
  m_contigid->setRange(a,b);
}

void MainWindow::setGindexRange(int a, int b)
{
  m_gindex->setRange(a,b);
  m_slider->setRange(a,b);
}

