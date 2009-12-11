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

#include <iostream>
#include <QtGui/QMdiSubWindow>

#include "../../redshift/include/redshift.hh"

#include "../include/picogenqt.hh"
#include "../include/camerasettings.hh"

int picogen_main ();

PicogenQTImpl::PicogenQTImpl(QWidget *parent)
: QMainWindow (parent), PicogenQT()
{
        setupUi(this);
        QMetaObject::connectSlotsByName(this);
        
        setCentralWidget (mdiArea);
        
        
        CameraSettingsImpl *s = new CameraSettingsImpl;        
        QMdiSubWindow *sub = mdiArea->addSubWindow(s);                
        sub->show();
        
        /*QImage *image = new QImage ("images/redshift.png");
        QPixmap pix = QPixmap::fromImage (*image);
        heightmapLabel->resize (image->width(), image->height());
        heightmapLabel->setPixmap (pix);*/        
                
        //picogen_main();
}

/*
void PicogenQTImpl::on_edRed_textChanged(QString value) {
}

void PicogenQTImpl::on_edGreen_textChanged(QString value) {
        exit(0);
}

void PicogenQTImpl::on_edBlue_textChanged(QString value) {
}*/