#include <cmath>
#include <algorithm>
#include "rtseis/utilities/filterRepresentations/sos.hpp"
#define RTSEIS_LOGGING 1
#include "rtseis/log.h"

using namespace RTSeis::Utilities::FilterRepresentations;

#define DEFAULT_TOL 1.e-12

class SOS::SOSImpl
{
public:
    /// Numerator coefficients
    std::vector<double> bs;
    /// Denominator coefficients
    std::vector<double> as;
    /// Number of sections
    int ns = 0;
    /// Equality tolerance
    double tol = DEFAULT_TOL;
};

SOS::SOS(void) :
    pImpl(std::make_unique<SOSImpl>())
{
    return;
}

SOS::SOS(const int ns,
         const std::vector<double> &bs,
         const std::vector<double> &as) :
    pImpl(std::make_unique<SOSImpl>())
{
    setSecondOrderSections(ns, bs, as);
    return;
}

SOS& SOS::operator=(const SOS &sos)
{
    if (&sos == this){return *this;}
    pImpl = std::make_unique<SOSImpl> ();
    //pImpl = std::unique_ptr<SOSImpl> (new SOSImpl());
    pImpl->bs  = sos.pImpl->bs;
    pImpl->as  = sos.pImpl->as;
    pImpl->ns  = sos.pImpl->ns;
    pImpl->tol = sos.pImpl->tol;
    return *this;
}

SOS& SOS::operator=(SOS &&sos)
{
    if (&sos == this){return *this;}
    pImpl = std::move(sos.pImpl);
    return *this;
}

bool SOS::operator==(const SOS &sos) const
{
    if (pImpl->bs.size() != sos.pImpl->bs.size()){return false;}
    if (pImpl->as.size() != sos.pImpl->as.size()){return false;}
    if (pImpl->ns != sos.pImpl->ns){return false;}
    for (size_t i=0; i<pImpl->bs.size(); i++)
    {
        if (std::abs(pImpl->bs[i] - sos.pImpl->bs[i]) > pImpl->tol)
        {
            return false;
        }
    }
    for (size_t i=0; i<pImpl->as.size(); i++)
    {
        if (std::abs(pImpl->as[i] - sos.pImpl->as[i]) > pImpl->tol)
        {
            return false;
        }
    }
    return true;
}

bool SOS::operator!=(const SOS &sos) const
{
    return !(*this == sos);
}

SOS::SOS(const SOS &sos)
{
   *this = sos;
   return;
}

SOS::SOS(SOS &&sos)
{
    *this = std::move(sos);
    return;
}

SOS::~SOS(void) = default;

void SOS::clear(void)
{
    pImpl->bs.clear();
    pImpl->as.clear();
    pImpl->ns = 0;
    pImpl->tol = DEFAULT_TOL;
    return;
}

void SOS::print(FILE *fout) const noexcept
{
    FILE *f = stdout;
    if (fout != nullptr){f = fout;}
    fprintf(f, "Numerator sections\n");
    for (int i=0; i<pImpl->ns; i++)
    {
        fprintf(f, "%+.16lf, %+.16lf, %+.16lf\n",
                pImpl->bs[3*i], pImpl->bs[3*i+1], pImpl->bs[3*i+2]);
    }
    fprintf(f, "Denominator sections\n");
    for (int i=0; i<pImpl->ns; i++)
    {
        fprintf(f, "%+.16lf, %+.16lf, %+.16lf\n",
                pImpl->as[3*i], pImpl->as[3*i+1], pImpl->as[3*i+2]);
    }
    return;
}

void SOS::setSecondOrderSections(const int ns,
                                 const std::vector<double> &bs,
                                 const std::vector<double> &as)
{
    clear();
    if (ns < 1)
    {
        RTSEIS_ERRMSG("%s", "No sections in SOS filter");
        throw std::invalid_argument("No sections in SOS filter");
    }
    size_t ns3 = static_cast<size_t> (ns)*3;
    if (ns3 != bs.size())
    {
        RTSEIS_ERRMSG("bs.size() = %ld must equal 3*ns=%ld", bs.size(), ns3);
        throw std::invalid_argument("ba.size() = " + std::to_string(bs.size())
                                 + " must equal 3*ns = " + std::to_string(ns3));
    }
    if (ns3 != as.size()) 
    {
        RTSEIS_ERRMSG("as.size() = %ld must equal 3*ns=%ld", as.size(), ns3);
        throw std::invalid_argument("as.size() = " + std::to_string(as.size())
                                 + " must equal 3*ns = " + std::to_string(ns3));
    }
    for (int i=0; i<ns; i++)
    {
        if (bs[3*i] == 0)
        {
            RTSEIS_ERRMSG("Leading bs coefficient of section %d is zero", i);
            throw std::invalid_argument("Leading bs coefficient of section "
                                     + std::to_string(i+1) + "is zero");
        }
    }
    for (int i=0; i<ns; i++)
    {
        if (as[3*i] == 0)
        {
            RTSEIS_ERRMSG("Leading bs coefficient of section %d is zero", i);
            throw std::invalid_argument("Leading as coefficient of section "
                                     + std::to_string(i+1) + "is zero");
        }
    }
    // It all checks out
    pImpl->ns = ns;
    pImpl->bs = bs;
    pImpl->as = as;
    return;
}

std::vector<double> SOS::getNumeratorCoefficients(void) const noexcept
{
    return pImpl->bs;
}

std::vector<double> SOS::getDenominatorCoefficients(void) const noexcept
{
    return pImpl->as;
}

int SOS::getNumberOfSections(void) const noexcept
{
    return pImpl->ns;
}

void SOS::setEqualityTolerance(const double tol)
{
    if (tol < 0){RTSEIS_WARNMSG("%s", "Tolerance is negative");}
    pImpl->tol = tol;
    return;
}
