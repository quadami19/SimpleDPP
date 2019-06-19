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

#ifndef PROCESSINGDIALOG_HPP
#define PROCESSINGDIALOG_HPP

#include <QtWidgets>
#include <QObject>
#include <vector>
#include <memory>

#include "core.hpp"
#include "pulseshapewidget.hpp"

class SearchSettingsDialog;
class DiscriminatorDialog;
class AmplMeasDialog;
class TimeMeasDialog;
class CircuitSettingsDialog;

class ProcessingDialog : public QDialog {
		Q_OBJECT

		Core* corePtr;

		QLabel* infoLabel;

		QLabel* threadsL;
		QComboBox* threadsCB;

		QLabel* pulseSizeL;
		QComboBox* pulseSizeCB;
		QLabel* pulseSearchL;
		QComboBox* pulseSearchCB;
		QPushButton* pulseSearchPB;
		QLabel* pulseDiscrL;
		QComboBox* pulseDiscrCB;
		QPushButton* pulseDiscrPB;
		QLabel* pulseAmplL;
		QComboBox* pulseAmplCB;
		QPushButton* pulseAmplPB;
		QLabel* pulseTimeL;
		QComboBox* pulseTimeCB;
		QPushButton* pulseTimePB;
		QLabel* specL;
		QPushButton* specPB;

		QLabel* shapeSmoothL;
		QSpinBox* shapeSmoothSB;
		QLabel* amplCorrectionL;
		QDoubleSpinBox* amplCorrectionDSB;
		QLabel* subtractL;
		QCheckBox* subtractCB;

		QHBoxLayout* thrdLayout;
		QGridLayout* widgLayout;
		QHBoxLayout* butnLayout;
		QHBoxLayout* acrjLayout;
		QVBoxLayout* mainLayout;

		QPushButton* startPB;
		QPushButton* capturePB;
		QPushButton* applyPB;
		QPushButton* resetPB;

		PulseShapeWidget* shapeWid;
		QTimer* shapeUpdTim;

		QPushButton* acceptPB;
		QPushButton* rejectPB;

		std::vector<SearchSettingsDialog*> searchDialogs;
		std::vector<DiscriminatorDialog*> discrDialogs;
		std::vector<AmplMeasDialog*> amplDialogs;
		std::vector<TimeMeasDialog*> timeDialogs;
		std::vector<CircuitSettingsDialog*> specDialogs;

		std::vector<Neural_Network::NuclearPhysicsNeuralNet>* amplNNsPtr;
		std::vector<Neural_Network::NuclearPhysicsNeuralNet>* timeNNsPtr;
		std::vector<Neural_Network::NuclearPhysicsNeuralNet>* discrNNsPtr;

		std::vector<std::shared_ptr<ProcessingThread::Settings>> oldSettings;
		std::vector<std::shared_ptr<ProcessingThread::Settings>> curSettings;

		void showEvent(QShowEvent *);
		void load_to_dial_settings(int index);
		void update_sub_widgets();
		void set_spec_dial();
		bool check_settings();

	public:
		ProcessingDialog(Core* _ptr, QWidget* parent = nullptr);

	private slots:

		void thread_changed();
		void size_changed();
		void method_changed();
		void settings_accepted();
		void show_search();
		void show_discr();
		void show_ampl();
		void show_time();
		void show_spec();
		void start();
		void capture();
		void apply();
		void reset();
		void accept();
		void reject();


	public slots:
		void processing_finished();
		void state_changed(bool newState);

};

class SettingsDialog : public QDialog {
		Q_OBJECT

	protected:

		QLabel* setInfo;
		QPushButton* acceptPB;
		QPushButton* rejectPB;
		QGridLayout* mainLayout;
		std::shared_ptr<ProcessingThread::Settings> settingsPtr;

	public:
		SettingsDialog(ProcessingDialog* parent = nullptr);
		virtual ~SettingsDialog() {}
		void set_settings(std::shared_ptr<ProcessingThread::Settings> set) { settingsPtr = set; }
		const std::shared_ptr<ProcessingThread::Settings> get_settings () const { return settingsPtr; }
		virtual bool check_settings(std::shared_ptr<ProcessingThread::Settings> sett) = 0;
		virtual void update_settings() = 0;

	public slots:
		virtual void accept() = 0;

};

class SearchSettingsDialog : public SettingsDialog {
		Q_OBJECT

	protected:

		QLabel* skipSamplesL;
		QSpinBox* skipSamplesSB;

	public:
		SearchSettingsDialog(ProcessingDialog* parent = nullptr);
		virtual ~SearchSettingsDialog() {}
		virtual quint32 get_search_type() const = 0;

};

class SearchThresholdDialog : public SearchSettingsDialog {
		Q_OBJECT

		QLabel* baselineSamplesL;
		QSpinBox* baselineSamplesSB;
		QLabel* detectPosL;
		QSpinBox* detectPosSB;
		QLabel* thresholdL;
		QDoubleSpinBox* thresholdDSB;

		void showEvent(QShowEvent *);

