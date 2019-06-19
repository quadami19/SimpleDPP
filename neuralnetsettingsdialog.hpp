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

#ifndef NeuralNETSETTINGSDIALOG_HPP
#define NeuralNETSETTINGSDIALOG_HPP

#include <QObject>
#include <QtWidgets>
#include <QEventLoop>
#include "core.hpp"
#include "nuclteachingclass.hpp"

const quint32 TEACHMAXLAYERS = 6;
class NeuralNetTeachingWaitingDialog;
class NoizeSettingsDialog;
class NeuralNetAddDialog;

class NeuralNetSettingsDialog : public QDialog {
		Q_OBJECT

		friend class NeuralNetParamsFunctionAddDialog;
		Core* coreClassPtr;

		QTabWidget * tabWidget;
		QPushButton* acceptPB;
		QPushButton* rejectPB;
		QHBoxLayout* buttonsLayout;
		QVBoxLayout* mainLayout;

		QWidget* nnShowTab;
		QLabel* currNNL;
		QComboBox* currNNCB;
		QLabel* currNumNNL;
		QComboBox* currNumNNCB;
		QLabel* inputsL;
		QLabel* inputsI;
		QLabel* totLayersL;
		QLabel* totLayersI;
		QLabel* totNeuronsL;
		QLabel* totNeuronsI;
		QLabel* totWeightsL;
		QLabel* totWeightsI;
		QPushButton* addNNPB;
		QPushButton* delNNPB;
		QPushButton* saveNNPB;
		QPushButton* loadNNPB;
		QGridLayout* nnShowTabGrid;
		NeuralNetAddDialog* nnAddDial;

		std::vector<Neural_Network::NuclearPhysicsNeuralNet> nnDiscrVec;
		std::vector<Neural_Network::NuclearPhysicsNeuralNet> nnAmplVec;
		std::vector<Neural_Network::NuclearPhysicsNeuralNet> nnTimeVec;

		QWidget* teachTabWidget;
		QLabel* infoLabel;
		QLabel* functionLabel;
		QComboBox* functionCB;
		QLabel* availNNLabel;
		QComboBox* availNNCB;
		QPushButton* noizeSettingsPB;
		QLabel* teachCyclesLabel;
		QSpinBox* teachCyclesSB;
		QLabel* teachSizeLabel;
		QComboBox* teachSizeCB;
		QLabel* pulShapeL;
		QLabel* pulShapeI;
		QLabel* shapeStreamL;
		QComboBox* shapeStreamCB;
		QPushButton* startTeachingPB;
		QGridLayout* teachLayout;
		Neural_Network::NuclAmplTeachingClass nucTeachAmpl;
		Neural_Network::NuclTimeTeachingClass nucTeachTime;
		Neural_Network::NuclDiscrTeachingClass nucTeachDiscr;
		QThread teachThread;

		NeuralNetTeachingWaitingDialog* teachingWaitDialog;
		NoizeSettingsDialog* noizSetDial;

		enum AvailableNNTypes {
			Perceptron = 0
		};

		enum AvailNNFunctions {
			AmplitudeMeasuring = 0,
			TimeMeasuring,
			Discriminating
		};

		void check_teaching();
		Neural_Network::NuclearPhysicsNeuralNet& get_selected (QComboBox* type, QComboBox* num);
		std::vector<Neural_Network::NuclearPhysicsNeuralNet>* get_nnvector (QComboBox* type);

	protected:

		virtual void showEvent(QShowEvent *_event);

	public:

		NeuralNetSettingsDialog(Core* _corePtr, QWidget* _parent);
		virtual ~NeuralNetSettingsDialog();

	private slots:

		void accept();
		void reject();
		void save_nn();
		void load_nn();
		void add_nn();
		void del_nn();
		void update_nn_info();

		void func_changed();
		void showNoizeDialog();
		void startTeaching();

		void update_show_widget ();
		void update_teach_widget ();
		void update_values ();
};

class NeuralNetAddDialog : public QDialog {
		Q_OBJECT

		QLabel* typeL;
		QComboBox* typeCB;
		QLabel* inputsL;
		QComboBox* inputsCB;
		QLabel* nameL;
		QLineEdit* nameLE;
		QLabel* layersL;
		QSpinBox* layersSB;
		QGridLayout* settLayout;

		std::vector<QLabel*> NeuralsLs;
		std::vector<QSpinBox*> NeuralsSBs;
		QLabel* lastLayerL;
		QLabel* lastLayerI;
		QGridLayout* layersLayout;

		QPushButton* addPB;
		QPushButton* rejPB;
		QHBoxLayout* buttonLayout;
		QVBoxLayout* mainLayout;
		bool status = true;

		void showEvent(QShowEvent *);

	public:
		NeuralNetAddDialog (QWidget* parent = nullptr);
		~NeuralNetAddDialog() {}
		Neural_Network::NuclearPhysicsNeuralNet get_nn ();
		bool get_status () { return status; }

	private slots:
		void accept();
		void reject();
		void update_layers();
};

class NeuralNetTeachingWaitingDialog : public QDialog {

		enum CurrentStatus {
			Initializating = 0,
			Teaching,
			Finishing
		};

		Q_OBJECT
		QProgressBar* teachingProgressBar;
		QLabel* statusLabel;
		QLabel* currentSTDLabel;
		QLabel* totalTimeLabel;
		QPushButton* stopPB;
		QVBoxLayout* mainLayout;
		QTimer* updateTimer;

		Neural_Network::TeachingClass* teachClass;
		quint32 totalSecs = 0;
		quint32 currStatus = Initializating;
	protected:

		void showEvent(QShowEvent* _event);
		void hideEvent(QHideEvent* _event);
		void closeEvent(QCloseEvent* _event);
		bool eventFilter(QObject *dist, QEvent *event);

	public:
		NeuralNetTeachingWaitingDialog (Neural_Network::TeachingClass* _teachingClass, QWidget* _parent = 0x0);
		virtual ~NeuralNetTeachingWaitingDialog();
		void set_teaching_class (Neural_Network::TeachingClass* _teachingClass)
			{ teachClass = _teachingClass; }

	private slots:
		void timerUpdated();
		void stopClicked();

};

class NoizeSettingsDialog : public QDialog {
		QLabel* infoLabel;
		QLabel* white;
		QDoubleSpinBox* whiteMagn;
		QLabel* pinkM;
		QDoubleSpinBox* pinkMagn;
		QLabel* pinkD;
		QDoubleSpinBox* pinkDiff;
		QLabel* redM;
		QDoubleSpinBox* redMagn;
		QLabel* redD;
		QDoubleSpinBox* redDiff;
		QLabel* blueM;
		QDoubleSpinBox* blueMagn;
		QLabel* blueD;
		QDoubleSpinBox* blueDiff;
		QLabel* violetM;
		QDoubleSpinBox* violetMagn;
		QLabel* violetD;
		QDoubleSpinBox* violetDiff;
		QGridLayout* mainLayout;
		QPushButton* acceptPB;

	public:
		NoizeSettingsDialog(QWidget* parent = nullptr);
		Neural_Network::NoizeInfo get_noize (qint32 order);

};

#endif // NeuralNETSETTINGSDIALOG_HPP
