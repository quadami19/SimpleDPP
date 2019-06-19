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

#ifndef DEBUGMENU_HPP
#define DEBUGMENU_HPP

#include <QtWidgets>
#include <QVector>
#include "core.hpp"
#include "qcustomplot/qcustomplot.h"
#include "nuclteachingclass.hpp"

class DebugInputWidget;
class DebugFiltersWidget;
class DebugProcWidget;
class DebugPulseWidget;

class DebugMenu : public QDialog {
		Q_OBJECT
		QLabel* availDataLabel;
		QComboBox* availDataCB;
		QHBoxLayout* headLayout;
		QVBoxLayout* mainLayout;
		QPushButton* closePB;

		DebugInputWidget* dbgInpData;
		DebugFiltersWidget* dbgFilData;
		DebugProcWidget* dbgProcData;
		DebugPulseWidget* dbgPulData;


		void showEvent(QShowEvent *);


		enum DataTypes {
			InputBuffer = 0,
			ProcesBuffer,
			NeuralPulse,
		};

	public:
		DebugMenu(Core* ptr, QWidget* parent);
		~DebugMenu() {}

	private slots:
		void replace_widgets();

};

class DebugWidget : public QWidget {
		Q_OBJECT

		QLabel* streamLabel;
		QLabel* xLeftLabel;
		QLabel* xRightLabel;
		QLabel* yDownLabel;
		QLabel* yUpLabel;
		QPushButton* rescalePB;
		QPushButton* clearPB;
		QGridLayout* scaleLayout;
		QGridLayout* butnLayout;
		QVBoxLayout* widgLayout;
		QHBoxLayout* mainLayout;

	protected:
		QDoubleSpinBox* xLeftDSB;
		QDoubleSpinBox* xRightDSB;
		QDoubleSpinBox* yDownDSB;
		QDoubleSpinBox* yUpDSB;
		Core* corePtr;
		QCustomPlot* plot;
		QComboBox* dataSelect;
		QPushButton* dataGet;
		QComboBox* streamCB;
		QGridLayout* customLayout;
		QVector<double> dataX;
		QVector<double> dataY;
		quint32 index = 0;

		virtual void showEvent(QShowEvent *);
	public:
		DebugWidget(Core* ptr, QWidget* parent);
		virtual ~DebugWidget() {}
	public slots:
		virtual void get_data () = 0;
		virtual void update_streams() = 0;
		virtual void clear_rescdsb();
		void scale_val_update();
		void rescale();
};

class DebugInputWidget : public DebugWidget {
		Q_OBJECT

		enum BufferType {
			Raw = 0,
			Filtered,
			Interpolated
		};

	public:
		DebugInputWidget(Core* ptr, QWidget* parent);
		~DebugInputWidget() {}

	private slots:
		void update_streams();
		void get_data();
};

class DebugProcWidget : public DebugWidget {
		Q_OBJECT

		enum BufferType {
			Processed = 0
		};

		std::vector<std::pair<quint32, quint32>> procIndex;
		void showEvent(QShowEvent *);

	public:
		DebugProcWidget(Core* ptr, QWidget* parent);
		~DebugProcWidget() {}

	private slots:
		void update_streams();
		void get_data();

};

class DebugPulseWidget : public DebugWidget {
		Q_OBJECT

		QLabel* wNLabel;
		QDoubleSpinBox* wNDSBa;
		QLabel* pNLabel;
		QDoubleSpinBox* pNDSBa;
		QDoubleSpinBox* pNDSBb;
		QLabel* rNLabel;
		QDoubleSpinBox* rNDSBa;
		QDoubleSpinBox* rNDSBb;
		QLabel* bNLabel;
		QDoubleSpinBox* bNDSBa;
		QDoubleSpinBox* bNDSBb;
		QLabel* vNLabel;
		QDoubleSpinBox* vNDSBa;
		QDoubleSpinBox* vNDSBb;
		QLabel* selTypeLabel;
		QComboBox* selTypeCB;

		enum PulseType {
			NeuralNet = 0
		};

		std::vector<std::pair<quint32, quint32>> procIndex;
		Neural_Network::NuclAmplTeachingClass teachA;
		Neural_Network::NuclTimeTeachingClass teachT;
		Neural_Network::NuclDiscrTeachingClass teachD;

		void showEvent(QShowEvent *);
		void clear_rescdsb();

	public:

		DebugPulseWidget (Core* ptr, QWidget* parent);
		~DebugPulseWidget() {}

	private slots:
		void update_streams();
		void get_data();
};


#endif // DEBUGMENU_HPP