	public:
		SearchThresholdDialog(ProcessingDialog* parent = nullptr);
		~SearchThresholdDialog() {}
		quint32 get_search_type() const { return PulseSearching::Threshold; }
		bool check_settings(std::shared_ptr<ProcessingThread::Settings> sett);

	public slots:
		void update_settings();

	private slots:

		void accept();
};

class SearchMonotonousDialog : public SearchSettingsDialog {
		Q_OBJECT

		QLabel* risingSamplesL;
		QSpinBox* risingSamplesSB;
		QLabel* indiffSamplesL;
		QSpinBox* indiffSamplesSB;
		QLabel* fallingSamplesL;
		QSpinBox* fallingSamplesSB;

		void showEvent(QShowEvent *);

	public:
		SearchMonotonousDialog(ProcessingDialog* parent = nullptr);
		~SearchMonotonousDialog() {}
		quint32 get_search_type() const { return PulseSearching::Monoton; }
		bool check_settings(std::shared_ptr<ProcessingThread::Settings> sett);

	public slots:
		void update_settings();

	private slots:

		void accept();
		void check_values();
};

class SearchTanThresholdDialog : public SearchSettingsDialog {
		Q_OBJECT

		QLabel* risingSamplesL;
		QSpinBox* risingSamplesSB;
		QLabel* indiffSamplesL;
		QSpinBox* indiffSamplesSB;
		QLabel* fallingSamplesL;
		QSpinBox* fallingSamplesSB;
		QLabel* risingTanL;
		QDoubleSpinBox* risingTanDSB;
		QLabel* fallingTanL;
		QDoubleSpinBox* fallingTanDSB;

		void showEvent(QShowEvent *);

	public:
		SearchTanThresholdDialog(ProcessingDialog* parent = nullptr);
		~SearchTanThresholdDialog() {}
		quint32 get_search_type() const { return PulseSearching::TanThreshold; }
		bool check_settings(std::shared_ptr<ProcessingThread::Settings> sett);

	public slots:
		void update_settings();

	private slots:

		void accept();
		void check_values();
};

class DiscriminatorDialog : public SettingsDialog {
		Q_OBJECT

	protected:

		QLabel* discEnabledL;
		QCheckBox* discEnabledCB;

	public:

		DiscriminatorDialog(ProcessingDialog* parent = nullptr);
		virtual ~DiscriminatorDialog() {}
		virtual quint32 get_disc_type () = 0;
};

class DiscriminatorDispersionDialog : public DiscriminatorDialog {
		Q_OBJECT

		QLabel* maxDisperL;
		QDoubleSpinBox* maxDisperDSB;

		void showEvent(QShowEvent *);

	public:

		DiscriminatorDispersionDialog(ProcessingDialog* parent = nullptr);
		virtual ~DiscriminatorDispersionDialog () {}
		quint32 get_disc_type() { return PulseDiscriminator::Dispersion; }
		bool check_settings(std::shared_ptr<ProcessingThread::Settings> sett);

	public slots:
		void update_settings();

	private slots:

		void accept();
		void check_values();
};

class DiscriminatorNeuralNetDialog : public DiscriminatorDialog {
		Q_OBJECT

		QLabel* nnListL;
		QComboBox* nnListCB;
		QLabel* nnInputsL;
		QLabel* nnInputsI;

		std::vector<Neural_Network::NuclearPhysicsNeuralNet>* discrPtr;

		void showEvent(QShowEvent *);

	public:

		DiscriminatorNeuralNetDialog(std::vector<Neural_Network::NuclearPhysicsNeuralNet>* discr, ProcessingDialog* parent = nullptr);
		virtual ~DiscriminatorNeuralNetDialog () {}
		quint32 get_disc_type() { return PulseDiscriminator::NeuralNet; }
		bool check_settings(std::shared_ptr<ProcessingThread::Settings> sett);

	public slots:
		void update_settings();

	private slots:

		void accept();
		void check_values();
};

class AmplMeasDialog : public SettingsDialog {
		Q_OBJECT

	protected:

		QLabel* blSamplesL;
		QSpinBox* blSamplesSB;

	public:

		AmplMeasDialog(ProcessingDialog* parent = nullptr);
		virtual ~AmplMeasDialog() {}
		virtual quint32 get_ampl_type() = 0;
};

class AmplMeasMaxDialog : public AmplMeasDialog {
		Q_OBJECT

		QLabel* maxLeftPosL;
		QSpinBox* maxLeftPosSB;
		QLabel* maxRightPosL;
		QSpinBox* maxRightPosSB;

		void showEvent(QShowEvent *);

	public:

		AmplMeasMaxDialog(ProcessingDialog* parent = nullptr);
		virtual ~AmplMeasMaxDialog() {}
		quint32 get_ampl_type() { return PulseAmplMeasuring::MaxVal; }
		bool check_settings(std::shared_ptr<ProcessingThread::Settings> sett);

	public slots:
		void update_settings();

	private slots:

		void accept();
		void check_values();
};

class AmplMeasPolyMaxDialog : public AmplMeasDialog {
		Q_OBJECT

