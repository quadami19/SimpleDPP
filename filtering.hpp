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

#ifndef FILTERING_HPP
#define FILTERING_HPP

#include <QObject>
#include <QThreadPool>
#include <QMutex>
#include <stdexcept>
#include <vector>
#include <cassert>
#include <memory>
#include <cmath>
#include <iostream>

class FilteringProcessor;

class Filter {

	protected:
		std::vector<float> const* inputPtr;
		std::vector<float> output;
		std::vector<float> buffer;

	public:
		Filter (quint32 dataSize) { output.resize(dataSize); }
		virtual ~Filter() {}
		void set_size (quint32 size)
			{ output.resize(size, 0.f); }
		quint32 get_size() const
			{ return output.size(); }
		void set_data (std::vector<float> const* data)
			{ assert (data->size() == output.size()); inputPtr = data; }
		std::vector<float> const* get_data () const
			{ return &output; }
		void clear_buffer ()
			{ std::fill(buffer.begin(), buffer.end(), 0.f); }
		virtual quint32 get_filter_id () const = 0;
		virtual void save_settings (std::ostream& os) const = 0;
		virtual void load_settings (std::istream& is) = 0;
		virtual void process() = 0;

		class FilterSettings {
			public:
				FilterSettings() {}
				virtual FilterSettings& operator= (const FilterSettings& a) = 0;
				virtual quint32 get_filter_id() const = 0;
				virtual ~FilterSettings() {}
		};

		virtual void set (std::shared_ptr<FilterSettings> settings) = 0;
		virtual std::shared_ptr<FilterSettings> get () = 0;
};

class Shaper : public Filter {

	protected:
		std::vector<float> shape;
		virtual void set_shape() = 0;
	public:
		Shaper(quint32 dataSize) : Filter (dataSize) {}
		void process();
};

class FilteringThread : public QRunnable {
		std::vector<std::shared_ptr<Filter>> filters;
		std::vector<float> const* inputPtr;
		quint32 size;
		bool isEnabled;

	public:
		FilteringThread(quint32 dataSize);
		virtual ~FilteringThread();
		void run ();
		void set_enabled (bool state)
			{ isEnabled = state; }
		bool get_enabled () const
			{ return isEnabled; }
		void set_size (quint32 size);
		void set_input (std::vector<float> const* input);
		std::vector<float> const* get_output () const;
		void set (std::shared_ptr<Filter::FilterSettings> settings, quint32 filter_num)
			{ assert (filter_num < filters.size()); filters[filter_num]->set (settings); }
		std::shared_ptr<Filter::FilterSettings> get (quint32 filter_num)
			{ assert (filter_num < filters.size()); return filters[filter_num]->get(); }
		void add_filter (quint32 filter_id);
		void del_filter (quint32 filter_num);
		quint32 get_filter_count () const;
		void set_filter_id (quint32 filter_id, quint32 filter_num);
		quint32 get_filter_id (quint32 filter_num) const;
		void save_settings (std::ostream& os) const;
		void load_settings (std::istream& is);
};

class FilteringProcessor : public QObject {
		Q_OBJECT

		std::vector<std::vector<float> const*> inputPtrs;
		quint32 dataSize;
		std::vector<FilteringThread*> filterStreams;
		std::shared_ptr<QThreadPool> thisPool;
		QMutex mutex;
	public:
		FilteringProcessor(quint32 size);
		FilteringProcessor (const FilteringProcessor&) = delete;
		~FilteringProcessor ();


		void set_enabled (bool state, quint32 stream)
			{  assert(stream < filterStreams.size()); mutex.lock(); filterStreams[stream]->set_enabled(state); mutex.unlock(); }
		bool get_enabled (quint32 stream) const
			{ assert(stream < filterStreams.size()); return filterStreams[stream]->get_enabled(); }

		void set_streams (quint32 streams);
		quint32 get_streams () const { return filterStreams.size(); }
		void add_filter (quint32 filter_id, quint32 stream)
			{ assert(stream < filterStreams.size()); mutex.lock(); filterStreams[stream]->add_filter(filter_id); mutex.unlock(); }
		void del_filter (quint32 stream, quint32 filter_num)
			{ assert(stream < filterStreams.size()); mutex.lock(); filterStreams[stream]->del_filter(filter_num); mutex.unlock(); }
		quint32 get_filter_count (quint32 stream) const
			{ assert(stream < filterStreams.size()); return filterStreams[stream]->get_filter_count(); }
		void set_filter_id (quint32 filter_id, quint32 stream, quint32 filter_num)
			{ assert(stream < filterStreams.size()); mutex.lock(); filterStreams[stream]->set_filter_id(filter_id, filter_num); mutex.unlock(); }
		quint32 get_filter_id (quint32 stream, quint32 filter_num) const
			{ assert(stream < filterStreams.size()); return filterStreams[stream]->get_filter_id(filter_num); }

