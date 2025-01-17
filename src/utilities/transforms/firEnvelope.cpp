#include <cstdio>
#include <cstdlib>
#include <array>
#include <ipps.h>
#include "rtseis/utilities/transforms/firEnvelope.hpp"
#include "rtseis/utilities/filterDesign/fir.hpp"
#include "rtseis/utilities/filterRepresentations/fir.hpp"
#include "rtseis/utilities/filterImplementations/firFilter.hpp"
#include "rtseis/private/throw.hpp"

using namespace RTSeis::Utilities;
using namespace RTSeis::Utilities::Transforms;

template<class T>
class FIREnvelope<T>::FIREnvelopeImpl
{
public:
    FIREnvelopeImpl() = default;
    ~FIREnvelopeImpl() = default;
    FIREnvelopeImpl(const FIREnvelopeImpl &firEnvelope)
    {
        *this = firEnvelope;
    }
    FIREnvelopeImpl& operator=(const FIREnvelopeImpl &firEnvelope)
    {
        if (&firEnvelope == this){return *this;}
        mRealFIRFilter = firEnvelope.mRealFIRFilter;
        mImagFIRFilter = firEnvelope.mImagFIRFilter;
        mMean = firEnvelope.mMean;
        mNumberOfTaps = firEnvelope.mNumberOfTaps;
        mZeroPhase = firEnvelope.mZeroPhase;
        mType3 = firEnvelope.mType3;
        mHaveInitialCondition = firEnvelope.mHaveInitialCondition;
        mInitialized = firEnvelope.mInitialized;
        mMode = firEnvelope.mMode;
        return *this; 
    }

    FilterImplementations::FIRFilter<T> mRealFIRFilter;
    FilterImplementations::FIRFilter<T> mImagFIRFilter;
    double mMean = 0;
    int mNumberOfTaps = 0;
    bool mZeroPhase = true;
    bool mType3 = false;
    bool mHaveInitialCondition = false;
    bool mInitialized = false;
    ProcessingMode mMode = ProcessingMode::POST_PROCESSING;
};

/// Constructor
template<class T>
FIREnvelope<T>::FIREnvelope() :
    pImpl(std::make_unique<FIREnvelopeImpl> ())
{
}

/// Copy constructor
template<class T>
FIREnvelope<T>::FIREnvelope(const FIREnvelope &firEnvelope)
{
    *this = firEnvelope;
}

/// Move constructor
template<class T>
FIREnvelope<T>::FIREnvelope(FIREnvelope &&firEnvelope) noexcept
{
    *this = std::move(firEnvelope);
}

/// Copy assignment
template<class T>
FIREnvelope<T>& FIREnvelope<T>::operator=(const FIREnvelope &firEnvelope)
{
    if (&firEnvelope == this){return *this;}
    pImpl = std::make_unique<FIREnvelopeImpl> (*firEnvelope.pImpl);
    return *this; 
}

/// Move assignment
template<class T>
FIREnvelope<T>& FIREnvelope<T>::operator=(FIREnvelope &&firEnvelope) noexcept
{
    if (&firEnvelope == this){return *this;}
    pImpl = std::move(firEnvelope.pImpl);
    return *this;
}

/// Destructor
template<class T>
FIREnvelope<T>::~FIREnvelope() = default;

/// Clear the filter
template<class T>
void FIREnvelope<T>::clear() noexcept
{
    pImpl->mRealFIRFilter.clear();
    pImpl->mImagFIRFilter.clear();
    pImpl->mMean = 0;
    pImpl->mNumberOfTaps = 0;
    pImpl->mZeroPhase = true;
    pImpl->mType3 = false;
    pImpl->mHaveInitialCondition = false;
    pImpl->mInitialized = false;
    pImpl->mMode = ProcessingMode::POST_PROCESSING;
}

/// Check if initialized
template<class T>
bool FIREnvelope<T>::isInitialized() const noexcept
{
    return pImpl->mInitialized;
}

