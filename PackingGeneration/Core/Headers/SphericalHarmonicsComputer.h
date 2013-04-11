// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Core_Headers_SphericalHarmonicsComputer_h
#define Core_Headers_SphericalHarmonicsComputer_h

#include <vector>
#include <complex>
#include "Types.h"
#include "Exceptions.h"
#include "Constants.h"

namespace Core
{
    class SphericalHarmonicsComputer
    {
    public:
        static void FillSphericalHarmonicValues(const SpatialVector& direction, int l, std::vector<std::complex<FLOAT_TYPE> >* harmonicValues)
        {
            FLOAT_TYPE theta = acos(direction[2]);
            FLOAT_TYPE phi = atan2(direction[1], direction[0]);

            FillSphericalHarmonicValues(theta, phi, l, harmonicValues);
        }

        static void FillSphericalHarmonicValues(FLOAT_TYPE theta, FLOAT_TYPE phi, int l, std::vector<std::complex<FLOAT_TYPE> >* harmonicValues)
        {
            harmonicValues->clear();
            for (int m = -l; m <= l; ++m)
            {
                std::complex<FLOAT_TYPE> harmonicValue = GetAnalyticalSphericalHarmonicValue(theta, phi, l, m);
                harmonicValues->push_back(harmonicValue);
            }
        }

        static std::complex<FLOAT_TYPE> GetAnalyticalSphericalHarmonicValue(FLOAT_TYPE theta, FLOAT_TYPE phi, int l, int m)
        {
            // We compute the entire function in double precision and then convert the result to FLOAT_TYPE. It's done, as complex<T> defines operation * as: T * complex<T>.
            // If a compiler encounters double * complex<float>, it reports error.
            // We have to convert all multipliers to FLOAT_TYPE (e.g. static_cast<FLOAT_TYPE>(1.0 / 2.0 * sqrt(1.0 / PI)) * complex<FLOAT_TYPE>(1.0, 0.0)), but that is weird.
            // So a slower, but a cleaner solution is preferred (others may also be used, e.g. template specification).

            std::complex<double> value = GetAnalyticalSphericalHarmonicValueDouble(theta, phi, l, m);

#ifdef SINGLE_PRECISION
            return std::complex<FLOAT_TYPE>(value.real(), value.imag());
#else
            return value;
#endif
        }

    private:
        static std::complex<double> GetAnalyticalSphericalHarmonicValueDouble(double theta, double phi, int l, int m)
        {
            std::complex<double> i = std::complex<double>(0, 1);

            if (l == 0)
            {
                return 1.0 / 2.0 * sqrt(1.0 / PI) * std::complex<double>(1.0, 0.0);
            }

            if (l == 1)
            {
                if (m == -1) { return 1.0 / 2.0 * sqrt(3.0 / 2.0 / PI) * std::complex<double>(cos(-phi), sin(-phi)) * sin(theta); }

                if (m == 0) { return 1.0 / 2.0 * sqrt(3.0 / PI) * cos(theta); }

                if (m == 1) { return 1.0 / 2.0 * sqrt(3.0 / 2.0 / PI) * std::complex<double>(cos(phi), sin(phi)) * sin(theta); }
            }

            if (l == 6)
            {
                double sinTheta = sin(theta);
                double cosTheta = cos(theta);

                if (m == -6) { return (1.0 / 64.0) * sqrt(3003.0 / PI) * exp(-6.0 * i * phi) * pow(sinTheta, 6); }

                if (m == -5) { return (3.0 / 32.0) * sqrt(1001.0 / PI) * exp(-5.0 * i * phi) * pow(sinTheta, 5) * cosTheta; }

                if (m == -4) { return (3.0 / 32.0) * sqrt(91.0 / 2.0 / PI) * exp(-4.0 * i * phi) * pow(sinTheta, 4) * (11.0 * pow(cosTheta, 2) - 1.0); }

                if (m == -3) { return (1.0 / 32.0) * sqrt(1365.0 / PI) * exp(-3.0 * i * phi) * pow(sinTheta, 3) * (11.0 * pow(cosTheta, 3) - 3.0 * cosTheta); }

                if (m == -2) { return (1.0 / 64.0) * sqrt(1365.0 / PI) * exp(-2.0 * i * phi) * pow(sinTheta, 2) * (33.0 * pow(cosTheta, 4) - 18.0 * pow(cosTheta, 2) + 1.0); }

                if (m == -1) { return (1.0 / 16.0) * sqrt(273.0 / 2.0 / PI) * exp(- i * phi) * sinTheta* (33.0 * pow(cosTheta, 5) - 30.0 * pow(cosTheta, 3) + 5.0 * cosTheta); }

                if (m == 0) { return (1.0 / 32.0) * sqrt(13.0 / PI) * (231.0 * pow(cosTheta, 6) - 315.0 * pow(cosTheta, 4) + 105.0 * pow(cosTheta, 2) - 5.0); }

                if (m == 1) { return (-1.0 / 16.0) * sqrt(273.0 / 2.0 / PI) * exp(i * phi) * sinTheta * (33.0 * pow(cosTheta, 5) - 30.0 * pow(cosTheta, 3) + 5.0 * cosTheta); }

                if (m == 2) { return (1.0 / 64.0) * sqrt(1365.0 / PI) * exp(2.0 * i * phi) * pow(sinTheta, 2) * (33.0 * pow(cosTheta, 4) - 18.0 * pow(cosTheta, 2) + 1.0); }

                if (m == 3) { return (-1.0 / 32.0) * sqrt(1365.0 / PI) * exp(3.0 * i * phi) * pow(sinTheta, 3) * (11.0 * pow(cosTheta, 3) - 3.0 * cosTheta); }

                if (m == 4) { return (3.0 / 32.0) * sqrt(91.0 / 2.0 / PI) * exp(4.0 * i * phi) * pow(sinTheta, 4) * (11.0 * pow(cosTheta, 2) - 1.0); }

                if (m == 5) { return (-3.0 / 32.0) * sqrt(1001.0 / PI) * exp(5.0 * i * phi) * pow(sinTheta, 5) * cosTheta; }

                if (m == 6) { return (1.0 / 64.0) * sqrt(3003.0 / PI) * exp(6.0 * i * phi) * pow(sinTheta, 6); }
            }

            throw InvalidOperationException("Spherical harmonics not supported");
        }
    };
}

#endif /* Core_Headers_SphericalHarmonicsComputer_h */
