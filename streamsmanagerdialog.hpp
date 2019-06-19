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

#ifndef STREAMSMANAGERDIALOG_HPP
#define STREAMSMANAGERDIALOG_HPP

#include <QtWidgets>
#include <sstream>
#include <vector>
#include "core.hpp"

class StandartSettingsDialog;
class CoincidenceSettingsDialog;

class StreamsManagerDialog : public QDialog {

		Q_OBJECT

		//friend class CoincidenceSettingsDialog;

		Core* corePtr;

		QLabel* inputNamesLabel;
		QLabel* procTypeLabel;
		QLabel* procNameLabel;

		QVBoxLayout* inptLayout;
		QGridLayout* procLayout;
		QHBoxLayout* widgLayout;
		QHBoxLayout* butnLayout;
		QVBoxLayout* mainLayout;
/*
		struct ProcWidg {
				QComboBox* type;
				QTextEdit* name;
		};
*/
		std::vector<QLineEdit*> inputsNames;
		std::vector<QComboBox*> procTypeWidgets;
		std::vector<QLineEdit*> procNameWidgets;

		QPushButton* acceptPB;
		QPushButton* rejectPB;
		QPushButton* addPB;
		QPushButton* delPB;

		void update_input_streams ();
		void update_processing_streams ();
		void replace_widgets ();

		void cb_add_items(QComboBox* box);

	protected:

		void showEvent(QShowEvent *);


	public:
		StreamsManagerDialog(Core* _ptr, QWidget* _parent = 0x0);

		void set_physical_streams (quint32 stream);

	private slots:

		void accept();
		void reject();
		void add_proc();
		void del_proc();
		//void add_spec();
		//void set_spec();
		//void del_spec();
		//void check_values();
};

#endif // STREAMSMANAGERDIALOG_HPP
