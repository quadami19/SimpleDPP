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

#ifndef AUTOSAVEDIALOG_HPP
#define AUTOSAVEDIALOG_HPP

#include <QtWidgets>

class MainWindow;


class AutosaveDialog : public QDialog {
		Q_OBJECT

		MainWindow* mainPtr;

		QLabel* enableLabel;
		QCheckBox* enableChB;
		QLabel* intervalLabel;
		QSpinBox* intervalSB;
		QLabel* filenameLabel;
		QLineEdit* filenameEdit;
		QPushButton* acceptPB;
		QPushButton* rejectPB;

		QGridLayout* mainLayout;

		void showEvent(QShowEvent *);
	public:
		AutosaveDialog(MainWindow* parent = 0);

	private slots:
		void accept();
};

#endif // AUTOSAVEDIALOG_HPP
