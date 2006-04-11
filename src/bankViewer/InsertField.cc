#include "InsertField.hh"
#include <qwmatrix.h>
#include "InsertCanvasItem.hh"
#include "FeatureCanvasItem.hh"
#include "ContigCanvasItem.hh"
#include "CoverageCanvasItem.hh"
#include "DataStore.hh"
#include "InsertCanvasItem.hh"


#include <iostream>
using namespace std;

InsertField::InsertField(DataStore * datastore,
                         int & hoffset,
                         QCanvas * canvas, 
                         QWidget * parent, 
                         const char * name)
 : QCanvasView(canvas, parent, name),
   m_datastore(datastore),
   m_hoffset(hoffset),
   m_featrect(NULL),
   m_feat(NULL)
{
  QWMatrix m = worldMatrix();
  m.translate(20, 0);
  m.scale( 0.05, 1.0 );
  setWorldMatrix( m );

  setHScrollBarMode(QScrollView::AlwaysOff);
  setVScrollBarMode(QScrollView::AlwaysOff);

  m_visibleRect = new QCanvasRectangle(0,0,100,100, canvas);
  m_visibleRect->setPen(Qt::black);
  m_visibleRect->setBrush(Qt::black);
  m_visibleRect->setZ(-1000);
  m_visibleRect->show();
}

void InsertField::highlightInsert(InsertCanvasItem * iitem, 
                                  bool highlight,
                                  bool highlightBuddy)
{
  iitem->m_highlight = highlight;

  Insert * ins = iitem->m_insert;

  QString s = "Insert";

  AMOS::ID_t iid = ins->m_aid;
  emit readIIDHighlighted(QString::number(iid));
  emit readEIDHighlighted(QString(m_datastore->read_bank.lookupEID(iid).c_str()));

  getInsertString(s, ins->m_active, ins, 0, iitem);
  getInsertString(s, !ins->m_active, ins, 1, iitem);

 // if (ins->m_atile) cerr << "a:" << (ins->m_atile->offset + ins->m_atile->getGappedLength() -1) << endl;
 // if (ins->m_btile) cerr << "b:" << (ins->m_btile->offset + ins->m_btile->getGappedLength() -1) << endl;
 // cerr << "i:" << ins->m_roffset << endl;

  s += QString(" [") + (char)ins->m_state + "] ";

  s += " Actual: "   + QString::number(ins->m_actual);
  s += " Expected: " + QString::number(ins->m_dist.mean - Insert::MAXSTDEV*ins->m_dist.sd) 
    +  " - "         + QString::number(ins->m_dist.mean + Insert::MAXSTDEV*ins->m_dist.sd);

  s += " Coordinates: " + QString::number(ins->m_loffset)
    +  " - "            + QString::number(ins->m_roffset);
    

  emit setStatus(s);

  canvas()->setChanged(iitem->boundingRect());

  if (highlightBuddy &&
      ins->m_other && 
      ins->m_other->m_canvasItem)
  {
    ins->m_other->m_canvasItem->m_highlight = iitem->m_highlight;
    canvas()->setChanged(ins->m_other->m_canvasItem->boundingRect());
  }

  canvas()->update();
}

void InsertField::highlightEID(const QString & qeid)
{
  AMOS::ID_t iid = m_datastore->read_bank.lookupIID(qeid.ascii());
  highlightRead(iid);
}

void InsertField::highlightIID(const QString & qiid)
{
  AMOS::ID_t iid = (AMOS::ID_t) (qiid.toUInt());
  highlightRead(iid);
}

void InsertField::highlightRead(int iid)
{
  if (iid == AMOS::NULL_ID)
  {
    return;
  }

  QCanvasItemList all = canvas()->allItems();

  for (QCanvasItemList::Iterator it=all.begin(); it!=all.end(); ++it) 
  {
    if ((*it)->rtti() == InsertCanvasItem::RTTI)
    {
      InsertCanvasItem * iitem = (InsertCanvasItem *) *it;
      Insert * ins = iitem->m_insert;

      if (ins->m_aid == iid || ins->m_bid == iid)
      {
        highlightInsert(iitem, true, true);
        break;
      }
    }
  }
}

