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

#ifndef FTT_HPP
#define FTT_HPP

#include <complex>
#include <cmath>
#include <cassert>
#include <vector>

const double PI = 3.141592653;


template <typename T> class FFT {

		std::vector<std::complex<T>> dataMassTmp;
		std::vector<std::complex<T>> expMassPlus;
		std::vector<std::complex<T>> expMassMinus;
		std::vector<uint32_t> bitReversMass;
		uint32_t size;
		uint32_t size2N;

		FFT(const FFT& _FFT) {
			set_size(_FFT.size);
		}

		uint32_t bitreverse (uint32_t x, uint8_t _size) {
			x = ((x & 0x55555555) << 1) | ((x >> 1) & 0x55555555);
			x = ((x & 0x33333333) << 2) | ((x >> 2) & 0x33333333);
			x = ((x & 0x0F0F0F0F) << 4) | ((x >> 4) & 0x0F0F0F0F);
			x = (x <<24) | ((x & 0xFF00) <<8) | ((x >> 8) & 0xFF00) | (x >> 24);
			x >>= (uint8_t)32 - _size;
			return x;
		}

	protected:


		std::vector<std::complex<T>> dataMass;
		std::vector<std::complex<T>> freqMass;

	public:

		FFT(uint32_t _size) { set_size(_size); }
		virtual ~FFT () {}

		void set_size (uint32_t _size) {
			if (size == _size) return;
			size = _size;
			uint32_t sizeTmp (size);
			size2N = 1;
			while ((sizeTmp /= 2) != 1) {
				assert (size % sizeTmp == 0 && size != 0);
				++size2N;
			}

			dataMass.resize(size);
			dataMassTmp.resize(size);
			freqMass.resize(size);
			expMassPlus.resize(size/2);
			expMassMinus.resize(size/2);
			bitReversMass.resize(size);

			for (uint32_t i = 0; i < size/2; i++) {
				expMassPlus[i] = exp(std::complex<float> (0.0, 2.0f * PI * static_cast<float> (i) / static_cast<float> (size)));
				expMassMinus[i] = exp(std::complex<float> (0.0, -2.0f * PI * static_cast<float> (i) / static_cast<float> (size)));
			}
			for (uint32_t i = 0; i < size; i++) {
				dataMass[i] = std::complex<float> (0.f, 0.f);
				dataMassTmp[i] = std::complex<float> (0.f, 0.f);
				freqMass[i] = std::complex<float> (1.f, 0.f);
				bitReversMass[i] = bitreverse(i, size2N);
			}
		}

		uint32_t data_size () const { return size; }
		uint32_t fourier_size () const { return size/2; }

		void set_time_data (const std::vector<T>& inp) {
			assert(inp.size() == dataMass.size());
			for (uint32_t i = 0; i < size; ++i) {
				dataMass[i] = inp[i];
			}
		}

		void set_time_data (const std::vector<std::complex<T>>& inp) {
			assert(inp.size() == dataMass.size());
			dataMass = inp;
		}

		void set_freq_data (const std::vector<std::complex<T>>& freq) {
			assert(freq.size() == freqMass.size());
			freqMass = freq;
		}

		void filtering (const std::vector<std::complex<T>>& filter) {
			if (filter.size() == size) {
				for (uint32_t i = 0, ie = size; i < ie; i++) freqMass[i] *= filter[i];
			} else if (filter.size() == size/2) {
				for (uint32_t i = 0, ie = size/2; i < ie; i++) {
					freqMass[i] *= filter[i];
					freqMass[size - 1 - i] *= filter[i];
				}
			} else assert(false);
		}

		void filtering (const std::vector<T>& filter) {
			if (filter.size() == size) {
				for (uint32_t i = 0, ie = size; i < ie; i++) freqMass[i] *= filter[i];
			} else if (filter.size() == size/2) {
				for (uint32_t i = 0, ie = size/2; i < ie; i++) {
					freqMass[i] *= filter[i];
					freqMass[size - 1 - i] *= filter[i];
				}
			} else assert(false);
		}

		const std::vector<std::complex<T>>& get_time_data () const { return dataMass; }
		const std::vector<std::complex<T>>& get_freq_data () const { return freqMass; }

		void go (bool inverse = false) {
			if (inverse) {
				T szf = static_cast<T> (size);
				for (uint32_t i = 0; i < size; ++i) dataMass[i] = (freqMass[bitReversMass[i]]/szf);
				for (uint32_t stage = 1; stage != size; stage *= 2) {
					for (uint32_t j = 0, n = size/(2*stage); j < n; ++j) {
						uint32_t t1 = (j * 2) * stage;
						uint32_t t2 = t1 + stage;
						for (uint32_t i = 0; i < stage; ++i) {
							dataMass[i + t2] *= expMassMinus[i * n];
						}
						for (uint32_t i = 0; i < stage; ++i) {
							uint32_t t3 = i + t1;
							uint32_t t4 = i + t2;
							dataMassTmp[t3] = dataMass[t3] + dataMass[t4];
							dataMassTmp[t4] = dataMass[t3] - dataMass[t4];
						}
					}
					std::swap (dataMass, dataMassTmp);
				}
			} else {
				for (uint32_t i = 0; i < size; ++i) freqMass[i] = (dataMass[bitReversMass[i]]);
				for (uint32_t stage = 1; stage != size; stage *= 2) {
					for (uint32_t j = 0, n = size/(2*stage); j < n; ++j) {
						uint32_t t1 = (j * 2) * stage;
						uint32_t t2 = t1 + stage;
						for (uint32_t i = 0; i < stage; ++i) {
							freqMass[i + t2] *= expMassPlus[i * n];
						}
						for (uint32_t i = 0; i < stage; ++i) {
							uint32_t t3 = i + t1;
							uint32_t t4 = i + t2;
							dataMassTmp[t3] = freqMass[t3] + freqMass[t4];
							dataMassTmp[t4] = freqMass[t3] - freqMass[t4];
						}
					}
					std::swap (freqMass, dataMassTmp);
				}
			}
		}
};

#endif // FTT_HPP