		void set_size (quint32 size);
		quint32 get_size () const
			{ return dataSize; }
		void set_input (std::vector<float> const* input, quint32 stream)
			{ assert(stream < filterStreams.size());  mutex.lock(); inputPtrs[stream] = input; mutex.unlock(); }
		std::vector<float> const* get_output (quint32 stream) const
			{ assert(stream < filterStreams.size()); return filterStreams[stream]->get_output(); }

		void set (std::shared_ptr<Filter::FilterSettings> settings, quint32 stream, quint32 filter_num)
			{ assert(stream < filterStreams.size()); mutex.lock(); filterStreams[stream]->set(settings, filter_num); mutex.unlock(); }
		std::shared_ptr<Filter::FilterSettings> get (quint32 stream, quint32 filter_num) const
			{ assert(stream < filterStreams.size()); return filterStreams[stream]->get(filter_num); }

		void save_settings (std::ostream& os) const;
		void load_settings (std::istream& is);

		enum AvailableFilters {
			Delay = 0,
			DelayLine,
			OverrunningLine,
			RC_IIR,
			CR_IIR,
			CR_REV_IIR,
			MovingAverage,
			Trapezoidal,
			Gaussian,
			Cusp,
			TotalAvailable
		};

		signals:

		void finished();

	public slots:

		void process ();
};

class Delay : public Filter {
		quint32 delayTime;

	public:
		Delay(quint32 dataSize) : Filter (dataSize) {}
		~Delay() {}
		void set (std::shared_ptr<FilterSettings> settings);
		std::shared_ptr<FilterSettings> get ();
		quint32 get_filter_id () const;
		void save_settings(std::ostream& os) const;
		void load_settings(std::istream& is);
		void process();

		class Settings : public FilterSettings {
			public:
				quint32 get_filter_id() const { return FilteringProcessor::Delay; }
				Settings& operator= (const FilterSettings& a);
				quint32 time = 1;
		};

};

class DelayLine : public Filter {
		quint32 delayTime;
		float delayAmplit;

	public:
		DelayLine(quint32 dataSize) : Filter (dataSize) {}
		~DelayLine() {}
		void set (std::shared_ptr<FilterSettings> settings);
		std::shared_ptr<FilterSettings> get ();
		quint32 get_filter_id () const;
		void save_settings(std::ostream& os) const;
		void load_settings(std::istream& is);
		void process();

		class Settings : public FilterSettings {
			public:
				quint32 get_filter_id() const { return FilteringProcessor::DelayLine; }
				Settings& operator= (const FilterSettings& a);
				quint32 time = 1;
				float ampl = 0.5;
		};

};

class OverrunLine : public Filter {
		quint32 overrunTime;
		float overrunAmplit;

	public:
		OverrunLine(quint32 dataSize) : Filter (dataSize) {}
		~OverrunLine() {}
		void set (std::shared_ptr<FilterSettings> settings);
		std::shared_ptr<FilterSettings> get ();
		quint32 get_filter_id () const;
		void save_settings(std::ostream& os) const;
		void load_settings(std::istream& is);
		void process();

		class Settings : public FilterSettings {
			public:
				quint32 get_filter_id() const { return FilteringProcessor::OverrunningLine; }
				Settings& operator= (const FilterSettings& a);
				quint32 time = 1;
				float ampl = 0.5;
		};

};

class R_C_Filter : public Filter {
	protected:

		float alpha = 0.5;

	public:

		R_C_Filter (quint32 dataSize) : Filter (dataSize) { buffer.resize(1, 0.f); }
		~R_C_Filter () {}

		void set (std::shared_ptr<FilterSettings> settings);

		void save_settings(std::ostream& os) const;
		void load_settings(std::istream& is);

		virtual quint32 get_filter_id() const = 0;
		virtual void process() = 0;

