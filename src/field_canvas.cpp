// -*-c++-*-

/*!
  \file field_canvas.cpp
  \brief main field canvas class Source File.
*/

/*
 *Copyright:

 Copyright (C) The RoboCup Soccer Server Maintenance Group.
 Hidehisa AKIYAMA

 This code is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 3, or (at your option)
 any later version.

 This code is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this code; see the file COPYING.  If not, write to
 the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.

 *EndCopyright:
 */

/////////////////////////////////////////////////////////////////////

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <QtGui>

#include "field_canvas.h"

#include "disp_holder.h"
#include "field_painter.h"

#include "ball_painter.h"
#include "player_painter.h"
#include "score_board_painter.h"
#include "team_graphic_painter.h"

#include "options.h"

#include <iostream>

/*-------------------------------------------------------------------*/
/*!

*/
FieldCanvas::FieldCanvas( DispHolder & disp_holder )
    :
#ifdef USE_GLWIDGET
    QGLWidget( QGLFormat( QGL::SampleBuffers ) ),
#else
    QWidget( /* parent, flags */ ),
#endif
    M_disp_holder( disp_holder ),
    M_monitor_menu( static_cast< QMenu * >( 0 ) )
{
    this->setMouseTracking( true ); // need for the MouseMoveEvent
    this->setFocusPolicy( Qt::WheelFocus );

    readSettings();

    createPainters();
}

/*-------------------------------------------------------------------*/
/*!

*/
FieldCanvas::~FieldCanvas()
{
    writeSettings();
}

