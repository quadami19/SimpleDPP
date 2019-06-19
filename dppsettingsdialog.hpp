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

#ifndef PulseProcessingDialog_HPP
#define PulseProcessingDialog_HPP

#include <QtWidgets>
#include <QDialog>
#include <QTabWidget>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QPushButton>
#include <QTimer>
#include <QResizeEvent>
#include "pulseshapewidget.hpp"
#include "core.hpp"
#include "pulseparametersprocessor.hpp"
#include <QFileDialog>
#include <QErrorMessage>
#include <QMessageBox>
#include <iostream>
#include <iomanip>

const quint32 detectingPulseWidgetUpdateInterval = 25;
const float detectingSmoothingValueDefault = 0.f;

class PulseProcessingDialog : public QDialog {

		Q_OBJECT
		Core* coreClassPtr;
		QComboBox* currentStream = 0x0;
		QPushButton* apply4currPB;
		QHBoxLayout* streamLayout;
		QTabWidget * tabWidget;

		QWidget* detectingTabWidget;
		QLabel* detectingMethodLabel;
		QComboBox* detectingMethodComBox;
		QLabel* detectingThresholdLabel;
		QDoubleSpinBox* detectingThresholdDSB;
		QLabel* detectingTotalPointsLabel;
		QLabel* detectingBLPointsLabel;
		QLabel* detectingRisingPointsLabel;
		QLabel* detectingIndiffPointsLabel;
		QLabel* detectingFallingPointsLabel;
		QSpinBox* detectingTotalPointsSB;
		QSpinBox* detectingBLPointsSB;
		QSpinBox* detectingRisingPointsSB;
		QSpinBox* detectingIndiffPointsSB;
		QSpinBox* detectingFallingPointsSB;
		QLabel* detectingCritRiseAngleLabel;
		QLabel* detectingCritFallAngleLabel;
		QDoubleSpinBox* detectingCritRiseAngleDSB;
		QDoubleSpinBox* detectingCritFallAngleDSB;
		QLabel* detectingMaxDispersionLabel;
		QDoubleSpinBox* detectingMaxDispersionDSBox;
		QLabel* detectingEnableSubtractionLabel;
		QCheckBox* detectingEnableSubtractionChBox;
		PulseShapeWidget* detectingPulseShapeWid;
		QLabel* detectingSmoothingLabel;
		QDoubleSpinBox* detectingSmoothingDSBox;
		QGridLayout* detectingParametersLayout;
		QPushButton* detectingStartPB;
		QPushButton* detectingCapturePB;
		QPushButton* detectingApplyPB;
		QPushButton* detectingResetPB;
		QHBoxLayout* detectingButtonsLayout;
		QVBoxLayout* detectingLayout;
		std::vector<float> detectingPulseShape = std::vector<float> ();
		quint32 detectingOldParameterSelectedMethod = detectingMethodDefault;
		quint32 detectingOldParameterTotalPoints = detectingTotalPointsDefault;
		quint32 detectingOldParameterBLPoints = baselinePointsDefault;
		quint32 detectingOldParameterRisingPoints = detectingRisingPointsDefault;
		quint32 detectingOldParameterIndiffPoints = detectingIndiffPointsDefault;
		quint32 detectingOldParameterFallingPoints = detectingFallingPointsDefault;
		float detectingOldParameterThreshold = detectingThresholdDefault;
		float detectingOldParameterRisingAngle = detectingRisingAngleDefault;
		float detectingOldParameterFallingAngle = detectingFallingAngleDefault;
		float detectingOldParameterMaxDispersion = detectingMaxDispersionDefault;
		float detectingOldParameterSmoothingValue = detectingSmoothingValueDefault;
		quint32 processingOldParameterProcMethod = processingMethodDefault;
		quint32 processingOldParameterPolyOrder = processingPolynomialOrderDefault;
		float processingOldParameterMult = 1.f;
		float processingOldParameterBLVal = processingBLIIRParamDefault;
		bool processingOldParameterSubCB = true;
		std::vector<float> detectingOldParameterPulseShape;
		QTimer* detectingWidgetUpdateTimer;

		QWidget* processingTabWidget;
		QLabel* processingMethodsLabel;
		QComboBox* processingMethodsCB;
		QLabel* processingMultiplicatorLabel;
		QDoubleSpinBox* processingMultiplicatorDSB;
		QLabel* processingSubtractionEnableLabel;
		QCheckBox* processingSubtractionEnableCB;
		//QLabel* processingBLTypeLabel;
		//QComboBox* processingBLTypeCB;
		QLabel* processingBLTypeIIRParamLabel;
		QDoubleSpinBox* processingBLTypeIIRParamDSB;
		//QLabel* processingBLTypeFIRPointsLabel;
		//QSpinBox* processingBLTypeFIRPointsSB;
		QLabel* processingPulseShapeLabel;
		QLabel* processingPulseShapeStatus;
		QLabel* processingPolynomialOrderLabel;
		QSpinBox* processingPolynomialOrderSB;
		QLabel* processingPerceptronLabel;
		QLabel* processingPerceptronStatus;
		QLabel* processingDiscriminatingPerceptronLabel;
		QLabel* processingDiscriminatingPerceptronStatus;
		QGridLayout* processingLayout;

		QPushButton* acceptPB;
		QPushButton* declinePB;
		QHBoxLayout* buttonsLayout;

		QVBoxLayout* mainLayout;

		std::vector<std::pair<quint32, quint32>> procIndex;


	protected:

		virtual void closeEvent(QCloseEvent* _event) { Q_UNUSED (_event); reject(); }
		virtual void showEvent(QShowEvent * _event);
		virtual void hideEvent(QHideEvent * _event);

	public:
                PulseProcessingDialog(Core* _core, QWidget* _parent = 0x0);
                ~PulseProcessingDialog ();
		void save_settings(std::ostream& ostr);
		void load_settings(std::istream& istr);

	public slots:

		void reload_values();
		void updateWidget();
		void processing_finished ();

	private slots:
		void detectingPointsSBValueChanged();
		void detectingStartClicked();
		void detectingCaptureClicked();
		void detectingApplyClicked();
		void detectingResetClicked();

		void accept();
		void reject();
		void apply();
		void stream_cb_changed();
};
#endif // PulseProcessingDialog_HPP