/// Initialize
template<class T>
void FIREnvelope<T>::initialize(const int ntaps,
                                const RTSeis::ProcessingMode mode)
{
    clear();
    if (ntaps < 1)
    {
        RTSEIS_THROW_IA("ntaps = %d must be positive", ntaps);
    }
    pImpl->mMode = mode;
    pImpl->mType3 = false;
    if (ntaps%2 == 1){pImpl->mType3 = true;}
    pImpl->mNumberOfTaps = ntaps;
    std::pair<FilterRepresentations::FIR, FilterRepresentations::FIR> zfir;
    constexpr double beta = 8;
    constexpr auto direct = FilterImplementations::FIRImplementation::DIRECT;
    // Create an FIR hilbert transform
    try
    {
        auto zfir = FilterDesign::FIR::HilbertTransformer(ntaps - 1, beta);
        auto rfir = zfir.first.getFilterTaps();
        pImpl->mRealFIRFilter.initialize(rfir.size(), rfir.data(),
                                         mode, direct);
        auto cfir = zfir.second.getFilterTaps();
        pImpl->mImagFIRFilter.initialize(cfir.size(), cfir.data(),
                                         mode, direct);
    }
    catch (const std::exception &e) 
    {
        clear();
        RTSEIS_THROW_RTE("%s; Failed to initialize Hilbert transformer",
                         e.what());
    }
    pImpl->mInitialized = true;
}

/// Get the initial condition length
template<class T>
int FIREnvelope<T>::getInitialConditionLength() const
{
    if (!isInitialized())
    {
        RTSEIS_THROW_RTE("%s", "Envelope class not initialized");
    }
    return pImpl->mImagFIRFilter.getInitialConditionLength();
}

/// Sets the initial conditions
template<class T>
void FIREnvelope<T>::setInitialConditions(const int nz, const double zi[])
{
    if (!isInitialized())
    {
        RTSEIS_THROW_RTE("%s", "Envelope class not initialized");
    }
    int nzRef = getInitialConditionLength();
    if (nz != nzRef)
    {
        RTSEIS_THROW_IA("nz = %d must equal %d", nz, nzRef);
    }
    pImpl->mRealFIRFilter.setInitialConditions(nz, zi);
    pImpl->mImagFIRFilter.setInitialConditions(nz, zi);
    pImpl->mHaveInitialCondition = true;
}

/// Resets the initial conditions
template<class T>
void FIREnvelope<T>::resetInitialConditions()
{
    if (!isInitialized())
    {
        RTSEIS_THROW_RTE("%s", "Envelope class not initialized");
    }
    pImpl->mRealFIRFilter.resetInitialConditions();
    pImpl->mImagFIRFilter.resetInitialConditions();
}

/// Perform transform
template<>
void FIREnvelope<double>::transform(const int n, const double x[], double *yIn[])
{
    pImpl->mMean = 0;
    if (n < 1){return;} // Nothing to do
    if (!isInitialized())
    {
        RTSEIS_THROW_RTE("%s", "Failed to initialize");
    }
    double *y = *yIn;
    if (x == nullptr || y == nullptr)
    {
        if (x == nullptr){RTSEIS_THROW_IA("%s", "x is NULL");}
        RTSEIS_THROW_IA("%s", "y is NULL");
    }
    // Post-processing removes the phase shift
    if (pImpl->mMode == RTSeis::ProcessingMode::POST_PROCESSING)
    {
        // Compute the mean
        double pMean;
        ippsMean_64f(x, n, &pMean);
        pImpl->mMean = pMean;
        // Remove the mean and pad out the signal
        // N.B. The group delay is actually + 1 but C wants to shift relative to
        // a base address so we subtract the one.  Hence, n/2 instead of n/2+1.
        int groupDelay = pImpl->mNumberOfTaps/2;
        int npad = n + groupDelay;
        double *xPad = ippsMalloc_64f(npad);
        ippsSubC_64f(x, pMean, xPad, n);
        ippsZero_64f(&xPad[n], groupDelay); // Post-pad with zeros
        // Now apply the filter and compute absolute value - Type III
        if (pImpl->mType3)
        {
            double *yPadr = xPad;
            double *yPadi = ippsMalloc_64f(npad);
            pImpl->mImagFIRFilter.apply(npad, xPad, &yPadi); 
            ippsMagnitude_64f(yPadr, &yPadi[groupDelay], y, n);
            ippsFree(yPadi);
        }
        else
        {
            double *yPadr = ippsMalloc_64f(npad);
            pImpl->mRealFIRFilter.apply(npad, xPad, &yPadr);
            double *yPadi = ippsMalloc_64f(npad);
            pImpl->mImagFIRFilter.apply(npad, xPad, &yPadi);
            ippsMagnitude_64f(&yPadr[groupDelay], &yPadi[groupDelay], y, n);
            ippsFree(yPadr);
            ippsFree(yPadi);
        }
        ippsFree(xPad);
        // Reconstitute the mean
        ippsAddC_64f_I(pMean, y, n);
    }
    else
    {
        // TODO - add a sparse FIR filter for the type III case
        constexpr int chunkSize = 1024;
        std::array<double, chunkSize> yrTemp;
        std::array<double, chunkSize> yiTemp;
        for (auto ic=0; ic<n; ic=ic+chunkSize)
        {
            auto npfilt = std::min(n - ic, chunkSize);
            auto yrTempDataPtr = yrTemp.data();
            auto yiTempDataPtr = yiTemp.data();
            pImpl->mRealFIRFilter.apply(npfilt, &x[ic], &yrTempDataPtr); //yrTemp.data());
            pImpl->mImagFIRFilter.apply(npfilt, &x[ic], &yiTempDataPtr); //yiTemp.data());
            ippsMagnitude_64f(yrTemp.data(), yiTemp.data(), &y[ic], npfilt);
        }
    }
}

