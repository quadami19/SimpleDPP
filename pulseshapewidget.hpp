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

#ifndef PULSESHAPEWIDGET_HPP
#define PULSESHAPEWIDGET_HPP

#include <QtWidgets>
#include "processing.hpp"

class PulseShapeWidget : public QWidget {

		Q_OBJECT

		std::vector<float> pulseShapeVector;
		std::vector<float> currPulseVector;
		std::vector<float> currErrorVector;
		bool pulseDrawingEnabled = false;
		float smoothing = 0;

	protected:

		virtual void paintEvent(QPaintEvent * _event);
		virtual QSize sizeHint() const { return QSize (200, 100); }

	public:
		PulseShapeWidget (QWidget* _parent = 0x0) : QWidget(_parent) {}
		void capture () { pulseShapeVector = currPulseVector; }
		void set_pulse (const std::vector<float>& pulse);
		void set_shape (const std::vector<float>& _pulseShape);
		void set_size (quint32 size);
		void reset_shape ();
		void set_smoothing (float _smoothing) { smoothing = _smoothing; }
		const std::vector<float>& get_shape () const { return pulseShapeVector; }

	public slots:
		void draw_enable () { pulseDrawingEnabled = true; update(); }
};

#endif // PULSESHAPEWIDGET_HPP