void InsertField::contentsMousePressEvent( QMouseEvent* e )
{
  QPoint real = inverseWorldMatrix().map(e->pos());
  QCanvasItemList l = canvas()->collisions(real);

  int gindex = 16*real.x() - m_hoffset;
  int jumptoread = 0;

  bool jump = true;
  bool emitstatus = true;

  QString s = "[" + QString::number(gindex) + "]";

  for (QCanvasItemList::Iterator it=l.begin(); it!=l.end(); ++it) 
  {
    if (((*it)->rtti() == FeatureCanvasItem::RTTI) || 
        ((*it)->rtti() == ContigCanvasItem::RTTI))
    {

      if (!m_featrect)
      {
        m_featrect = new QCanvasRectangle((*it)->x(), 0, 
                                          (*it)->boundingRect().width(), canvas()->height(), 
                                          canvas());
        m_featrect->setBrush(QColor(59,49,31));
        m_featrect->setPen(QColor(139,119,111));
        m_featrect->setZ(-2);
        m_featrect->show();
      }
      else
      {
        if (m_feat == *it)
        {
          if (m_featrect->isVisible()) { m_featrect->hide(); }
          else                         { m_featrect->show(); }
        }
        else
        {
          canvas()->setChanged(m_featrect->boundingRect());
          m_featrect->setSize((*it)->boundingRect().width(), canvas()->height());
          m_featrect->move((*it)->x(), 0);
          m_featrect->show();
        }
      }

      m_feat = *it;
      canvas()->setChanged(m_featrect->boundingRect());
      canvas()->update();

    }


    if ((*it)->rtti() == InsertCanvasItem::RTTI)
    {
      InsertCanvasItem * iitem = (InsertCanvasItem *) *it;
      bool highlight = !iitem->m_highlight;
      bool highlightBuddy = e->button() == RightButton;

      if (e->state() == Qt::ControlButton)
      {
        if (iitem->m_contigcolor)
        {
          jumptoread = iitem->m_alinkedread;
        }
        else
        {
          jumptoread = iitem->m_insert->m_bid;
        }
      }

      highlightInsert(iitem, highlight, highlightBuddy);
      jump = false;
      emitstatus = false;
    }
    else if ((*it)->rtti() == FeatureCanvasItem::RTTI)
    {
      FeatureCanvasItem * fitem = (FeatureCanvasItem *) * it;
      s += " Feature EID: ";
      s += fitem->m_feat.getEID().c_str();
      s += " Comment:";
      s += fitem->m_feat.getComment().c_str();
      s += " Type:";
      s += (char)fitem->m_feat.getType();
      s += " [" +  QString::number(fitem->m_feat.getRange().begin) + ",";
      s += QString::number(fitem->m_feat.getRange().end) + "]";
      s += " " + QString::number(fitem->m_feat.getRange().getLength()) + "bp";

      jump = false;
      emitstatus = true;
    }
    else if ((*it)->rtti() == ContigCanvasItem::RTTI)
    {
      ContigCanvasItem * citem = (ContigCanvasItem *) * it;

      s += " Contig ID: " + QString::number(m_datastore->contig_bank.getIDMap().lookupBID(citem->m_tile.source));
      s += " IID: " + QString::number(citem->m_tile.source);
      s += " EID: " + QString(m_datastore->contig_bank.lookupEID(citem->m_tile.source).c_str());
      s += " [" + QString::number(citem->m_tile.offset) +
           ","  + QString::number(citem->m_tile.offset + citem->m_tile.range.getLength()) +
           "]";
      s += " " + QString::number(citem->m_tile.range.getLength()) + "bp";

      jump = false;
      emitstatus = true;
    }
    else if ((*it)->rtti() == CoverageCanvasItem::RTTI)
    {
      CoverageCanvasItem * citem = (CoverageCanvasItem *) * it;

      int i = 1;
      int click = real.x();

      for (i = 1; i < citem->m_points.size(); i++)
      {
        int xval = citem->m_points[i].x();
        if (xval > click)
        {
          break;
        }
      }

      i--;

      if (0)
      {
        int xval = citem->m_points[i].x();
        xval = 16*xval - m_hoffset;
        cerr << "i: " << i << " xval: " << xval << endl;
      }

      if (citem->m_libid < 0)
      {
             if (citem->m_libid == -1) { s += " Clone"; }
        else if (citem->m_libid == -2) { s += " Read";  }
        else if (citem->m_libid == -3) { s += " Kmer";  }

        s += " Coverage " + QString::number(citem->y() + citem->height() - citem->m_points[i].y());
        s += " [" + QString::number(citem->m_baseLevel, 'f', 2) + "]";
      }
      else
      {
        s += " Lib: " + QString::number(citem->m_libid);
        s += " " + QString::number(citem->m_raw[i], 'f', 3);
      }

      jump = true;
      emitstatus = true;
    }
  }

  if (emitstatus)
  {
    emit setStatus(s);
  }

  if (jump)
  {
    emit setGindex(gindex);
  }

  if (jumptoread)
  {
    emit jumpToRead(jumptoread);
  }
}

void InsertField::viewportPaintEvent(QPaintEvent * e)
{
  QRect rc = QRect(contentsX(),    contentsY(),
                   visibleWidth(), visibleHeight() );
  QRect real = inverseWorldMatrix().mapRect(rc);

#if 0
  if (m_visibleRect)
  {
    m_visibleRect->setPen(Qt::black);
    m_visibleRect->setBrush(Qt::black);
    m_visibleRect->setSize(real.width(), real.height());
    m_visibleRect->move(real.x(), real.y());
    canvas()->setChanged(m_visibleRect->boundingRect());
    canvas()->update();
  }
  else
  {
    cerr << "WTF???" << endl;
  }
#endif

  QCanvasView::viewportPaintEvent(e);

  emit visibleRange(16*real.x()-m_hoffset, worldMatrix().m11()/16);
}



void InsertField::getInsertString(QString & s, int selectb, Insert * ins, int second, InsertCanvasItem * iitem)
{
  AMOS::ID_t iid = ins->m_aid;
  AMOS::ID_t contigiid = ins->m_acontig;
  AMOS::ID_t linked = iitem->m_alinked;

  if (selectb)
  {
    iid = ins->m_bid;
    contigiid = ins->m_bcontig;
    linked = iitem->m_blinked;
  }

  if (iid != AMOS::NULL_ID)
  {
    if (second)
    {
      s += " <=>";
    }

    s += " ";
    if (ins->m_active == selectb)
    {
      s += "*";
    }

    s += "[";
    s += "e:" + QString(m_datastore->read_bank.lookupEID(iid).c_str());
    s += " i:" + QString::number(iid);
    s += " c:" + QString::number(contigiid);

    if (iitem->m_contigcolor)
    {
      s += " l:" + QString::number(linked);
    }

    s += "]";
  }
}

void InsertField::canvasCleared()
{
  m_featrect = NULL;
  m_feat = NULL;
}