template<>
void FIREnvelope<float>::transform(const int n, const float x[], float *yIn[])
{
    pImpl->mMean = 0;
    if (n < 1){return;} // Nothing to do
    if (!isInitialized())
    {
        RTSEIS_THROW_RTE("%s", "Failed to initialize");
    }
    float *y = *yIn;
    if (x == nullptr || y == nullptr)
    {
        if (x == nullptr){RTSEIS_THROW_IA("%s", "x is NULL");}
        RTSEIS_THROW_IA("%s", "y is NULL");
    }
    // Post-processing removes the phase shift
    if (pImpl->mMode == RTSeis::ProcessingMode::POST_PROCESSING)
    {
        // Compute the mean
        float pMean;
        ippsMean_32f(x, n, &pMean, ippAlgHintAccurate);
        pImpl->mMean = pMean;
        // Remove the mean and pad out the signal
        // N.B. The group delay is actually + 1 but C wants to shift relative to
        // a base address so we subtract the one.  Hence, n/2 instead of n/2+1.
        int groupDelay = pImpl->mNumberOfTaps/2;
        int npad = n + groupDelay;
        float *xPad = ippsMalloc_32f(npad);
        ippsSubC_32f(x, pMean, xPad, n);
        ippsZero_32f(&xPad[n], groupDelay); // Post-pad with zeros
        // Now apply the filter and compute absolute value - Type III
        if (pImpl->mType3)
        {
            float *yPadr = xPad;
            float *yPadi = ippsMalloc_32f(npad);
            pImpl->mImagFIRFilter.apply(npad, xPad, &yPadi);
            ippsMagnitude_32f(yPadr, &yPadi[groupDelay], y, n);
            ippsFree(yPadi);
        }
        else
        {
            float *yPadr = ippsMalloc_32f(npad);
            pImpl->mRealFIRFilter.apply(npad, xPad, &yPadr);
            float *yPadi = ippsMalloc_32f(npad);
            pImpl->mImagFIRFilter.apply(npad, xPad, &yPadi);
            ippsMagnitude_32f(&yPadr[groupDelay], &yPadi[groupDelay], y, n);
            ippsFree(yPadr);
            ippsFree(yPadi);
        }
        ippsFree(xPad);
        // Reconstitute the mean
        ippsAddC_32f_I(pMean, y, n);
    }
    else
    {
        // TODO - add a sparse FIR filter for the type III case
        constexpr int chunkSize = 1024;
        std::array<float, chunkSize> yrTemp;
        std::array<float, chunkSize> yiTemp;
        for (auto ic=0; ic<n; ic=ic+chunkSize)
        {
            auto npfilt = std::min(n - ic, chunkSize);
            auto yrTempDataPtr = yrTemp.data();
            auto yiTempDataPtr = yiTemp.data();
            pImpl->mRealFIRFilter.apply(npfilt, &x[ic], &yrTempDataPtr); //yrTemp.data());
            pImpl->mImagFIRFilter.apply(npfilt, &x[ic], &yiTempDataPtr); //yiTemp.data());
            ippsMagnitude_32f(yrTemp.data(), yiTemp.data(), &y[ic], npfilt);
        }
    }
}

/// Template instantiation
template class RTSeis::Utilities::Transforms::FIREnvelope<double>;
template class RTSeis::Utilities::Transforms::FIREnvelope<float>;
