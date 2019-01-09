#ifndef RTSEIS_UTILS_DESIGN_HPP
#define RTSEIS_UTILS_DESIGN_HPP 1
#include "rtseis/config.h"
#include "rtseis/utils/polynomial.h"
#include "rtseis/utils/zpk.h"
#include "rtseis/utils/ba.h"
#include <stdio.h>
#include <complex>
#include <vector>

#ifdef __cplusplus
class AnalogPrototype
{
    public:
        AnalogPrototype(void);
        ~AnalogPrototype(void);
        int cheb1ap(const int n, const double rp);
        int cheb2ap(const int n, const double rs);
        int butter(const int n);
        int bessel(const int n);
        ZPK getTransferFunction(void) const{return zpk_;};
    private:
        ZPK zpk_;
};

class Design : public AnalogPrototype
{
    public:
        Design(void);
        ~Design(void);
        int freqs(const BA ba, const std::vector<double> w,
                  std::vector<std::complex<double>> &h);
        int freqz(const BA ba, const std::vector<double> w,
                  std::vector<std::complex<double>> &h);
        
};
#endif

#endif
