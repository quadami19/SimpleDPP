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

#ifndef INTERPOLATOR_HPP
#define INTERPOLATOR_HPP

#include <QtGlobal>
#include <QThreadPool>
#include <QObject>
#include <QMutex>
#include <cassert>
#include <memory>
#include <cmath>
#include <iostream>

struct InterpolatorSettings {
		quint32 pointsMult = 2;
		quint32 precision = 16;
		bool artifactRedution = false;
};

class Interpolator {

	protected:
		std::vector<float> buffer;
		std::vector<float> output;
		std::vector<float> const* input;

		InterpolatorSettings settings;

	public:

		Interpolator();
		virtual void set_settings (InterpolatorSettings sett);
		InterpolatorSettings get_settings () const
			{ return settings; }
		void artifact_reduction ();
		void set_input (std::vector<float> const* inp)
			{ input = inp; }
		virtual void interpolate () = 0;
		std::vector<float> const* get_output () const
			{ return &output; }

		void save_settings (std::ostream& os) const;
		void load_settings (std::istream& is);

};


class WhitShanInterpolator : public Interpolator {

		std::vector<float> sinc_tab;

		float sinc (float delta) {
			if (delta == 0) return 1.f;
			return std::sin(3.14159f*delta)/(3.14159f*delta);
		}

		void tabulate_sinc ();

	public:
		WhitShanInterpolator();
		void set_settings(InterpolatorSettings sett);
		void interpolate ();

};


class InterpolationThread : public QRunnable {

	public:
		std::shared_ptr<Interpolator> inter;

		InterpolationThread () : QRunnable() {
			setAutoDelete(false);
			inter = std::shared_ptr<Interpolator> (new WhitShanInterpolator());
		}
		virtual ~InterpolationThread() {}
		void run();

};

#include <QWaitCondition>

class InterpolationClass : public QObject {
		Q_OBJECT


		std::vector<InterpolationThread> interpolators;
		quint32 interType = WhittakerShannon;
		bool interEnabled = false;
		std::shared_ptr<QThreadPool> thisPool;
		QMutex mutex;


	public:

		InterpolationClass();
		~InterpolationClass();
		void set_input (std::vector<float> const* input, quint32 stream)
			{ mutex.lock(); interpolators[stream].inter.get()->set_input(input); mutex.unlock(); }
		std::vector<float> const* get_output (quint32 stream) const
			{ return interpolators[stream].inter.get()->get_output(); }
		void set_inputs (quint32 inputs);
		quint32 get_inputs () const { return interpolators.size(); }
		void set_settings (InterpolatorSettings set)
			{ mutex.lock();
			  for(quint32 stream = 0, ie = interpolators.size(); stream < ie; stream++) interpolators[stream].inter.get()->set_settings(set);
			  mutex.unlock(); }
		InterpolatorSettings get_settings () const
			{ return interpolators[0].inter.get()->get_settings(); }
		void set_inter_type (quint32 type);
		void set_inter_enabled (bool val)
			{ mutex.lock(); interEnabled = val; mutex.unlock(); }
		bool get_inter_enabled() const
			{ return interEnabled; }
		quint32 get_inter_type () const
			{ return interType; }
		void save_settings (std::ostream& os) const;
		void load_settings (std::istream& is);

		enum Interpolators {
			WhittakerShannon = 0,
			TotalIntTypes
		};

	signals:
		void finished();

	public slots:
		void start ();

};

#endif // INTERPOLATOR_HPP