/*-------------------------------------------------------------------*/
/*!

*/
void
FieldCanvas::setMonitorMenu( QMenu * menu )
{
    if ( M_monitor_menu )
    {
        delete M_monitor_menu;
        M_monitor_menu = static_cast< QMenu * >( 0 );
    }

    M_monitor_menu = menu;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
FieldCanvas::readSettings()
{
    QSettings settings( Options::CONF_FILE,
                        QSettings::IniFormat );

    settings.beginGroup( "FieldCanvas" );

//     QVariant val;

//     val = settings.value( "measure_line_pen" );
//     if ( val.isValid() ) M_measure_line_pen.setColor( val.toString() );

//     val = settings.value( "measure_mark_pen" );
//     if ( val.isValid() ) M_measure_mark_pen.setColor( val.toString() );

//     val = settings.value( "measure_font_pen" );
//     if ( val.isValid() ) M_measure_font_pen.setColor( val.toString() );

//     val = settings.value( "measure_font_pen2" );
//     if ( val.isValid() ) M_measure_font_pen2.setColor( val.toString() );

//     val = settings.value( "measure_font" );
//     if ( val.isValid() ) M_measure_font.fromString( val.toString() );

    settings.endGroup();
}

/*-------------------------------------------------------------------*/
/*!

*/
void
FieldCanvas::writeSettings()
{
    QSettings settings( Options::CONF_FILE,
                        QSettings::IniFormat );

    settings.beginGroup( "FieldCanvas" );

//     settings.setValue( "measure_line_pen", M_measure_line_pen.color().name() );
//     settings.setValue( "measure_mark_pen", M_measure_mark_pen.color().name() );
//     settings.setValue( "measure_font_pen", M_measure_font_pen.color().name() );
//     settings.setValue( "measure_font_pen2", M_measure_font_pen2.color().name() );
//     settings.setValue( "measure_font", M_measure_font.toString() );

    settings.endGroup();
}

/*-------------------------------------------------------------------*/
/*!

*/
void
FieldCanvas::createPainters()
{
    M_field_painter = boost::shared_ptr< FieldPainter >( new FieldPainter() );

    M_painters.push_back( boost::shared_ptr< PainterInterface >( new PlayerPainter( M_disp_holder ) ) );
    M_painters.push_back( boost::shared_ptr< PainterInterface >( new BallPainter( M_disp_holder ) ) );
    M_painters.push_back( boost::shared_ptr< PainterInterface >( new TeamGraphicPainter( M_disp_holder ) ) );
    M_painters.push_back( boost::shared_ptr< PainterInterface >( new ScoreBoardPainter( M_disp_holder ) ) );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
FieldCanvas::mouseDoubleClickEvent( QMouseEvent * event )
{
    this->unsetCursor();

    if ( event->button() == Qt::LeftButton )
    {
        emit focusChanged( event->pos() );
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
FieldCanvas::mousePressEvent( QMouseEvent * event )
{
    if ( event->button() == Qt::LeftButton )
    {
        M_mouse_state[0].pressed( event->pos() );
        if ( M_mouse_state[1].isMenuFailed() )
        {
            M_mouse_state[1].setMenuFailed( false );
        }

        if ( event->modifiers() == Qt::ControlModifier )
        {
            emit focusChanged( event->pos() );
        }
//         else
//         {
//             selectPlayer( event->pos() );
//         }
    }
    else if ( event->button() == Qt::MidButton )
    {
        M_mouse_state[1].pressed( event->pos() );
        if ( M_mouse_state[1].isMenuFailed() )
        {
            M_mouse_state[1].setMenuFailed( false );
        }
    }
    else if ( event->button() == Qt::RightButton )
    {
        M_mouse_state[2].pressed( event->pos() );
        if ( M_mouse_state[2].isMenuFailed() )
        {
            M_mouse_state[2].setMenuFailed( false );
        }
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
FieldCanvas::mouseReleaseEvent( QMouseEvent * event )
{
    this->unsetCursor();

    if ( event->button() == Qt::LeftButton )
    {
        M_mouse_state[0].released();
    }
    else if ( event->button() == Qt::MidButton )
    {
        M_mouse_state[1].released();
    }
    else if ( event->button() == Qt::RightButton )
    {
        std::cerr << "mouse 2 released" << std::endl;
        M_mouse_state[2].released();
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
FieldCanvas::mouseMoveEvent( QMouseEvent * event )
{
    if ( this->cursor().shape() == Qt::BlankCursor )
    {
        this->unsetCursor();
    }

    if ( M_mouse_state[0].isDragged() )
    {
#if QT_VERSION >= 0x040200
        if ( this->cursor().shape() != Qt::ClosedHandCursor )
        {
            this->setCursor( QCursor( Qt::ClosedHandCursor ) );
        }
#else
        if ( this->cursor().shape() != Qt::SizeAllCursor )
        {
            this->setCursor( QCursor( Qt::SizeAllCursor ) );
        }
#endif

        int new_x = Options::instance().screenX( Options::instance().focusPoint().x() );
        int new_y = Options::instance().screenY( Options::instance().focusPoint().y() );
        new_x -= ( event->pos().x() - M_mouse_state[0].draggedPoint().x() );
        new_y -= ( event->pos().y() - M_mouse_state[0].draggedPoint().y() );
        emit focusChanged( QPoint( new_x, new_y ) );
    }

    for ( int i = 0; i < 3; ++i )
    {
        M_mouse_state[i].moved( event->pos() );
    }

//     if ( M_mouse_state[2].isDragged() )
//     {
//         static QRect s_last_rect;
//         std::cerr << "mouse 2 dragged" << std::endl;
//         if ( this->cursor().shape() != Qt::CrossCursor )
//         {
//             this->setCursor( QCursor( Qt::CrossCursor ) );
//         }

//         QRect new_rect
//             = QRect( M_mouse_state[2].pressedPoint(),
//                      M_mouse_state[2].draggedPoint() ).normalized();
//         new_rect.adjust( -32, -32, 32, 32 );
//         if ( new_rect.right() < M_mouse_state[2].draggedPoint().x() + 256 )
//         {
//             new_rect.setRight( M_mouse_state[2].draggedPoint().x() + 256 );
//         }
//         // draw mouse measure
//         this->update( s_last_rect.unite( new_rect ) );
//         s_last_rect = new_rect;
//     }

    emit mouseMoved( event->pos() );
}

void
FieldCanvas::contextMenuEvent( QContextMenuEvent * event )
{
    if ( event->reason() == QContextMenuEvent::Mouse )
    {
        M_mouse_state[2].released();
        M_monitor_menu->exec( event->globalPos() );
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
FieldCanvas::paintEvent( QPaintEvent * )
{
    QPainter painter( this );

    if ( Options::instance().antiAliasing() )
    {
#ifdef USE_GLWIDGET
        painter.setRenderHint( QPainter::HighQualityAntialiasing );
#else
        painter.setRenderHint( QPainter::Antialiasing );
#endif
    }

    draw( painter );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
FieldCanvas::updateFocus()
{
    DispConstPtr disp = M_disp_holder.currentDisp();

    if ( ! disp )
    {
        return;
    }

    // if auto select mode, update the ball nearest player
    if ( Options::instance().playerAutoSelect() )
    {
        const rcss::rcg::ShowInfoT & show = disp->show_;

        Options::PlayerSelectType old_type = Options::instance().playerSelectType();

        float min_dist2 = 40000.0f;

        rcss::rcg::Side side = rcss::rcg::NEUTRAL;
        int unum = 0;

        const int first = ( old_type == Options::SELECT_AUTO_RIGHT
                            ? 11
                            : 0 );
        const int last = ( old_type == Options::SELECT_AUTO_LEFT
                           ? 11
                           : 22 );
        for ( int i = first; i < last; ++i )
        {
            if ( show.player_[i].state_ != 0 )
            {
                float d2
                    = std::pow( show.ball_.x_ - show.player_[i].x_, 2 )
                    + std::pow( show.ball_.y_ - show.player_[i].y_, 2 );
                if ( d2 < min_dist2 )
                {
                    min_dist2 = d2;
                    side = show.player_[i].side();
                    unum = show.player_[i].unum_;
                }
            }
        }

        if ( unum != 0 )
        {
            Options::instance().setSelectedNumber( side, unum );
        }
    }

    // update focus point
    if ( Options::instance().focusType() == Options::FOCUS_BALL )
    {
        Options::instance().setFocusPointReal( disp->show_.ball_.x_,
                                               disp->show_.ball_.y_ );
    }
    else if ( Options::instance().focusType() == Options::FOCUS_PLAYER
              && Options::instance().selectedNumber() != 0 )
    {
        int id = Options::instance().selectedNumber();
        if ( id < 0 )
        {
            id = -1*id + 11;
        }
        id -= 1;

        if ( disp->show_.player_[id].state_ != 0 )
        {
            Options::instance().setFocusPointReal( disp->show_.player_[id].x_,
                                                   disp->show_.player_[id].y_ );
        }
    }
    else
    {
        // already set
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
FieldCanvas::draw( QPainter & painter )
{
    updateFocus();
    Options::instance().updateFieldSize( this->width(), this->height() );

    M_field_painter->draw( painter );

    if ( ! M_disp_holder.currentDisp() )
    {
        return;
    }

    for ( std::vector< boost::shared_ptr< PainterInterface > >::iterator it = M_painters.begin();
          it != M_painters.end();
          ++it )
    {
        (*it)->draw( painter );
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
FieldCanvas::dropBall()
{
    emit dropBall( M_mouse_state[2].pressedPoint() );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
FieldCanvas::freeKickLeft()
{
    emit freeKickLeft( M_mouse_state[2].pressedPoint() );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
FieldCanvas::freeKickRight()
{
    emit freeKickRight( M_mouse_state[2].pressedPoint() );
}