//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Copyright (C) 2009  Sebastian Mach (*1983)
// * mail: phresnel/at/gmail/dot/com
// * http://phresnel.org
// * http://picogen.org
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#ifndef COLORWHEELWIDGET_HH
#define COLORWHEELWIDGET_HH

#include <QGLWidget>

#include "color.hh"


class ColorWheelWidget : public QGLWidget {
        Q_OBJECT
public:
        ColorWheelWidget (QWidget *parent = 0) ;
        void paintGL();

        void initializeGL();
        void resizeGL(int w, int h);

protected:
        void mouseMoveEvent(QMouseEvent*);
        void mousePressEvent(QMouseEvent*);

signals:
        void currentColorChanged (color::rgbf col);

private slots:
        void setCurrentColor (color::rgbf col);

private:
        void update(QPointF const & mousePosition);
        color::rgbf currentColor;
};

#endif // COLORWHEELWIDGET_HH
