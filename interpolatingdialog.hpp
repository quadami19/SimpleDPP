/*

	Copyright (C) 2019 Gostev Roman

	This file is part of SimpleDPP.

	SimpleDPP is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	SimpleDPP is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with SimpleDPP.  If not, see <https://www.gnu.org/licenses/>.

*/

#ifndef SIGNALINTERPOLATINGDIALOG_HPP
#define SIGNALINTERPOLATINGDIALOG_HPP

#include <QObject>
#include <iostream>
#include <vector>
#include <QString>
#include <QtWidgets>
#include <complex>
#include <cstring>
#include "core.hpp"

class InterpolatingDialog : public QDialog {
		Q_OBJECT

		Core* CoreClassPtr;

		QLabel* interEnabledLab;
		QCheckBox* interEnabledCB;
		QLabel* interTypeLab;
		QComboBox* interTypeCB;
		QLabel* interMultLab;
		QComboBox* interMultCB;
		QLabel* interPrecLab;
		QComboBox* interPrecCB;
		QLabel* artReducLab;
		QCheckBox* artReducCB;
		QPushButton* acceptPB;
		QPushButton* rejectPB;
		QGridLayout* settingsLayout;
		QHBoxLayout* buttonsLayout;
		QVBoxLayout* mainLayout;

		void reload_settings ();

	protected:

		void showEvent(QShowEvent* _event);

	public:
		explicit InterpolatingDialog(Core* _core, QWidget * parent = 0);
		virtual ~InterpolatingDialog();

	signals:

	private slots:
		virtual void accept();
		virtual void reject();
};
#endif // SIGNALINTERPOLATINGDIALOG_HPP
