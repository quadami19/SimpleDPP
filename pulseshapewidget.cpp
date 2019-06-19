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

#include "pulseshapewidget.hpp"

void PulseShapeWidget::set_pulse(const std::vector<float>& pulse) {
	assert (!pulse.empty());
	float max = *std::max_element (pulse.begin(), pulse.end());
	if (!(max > 0.005f) || !(max < 1.f)) return;

	currPulseVector.resize(pulse.size(), 0.f);
	currErrorVector.resize(pulse.size(), 0.f);
	for (quint32 i = 0; i < currPulseVector.size(); ++i) {
		currPulseVector[i] = currPulseVector[i]*smoothing + pulse[i]/max * (1.f - smoothing);
	}
	if (!pulseShapeVector.empty()) for (quint32 i = 0; i < currPulseVector.size(); ++i)
		currErrorVector[i] = currErrorVector[i] * smoothing + (pulseShapeVector[i] - currPulseVector[i]) * (1.f - smoothing);
}

void PulseShapeWidget::set_shape(const std::vector<float> &_pulseShape) {
	currErrorVector.resize(_pulseShape.size(), 0.f);
	std::fill(currErrorVector.begin(), currErrorVector.end(), 0.f);
	currPulseVector = _pulseShape;
	pulseShapeVector = _pulseShape;
}

void PulseShapeWidget::set_size(quint32 size) {
	assert (size);
	currPulseVector.resize(size, 0.f);
	if (pulseShapeVector.size()) {
		pulseShapeVector.resize(size, 0.f);
		currErrorVector.resize(size, 0.f);
	} else {
		pulseShapeVector.resize(0);
		currErrorVector.resize(0);
	}
}

void PulseShapeWidget::reset_shape() {
	std::fill(currPulseVector.begin(), currPulseVector.end(), 0.f);
	std::fill(currErrorVector.begin(), currErrorVector.end(), 0.f);
	pulseShapeVector.resize(0);
}

void PulseShapeWidget::paintEvent(QPaintEvent *_event) {
	if (!pulseDrawingEnabled) _event->ignore();
	Q_UNUSED (_event);
	QPainter painter (this);
	QPen pen;
	pen.setColor(Qt::black);
	pen.setWidth(2);
	painter.setPen(pen);
	painter.setBrush(QBrush(Qt::white));
	painter.drawRect(QRect(1, 1, size().width() - 2, size().height() - 2));
	pen.setWidth(1);
	pen.setStyle(Qt::DotLine);
	pen.setColor(Qt::black);
	painter.setPen(pen);
	painter.drawLine(0, size().height() * 0.5f, size().width(), size().height() * 0.5f);
	pen.setStyle(Qt::SolidLine);

	if (pulseDrawingEnabled && !currPulseVector.empty()) {
		float totSize = static_cast<float> (currPulseVector.size() - 1);
		float maxElem;
		QPointF point1, point2;

		if (pulseShapeVector.size() != 0) {
			maxElem = 1.1f * (*std::max_element (pulseShapeVector.begin(), pulseShapeVector.end()));
			if (maxElem > 0.001f || maxElem < -0.001f) {
				pen.setColor(Qt::green);
				painter.setBrush(QBrush(Qt::green));
				painter.setPen(pen);
				point1.rx() = 0;
				point1.ry() = 0.5f * (1.0f - pulseShapeVector[0] / maxElem) * size().height();
				painter.drawEllipse(point1, 2, 2);

				for (quint16 i = 1; i < pulseShapeVector.size(); ++i) {
					point2.rx() = (static_cast<float> (i) / totSize) * size().width();
					point2.ry() = size().height() * 0.5f * (1.0f - pulseShapeVector[i] / maxElem);
					painter.drawEllipse(point2, 2, 2);
					painter.drawLine(point1, point2);
					point1 = point2;
				}
				if (currErrorVector.size() != 0) {
					pen.setColor(Qt::red);
					painter.setPen(pen);
					point1.rx() = 0;
					point1.ry() = 0.5f * (1.0f - currErrorVector[0] / maxElem) * size().height();

					for (quint32 i = 1; i < currErrorVector.size(); ++i) {
						point2.rx() = (static_cast<float> (i) / totSize) * size().width();
						point2.ry() = 0.5f * (1.0f - currErrorVector[i] / maxElem) * size().height();
						painter.drawLine(point1, point2);
						point1 = point2;
					}
				}
			}
		}
		pen.setColor(Qt::blue);
		painter.setBrush(QBrush(Qt::blue));
		painter.setPen(pen);

		maxElem = 1.1f * (*std::max_element (currPulseVector.begin(), currPulseVector.end()));
		if (maxElem > 0.001f || maxElem < -0.001f) {
			point1.rx() = 0;
			point1.ry() = size().height() * 0.5f * (1.0f - currPulseVector[0] / maxElem);
			painter.drawEllipse(point1, 2, 2);
			for (quint16 i = 1; i < currPulseVector.size(); ++i) {
				point2.rx() = (static_cast<float> (i) / totSize) * size().width();
				point2.ry() = size().height() * 0.5f * (1.0f - currPulseVector[i] / maxElem);
				painter.drawEllipse(point2, 2, 2);
				painter.drawLine(point1, point2);
				point1 = point2;
			}
		}

	}
	pulseDrawingEnabled = false;

}