		QLabel* polyOrderL;
		QSpinBox* polyOrderSB;
		QLabel* maxLeftPosL;
		QSpinBox* maxLeftPosSB;
		QLabel* maxRightPosL;
		QSpinBox* maxRightPosSB;

		void showEvent(QShowEvent *);

	public:

		AmplMeasPolyMaxDialog(ProcessingDialog* parent = nullptr);
		virtual ~AmplMeasPolyMaxDialog() {}
		quint32 get_ampl_type() { return PulseAmplMeasuring::Polynomial; }
		bool check_settings(std::shared_ptr<ProcessingThread::Settings> sett);

	public slots:
		void update_settings();

	private slots:

		void accept();
		void check_values();
};

class AmplMeasNeuralNetDialog : public AmplMeasDialog {
		Q_OBJECT

		QLabel* nnListL;
		QComboBox* nnListCB;
		QLabel* nnInputsL;
		QLabel* nnInputsI;

		std::vector<Neural_Network::NuclearPhysicsNeuralNet>* amplPtr;

		void showEvent(QShowEvent *);

	public:

		AmplMeasNeuralNetDialog(std::vector<Neural_Network::NuclearPhysicsNeuralNet>* ampl, ProcessingDialog* parent = nullptr);
		virtual ~AmplMeasNeuralNetDialog() {}
		quint32 get_ampl_type() { return PulseAmplMeasuring::NeuralNet; }
		bool check_settings(std::shared_ptr<ProcessingThread::Settings> sett);

	public slots:
		void update_settings();

	private slots:

		void accept();
		void check_values();
};

class TimeMeasDialog : public SettingsDialog {
		Q_OBJECT

	public:

		TimeMeasDialog(ProcessingDialog* parent = nullptr) : SettingsDialog (parent) {}
		virtual ~TimeMeasDialog() {}
		virtual quint32 get_time_type() = 0;
};

class TimeMeasMaxDialog : public TimeMeasDialog {
		Q_OBJECT

		QLabel* maxLeftPosL;
		QSpinBox* maxLeftPosSB;
		QLabel* maxRightPosL;
		QSpinBox* maxRightPosSB;

		void showEvent(QShowEvent *);

	public:

		TimeMeasMaxDialog(ProcessingDialog* parent = nullptr);
		virtual ~TimeMeasMaxDialog() {}
		quint32 get_time_type() { return PulseTimeMeasuring::MaxVal; }
		bool check_settings(std::shared_ptr<ProcessingThread::Settings> sett);

	public slots:
		void update_settings();

	private slots:

		void accept();
		void check_values();
};

class TimeMeasNeuralNetDialog : public TimeMeasDialog {
		Q_OBJECT

		QLabel* nnListL;
		QComboBox* nnListCB;
		QLabel* nnInputsL;
		QLabel* nnInputsI;

		std::vector<Neural_Network::NuclearPhysicsNeuralNet>* timePtr;

		void showEvent(QShowEvent *);

	public:

		TimeMeasNeuralNetDialog(std::vector<Neural_Network::NuclearPhysicsNeuralNet>* time, ProcessingDialog* parent = nullptr);
		virtual ~TimeMeasNeuralNetDialog() {}
		quint32 get_time_type() { return PulseTimeMeasuring::NeuralNet; }
		bool check_settings(std::shared_ptr<ProcessingThread::Settings> sett);

	public slots:
		void update_settings();

	private slots:

		void accept();
		void check_values();
};

class CircuitSettingsDialog : public SettingsDialog {
		Q_OBJECT

	protected:
		QLabel* currDataStreamL;
		QComboBox* currDataStreamCB;

		Core* corePtr;

		quint32 currThread = 0;

		virtual void showEvent(QShowEvent *);

	public:
		CircuitSettingsDialog (Core* core, ProcessingDialog* parent = nullptr);
		virtual ~CircuitSettingsDialog () {}

		void set_current_thread (quint32 thread)
			{ currThread = thread; }

		virtual bool check_settings(std::shared_ptr<ProcessingThread::Settings> sett);

	public slots:
		void update_settings();
		virtual void accept();
};

class CoincidenceSettingsDialog : public CircuitSettingsDialog {
		Q_OBJECT

		QLabel* coinThreadL;
		QComboBox* coinThreadCB;
		QLabel* coinAmplMinL;
		QDoubleSpinBox* coinAmplMinDSB;
		QLabel* coinAmplMaxL;
		QDoubleSpinBox* coinAmplMaxDSB;
		QLabel* coinTimeDiffL;
		QDoubleSpinBox* coinTimeDiffDSB;

		void showEvent(QShowEvent *);

	public:
		CoincidenceSettingsDialog (Core* core, ProcessingDialog* parent = nullptr);
		virtual ~CoincidenceSettingsDialog () {}
		bool check_settings(std::shared_ptr<ProcessingThread::Settings> sett);

	public slots:
		void update_settings();

	private slots:

		void accept();
		void check_values();
};


#endif // PROCESSINGDIALOG_HPP
