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

#ifndef PICOGENQT_H_INCLUDED_20090323
#define PICOGENQT_H_INCLUDED_20090323

#include "ui_picogenqt.h"

#include <QMdiSubWindow>

#include "../../redshift/include/redshift.hh"

#include "../include/scene.hh"

#include "../include/camerasettings.hh"
#include "../include/camera-list.hh"
#include "../include/heightmap-designer.hh"
#include "../include/heightmap-layers.hh"
#include "../include/scene-display.hh"



class PicogenQTImpl : public QMainWindow, private Ui::PicogenQT
{
        Q_OBJECT

public:
        PicogenQTImpl(QWidget* parent=0);
        virtual ~PicogenQTImpl() {}

private slots:

        void on_actionCamera_triggered ();
        void on_actionSun_and_Sky_triggered ();
        void on_actionTerrain_Definition_triggered ();

        void openCameraSettings (QString const &);
        void renameCamera (QString const &oldName, QString const &newName);
        void closeCamera (QString const &name);

private:
        QMap <QString, CameraSettingsImpl*> cameraSettings;

        redshift::shared_ptr<scene::Scene> scene;
};


#endif // PICOGENQT_H_INCLUDED_20090323