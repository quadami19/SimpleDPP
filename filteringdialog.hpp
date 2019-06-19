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

#ifndef FILTERINGDIALOG_HPP
#define FILTERINGDIALOG_HPP

#include "core.hpp"
#include <QDialog>
#include <QtWidgets>

class ShaperDialog;
class DelayLineDialog;
class RCIIRDialog;
class CRIIRDialog;
class CRREVIIRDialog;
class TrapezoidalDialog;
class GaussianDialog;
class CuspDialog;

class FilteringDialog : public QDialog {
		Q_OBJECT

		std::vector<QLabel*> filterTypeLabel;
		std::vector<QPushButton*> filterSetPB;
		std::vector<QPushButton*> filterDelPB;
		std::vector<std::vector<std::shared_ptr<Filter::FilterSettings>>> filterSettings;
		std::vector<bool> filterEnabled;

		QComboBox* streamCB;
		QPushButton* applyPB;
		QLabel* enableLabel;
		QCheckBox* enableCB;
		QComboBox* filterAddCB;
		QPushButton* filterAddPB;
		QPushButton* acceptPB;
		QPushButton* rejectPB;

		QVBoxLayout* mainLayout;
		QHBoxLayout* strmLayout;
		QGridLayout* listLayout;
		QHBoxLayout* addSLayout;
		QHBoxLayout* butnLayout;
		std::vector<std::shared_ptr<ShaperDialog>> filDialogs;

		Core* coreClassPtr;

		void showEvent(QShowEvent *);
		void update_streams ();
		void reload_settings();
		void update_widgets();

	public:
		explicit FilteringDialog(Core* core, QWidget* parent = 0);
	private slots:
		void accept();
		void apply();
		void add();
		void set_filter();
		void del_filter();
		void cb_stream_changed();
		void chb_changed();
};

class ShaperDialog : public QDialog {
		Q_OBJECT

	protected:
		QLabel* infoLabel;
		QPushButton* acceptPB;
		QPushButton* rejectPB;
		QGridLayout* mainLayout;
		std::shared_ptr<Filter::FilterSettings> settings;
		virtual void update_values() = 0;

	public:
		ShaperDialog(QWidget* parent);
		void set_settings (std::shared_ptr<Filter::FilterSettings> set)
			{ settings = set; update_values(); }

	public slots:
		virtual void accept() = 0;
};

class DelayDialog : public ShaperDialog {
		Q_OBJECT
		QLabel* delayerTimeLabel;
		QSpinBox* delayerTime;

		void update_values();

	public:
		explicit DelayDialog(QWidget* parent = 0);

	private slots:
		void accept();

};

class DelayLineDialog : public ShaperDialog {
		Q_OBJECT
		QLabel* delayerTimeLabel;
		QSpinBox* delayerTime;
		QLabel* delayerAmplLabel;
		QDoubleSpinBox* delayerAmpl;

		void update_values();

	public:
		explicit DelayLineDialog(QWidget* parent = 0);

	private slots:
		void accept();

};

class OverrunLineDialog : public ShaperDialog {
		Q_OBJECT
		QLabel* overrunTimeLabel;
		QSpinBox* overrunTime;
		QLabel* overrunAmplLabel;
		QDoubleSpinBox* overrunAmpl;

		void update_values();

	public:
		explicit OverrunLineDialog(QWidget* parent = 0);

	private slots:
		void accept();

};

class RC_CR_Dialog : public ShaperDialog {
		Q_OBJECT

	protected:
		QLabel* tauConstLabel;
		QDoubleSpinBox* tauConstDSB;

	public:

		RC_CR_Dialog(QWidget* parent);

};

class RCIIRDialog : public RC_CR_Dialog {
		Q_OBJECT

		void update_values();

	public:

		RCIIRDialog(QWidget* parent);

	private slots:
		void accept();
};

class CRIIRDialog : public RC_CR_Dialog {
		Q_OBJECT

		void update_values();

	public:

		CRIIRDialog(QWidget* parent);

	private slots:
		void accept();
};

class CRREVIIRDialog : public RC_CR_Dialog {
		Q_OBJECT

		void update_values();

	public:

		CRREVIIRDialog(QWidget* parent);

	private slots:
		void accept();
};

class TrapezoidalDialog : public ShaperDialog {
		Q_OBJECT
		QLabel* sideLabel;
		QSpinBox* side;
		QLabel* flatLabel;
		QSpinBox* flat;

		void update_values();

	public:
		explicit TrapezoidalDialog(QWidget* parent = 0);

	private slots:
		void accept();

};

class MovingAverageDialog : public ShaperDialog {
		Q_OBJECT
		QLabel* sizeLabel;
		QSpinBox* size;

		void update_values();

	public:
		explicit MovingAverageDialog(QWidget* parent = 0);

	private slots:
		void accept();

};

class GaussianDialog : public ShaperDialog {
		Q_OBJECT
		QLabel* widthLabel;
		QDoubleSpinBox* width;

		void update_values();

	public:
		explicit GaussianDialog(QWidget* parent = 0);

	private slots:
		void accept();

};

class CuspDialog : public ShaperDialog {
		Q_OBJECT
		QLabel* widthLabel;
		QDoubleSpinBox* width;
		QLabel* flatLabel;
		QSpinBox* flat;

		void update_values();

	public:
		explicit CuspDialog(QWidget* parent = 0);

	private slots:
		void accept();

};

#endif // FILTERINGDIALOG_HPP