		class RC_Settings : public FilterSettings {
			public:
				RC_Settings& operator= (const FilterSettings& a);
				float alpha = 0.f;
		};

};

class RC_IIR_Filter : public R_C_Filter {

	public:

		RC_IIR_Filter (quint32 dataSize) : R_C_Filter (dataSize) {}
		~RC_IIR_Filter () {}

		std::shared_ptr<FilterSettings> get ();

		quint32 get_filter_id () const;
		void process();

		class Settings : public RC_Settings {
			public:
				quint32 get_filter_id() const { return FilteringProcessor::RC_IIR; }
		};

};

class CR_IIR_Filter : public R_C_Filter {


	public:

		CR_IIR_Filter (quint32 dataSize) : R_C_Filter (dataSize) {}
		~CR_IIR_Filter () {}

		std::shared_ptr<FilterSettings> get ();

		quint32 get_filter_id () const;
		void process();

		class Settings : public RC_Settings {
			public:
				quint32 get_filter_id() const { return FilteringProcessor::CR_IIR; }
		};

};

class CR_REV_IIR_Filter : public R_C_Filter {


	public:

		CR_REV_IIR_Filter (quint32 dataSize) : R_C_Filter (dataSize) {}
		~CR_REV_IIR_Filter () {}

		std::shared_ptr<FilterSettings> get ();

		quint32 get_filter_id () const;
		void process();

		class Settings : public RC_Settings {
			public:
				quint32 get_filter_id() const { return FilteringProcessor::CR_REV_IIR; }
		};

};

class MovingAverage : public Shaper {
		quint32 size = 3;

		void set_shape();

	public:
		MovingAverage(quint32 dataSize) : Shaper (dataSize) {}
		~MovingAverage() {}

		void set (std::shared_ptr<FilterSettings> settings);
		std::shared_ptr<FilterSettings> get ();
		quint32 get_filter_id () const;
		void save_settings(std::ostream& os) const;
		void load_settings(std::istream& is);

		class Settings : public FilterSettings {
			public:
				quint32 get_filter_id() const { return FilteringProcessor::MovingAverage; }
				Settings& operator= (const FilterSettings& a);
				quint32 size = 3;
		};

};

class TrapezoidalShaper : public Shaper {
		quint32 sideSz = 3;
		quint32 flatSz = 2;

		void set_shape();

	public:
		TrapezoidalShaper(quint32 dataSize) : Shaper (dataSize) {}
		~TrapezoidalShaper() {}

		void set (std::shared_ptr<FilterSettings> settings);
		std::shared_ptr<FilterSettings> get ();
		quint32 get_filter_id () const;
		void save_settings(std::ostream& os) const;
		void load_settings(std::istream& is);

		class Settings : public FilterSettings {
			public:
				quint32 get_filter_id() const { return FilteringProcessor::Trapezoidal; }
				Settings& operator= (const FilterSettings& a);
				quint32 side = 3;
				quint32 flat = 2;
		};

};

class GaussianShaper : public Shaper {
		float width = 1.f;

		void set_shape();

	public:
		GaussianShaper(quint32 dataSize) : Shaper (dataSize) {}
		~GaussianShaper() {}

		void set (std::shared_ptr<FilterSettings> settings);
		std::shared_ptr<FilterSettings> get ();

		quint32 get_filter_id () const;
		void save_settings(std::ostream& os) const;
		void load_settings(std::istream& is);

		class Settings : public FilterSettings {
			public:
				quint32 get_filter_id() const { return FilteringProcessor::Gaussian; }
				Settings& operator= (const FilterSettings& a);
				float width = 1.f;
		};

};

class CuspShaper : public Shaper {
		float width = 1.f;
		quint32 flatSz = 2;

		void set_shape();

	public:
		CuspShaper(quint32 dataSize) : Shaper (dataSize) {}
		~CuspShaper() {}

		void set (std::shared_ptr<FilterSettings> settings);
		std::shared_ptr<FilterSettings> get ();

		quint32 get_filter_id () const;
		void save_settings(std::ostream& os) const;
		void load_settings(std::istream& is);

		class Settings : public FilterSettings {
			public:
				quint32 get_filter_id() const { return FilteringProcessor::Cusp; }
				Settings& operator= (const FilterSettings& a);
				float width = 1.f;
				quint32 flat = 2;
		};

};

#endif // FILTERING_HPP
