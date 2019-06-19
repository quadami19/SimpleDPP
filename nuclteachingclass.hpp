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

#ifndef NUCLTEACHINGCLASS_HPP
#define NUCLTEACHINGCLASS_HPP
#include "teachingclass.hpp"
#include "nuclearphysicsperceptron.hpp"
#include "fft.hpp"
#include <random>
#include <list>

namespace Neural_Network {

struct NoizeInfo {
		qint32 order = 0;
		float magnitude = 1.f;
		float diff = 0.f;
};

class NuclTeachingClass : public TeachingClass {

	protected:

		std::default_random_engine generator;
		std::normal_distribution<float>* gauChance;
		std::uniform_real_distribution<float>* linChance;
		std::list<NoizeInfo> ni;
		std::vector<std::vector<float>> noizeAFCs;
		FFT<float> fft;

		std::vector<float> pulse;
		std::vector<float> interpolated;
		std::vector<float> sincTab;
		quint32 interMult = 16;
		quint32 interPrec = 32;
		float pulMaxTime;

		std::vector<float> get_random (quint32 size);
		void add_noize(std::vector<float>::iterator iter, quint32 size);
		//void add_noize_shaped(std::vector<float>::iterator iter, quint32 size);
		void overlay(std::vector<float>::iterator iter, const std::vector<float>& input, float magnitude = 1.f);

		std::vector<float> interpolate (const std::vector<float>& input);
		std::vector<float> deinterpolate (const std::vector<float>& input, quint32 displ = 0);

		virtual void assert_nn(NuclearPhysicsNeuralNet *_percep) = 0;

		float lastTime;
	public:

		NuclTeachingClass();
		virtual ~NuclTeachingClass();

		void set_pulse (const std::vector<float>& _pulse);
		void set_noize (qint32 order, float magn, float diff);
		void set_noize (NoizeInfo n);
		void init_noizes();
		NoizeInfo get_noize (qint32 order);
		void reset_noizes ();

		double start (NuclearPhysicsNeuralNet *_percep, quint32 iterations);
		virtual std::pair<std::vector<float>, std::vector<float>> generate_pulse () = 0;
};

class NuclAmplTeachingClass : public NuclTeachingClass {

	public:
		NuclAmplTeachingClass() : NuclTeachingClass() {}
		virtual ~NuclAmplTeachingClass() {}

		void assert_nn (NuclearPhysicsNeuralNet *_percep);

		std::pair<std::vector<float>, std::vector<float> > generate_pulse();
};

class NuclTimeTeachingClass : public NuclTeachingClass {

	public:
		NuclTimeTeachingClass() : NuclTeachingClass() {}
		virtual ~NuclTimeTeachingClass() {}

		void assert_nn (NuclearPhysicsNeuralNet *_percep);

		std::pair<std::vector<float>, std::vector<float> > generate_pulse();
};

class NuclDiscrTeachingClass : public NuclTeachingClass {

	public:
		NuclDiscrTeachingClass() : NuclTeachingClass() {}
		virtual ~NuclDiscrTeachingClass() {}

		void assert_nn (NuclearPhysicsNeuralNet *_percep);

		std::pair<std::vector<float>, std::vector<float> > generate_pulse();
};

}
#endif // NUCLTEACHINGCLASS_HPP
