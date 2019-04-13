#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <ipps.h>
#define RTSEIS_LOGGING 1
#include "rtseis/utilities/filterImplementations/medianFilter.hpp"
#include "rtseis/log.h"

using namespace RTSeis::Utilities::FilterImplementations;

class MedianFilter::MedianFilterImpl
{
    public:
        /// Default constructor.
        MedianFilterImpl(void)
        {
            return;
        }
        /// Copy constructor.
        MedianFilterImpl(const MedianFilterImpl &median)
        {
            *this = median;
        }
        /// (Deep) Copy operator
        MedianFilterImpl& operator=(const MedianFilterImpl &median)
        {
            if (&median == this){return *this;}
            if (!median.linit_){return *this;}
            // Reinitialize the filter
            int ierr = initialize(median.maskSize_, median.mode_,
                                  median.precision_);
            if (ierr != 0)
            {
                RTSEIS_ERRMSG("%s", "Failed to initialize");
                clear();
                return *this;
            }
            // Now copy the filter states
            if (bufferSize_ > 0)
            {
                ippsCopy_8u(median.pBuf_, pBuf_, bufferSize_);
            }
            if (nwork_ > 0)
            {
                ippsCopy_64f(median.zi_, zi_, nwork_);
                if (median.precision_ == RTSeis::Precision::DOUBLE)
                {
                    ippsCopy_64f(median.dlysrc64_, dlysrc64_, nwork_);
                    ippsCopy_64f(median.dlydst64_, dlydst64_, nwork_);
                }
                else
                {
                    ippsCopy_32f(median.dlysrc32_, dlysrc32_, nwork_);
                    ippsCopy_32f(median.dlydst32_, dlydst32_, nwork_);
                }
            }
            return *this;
        }
        /// Destructor
        ~MedianFilterImpl(void)
        {
            clear();
            return;
        }
        //====================================================================//
        /// Clears the filter/releases the memory
        void clear(void)
        {
            if (dlysrc64_ != nullptr){ippsFree(dlysrc64_);}
            if (dlydst64_ != nullptr){ippsFree(dlydst64_);}
            if (dlysrc32_ != nullptr){ippsFree(dlysrc32_);}
            if (dlydst32_ != nullptr){ippsFree(dlydst32_);}
            if (pBuf_     != nullptr){ippsFree(pBuf_);}
            if (zi_       != nullptr){ippsFree(zi_);}
            dlysrc64_ = nullptr;
            dlydst64_ = nullptr;
            dlysrc32_ = nullptr;
            dlydst32_ = nullptr;
            pBuf_ = nullptr;
            zi_ = nullptr;
            maskSize_ = 0;
            bufferSize_ = 0;
            mode_ = RTSeis::ProcessingMode::POST_PROCESSING;
            precision_ = RTSeis::Precision::DOUBLE;
            linit_ = false;
            return;
        }
        /// Initializes the filter
        int initialize(const int n,
                       const RTSeis::ProcessingMode mode,
                       const RTSeis::Precision precision)
        {
            clear();
            maskSize_ = n; // This better be odd by this point
            // Set the space
            nwork_ = std::max(8, maskSize_ - 1);
            zi_ = ippsMalloc_64f(nwork_);
            ippsZero_64f(zi_, nwork_);
            if (precision == RTSeis::Precision::DOUBLE)
            {
                IppStatus status = ippsFilterMedianGetBufferSize(maskSize_,
                                                                 ipp64f,
                                                                 &bufferSize_);
                if (status != ippStsNoErr)
                {
                    RTSEIS_ERRMSG("%s", "Error getting buffer size");
                    clear();
                    return -1;
                }
                dlysrc64_ = ippsMalloc_64f(nwork_);
                ippsZero_64f(dlysrc64_, nwork_);
                dlydst64_ = ippsMalloc_64f(nwork_);
                ippsZero_64f(dlydst64_, nwork_);
                pBuf_ = ippsMalloc_8u(bufferSize_);
                ippsZero_8u(pBuf_, bufferSize_);
            }
            else
            {
                IppStatus status = ippsFilterMedianGetBufferSize(maskSize_,
                                                                 ipp32f,
                                                                 &bufferSize_);
                if (status != ippStsNoErr)
                {
                    RTSEIS_ERRMSG("%s", "Error getting buffer size");
                    clear();
                    return -1;
                }
                dlysrc32_ = ippsMalloc_32f(nwork_);
                ippsZero_32f(dlysrc32_, nwork_);
                dlydst32_ = ippsMalloc_32f(nwork_);
                ippsZero_32f(dlydst32_, nwork_);
                pBuf_ = ippsMalloc_8u(bufferSize_);
                ippsZero_8u(pBuf_, bufferSize_);
            }
            precision_ = precision;
            mode_ = mode;
            linit_ = true;
            return 0;
        }
        /// Determines if the module is initialized.
        bool isInitialized(void) const
        {
            return linit_;
        }
        /// Determines the length of the initial conditions.
        int getInitialConditionLength(void) const
        {
            int len = maskSize_ - 1;;
            return len;
        }
        /// Gets the group delay.
        int getGroupDelay(void) const
        {
            int grpDelay = maskSize_/2;
            return grpDelay;
        }
        /// Set the initial conditions
        int setInitialConditions(const int nz, const double zi[])
        {
            resetInitialConditions();
            int nzRef = getInitialConditionLength();
            if (nzRef != nz){RTSEIS_WARNMSG("%s", "Shouldn't happen");}
            if (nzRef > 0){ippsCopy_64f(zi, zi_, nzRef);}
            if (precision_ == RTSeis::Precision::DOUBLE)
            {   
                if (nzRef > 0){ippsCopy_64f(zi, dlysrc64_, nzRef);}
            }
            else
            {
                if (nzRef > 0){ippsConvert_64f32f(zi, dlysrc32_, nzRef);}
            }   
            return 0;
        }
        /// Resets the initial conditions
        int resetInitialConditions(void)
        {
            if (precision_ == RTSeis::Precision::DOUBLE)
            {
                if (nwork_ > 0){ippsCopy_64f(zi_, dlysrc64_, nwork_);}
            }
            else
            {
                if (nwork_ > 0){ippsConvert_64f32f(zi_, dlysrc32_, nwork_);}
            }   
            return 0;
        }
        /// Apply the filter
        int apply(const int n, const double x[], double y[])
        {
            if (n <= 0){return 0;} // Nothing to do
            if (precision_ == RTSeis::Precision::FLOAT)
            {
                Ipp32f *x32 = ippsMalloc_32f(n);
                Ipp32f *y32 = ippsMalloc_32f(n);
                ippsConvert_64f32f(x, x32, n);
                int ierr = apply(n, x32, y32);
                ippsFree(x32);
                if (ierr != 0)
                {
                    RTSEIS_ERRMSG("%s", "Failed to apply filter");
                    ippsFree(y32);
                    return -1;
                }
                ippsConvert_32f64f(y32, y, n);
                ippsFree(y32);
                return 0;
            }
            IppStatus status;
            if (mode_ == RTSeis::ProcessingMode::REAL_TIME)
            {
                status = ippsFilterMedian_64f(x, y, n, maskSize_,
                                              dlysrc64_, dlydst64_, pBuf_);
                if (status != ippStsNoErr)
                {
                    RTSEIS_ERRMSG("%s", "Error applying real-time filter");
                    return -1;
                }
                if (maskSize_ > 1)
                {
                    ippsCopy_64f(dlydst64_, dlysrc64_, maskSize_-1);
                }
            }
            else
            {
                status = ippsFilterMedian_64f(x, y, n, maskSize_,
                                              dlysrc64_, NULL, pBuf_);
                if (status != ippStsNoErr)
                {
                    RTSEIS_ERRMSG("%s", "Error applying real-time filter");
                    return -1;
                }
            }
            return 0;
        }
        /// Apply the filter
        int apply(const int n, const float x[], float y[])
        {
            if (n <= 0){return 0;} // Nothing to do
            if (precision_ == RTSeis::Precision::DOUBLE)
            {
                Ipp64f *x64 = ippsMalloc_64f(n);
                Ipp64f *y64 = ippsMalloc_64f(n);
                ippsConvert_32f64f(x, x64, n);
                int ierr = apply(n, x64, y64);
                ippsFree(x64);
                if (ierr != 0)
                {
                    RTSEIS_ERRMSG("%s", "Failed to apply filter");
                    ippsFree(y64);
                    return -1;
                }
                ippsConvert_64f32f(y64, y, n);
                ippsFree(y64);
                return 0;
            }
            IppStatus status;
            if (mode_ == RTSeis::ProcessingMode::REAL_TIME)
            {
                status = ippsFilterMedian_32f(x, y, n, maskSize_,
                                              dlysrc32_, dlydst32_, pBuf_);
                if (status != ippStsNoErr)
                {
                    RTSEIS_ERRMSG("%s", "Error applying real-time filter");
                    return -1;
                }
                if (maskSize_ > 1)
                {
                    ippsCopy_32f(dlydst32_, dlysrc32_, maskSize_-1);
                }
            }
            else
            {
                status = ippsFilterMedian_32f(x, y, n, maskSize_,
                                              dlysrc32_, NULL, pBuf_);
                if (status != ippStsNoErr)
                {
                    RTSEIS_ERRMSG("%s", "Error applying real-time filter");
                    return -1;
                }
            }
            return 0;
        }
    private:
        /// Delay line source vector.  This has dimension [nwork_].
        Ipp64f *dlysrc64_ = nullptr;
        /// Delay line destination vector.  This has dimension [nwork_].
        Ipp64f *dlydst64_ = nullptr;
        /// Delay line source vector.  This has dimension [nwork_].
        Ipp32f *dlysrc32_ = nullptr;
        /// Delay line destination vector.  This has dimension [nwork_].
        Ipp32f *dlydst32_ = nullptr;
        /// Workspace for median filter.  This has dimension [bufferSize_].
        Ipp8u *pBuf_ = nullptr;
        /// A reference of the saved initial conditions.  This has 
        /// dimension [nwork_] though only the first maskSize_  - 1
        /// points are valid.
        Ipp64f *zi_ = nullptr;
        /// The median filter window length.
        int maskSize_ = 0;
        /// The workspace for the delay lines.
        int nwork_ = 0;
        /// The size of the workspace buffer.
        int bufferSize_ = 0;
        /// By default the module does post-procesing.
        RTSeis::ProcessingMode mode_ = RTSeis::ProcessingMode::POST_PROCESSING;
        /// The default module implementation.
        RTSeis::Precision precision_ = RTSeis::Precision::DOUBLE;
        /// Flag indicating the module is initialized.
        bool linit_ = false;

};

MedianFilter::MedianFilter(void) :
    pMedian_(new MedianFilterImpl())
{
    return;
}

MedianFilter::MedianFilter(const MedianFilter &median)
{
    *this = median;
    return;
}

MedianFilter& MedianFilter::operator=(const MedianFilter &median)
{
    if (&median == this){return *this;}
    if (pMedian_){pMedian_->clear();}
    pMedian_ = std::unique_ptr<MedianFilterImpl>
               (new MedianFilterImpl(*median.pMedian_));
    return *this;
}

MedianFilter::~MedianFilter(void)
{
    clear();
    return;
}

void MedianFilter::clear(void)
{
    pMedian_->clear();
    return;
}

int MedianFilter::initialize(const int n,
                             const RTSeis::ProcessingMode mode,
                             const RTSeis::Precision precision)
{
    clear();
    // Set the mask size
    if (n < 1)
    {
        RTSEIS_ERRMSG("Mask size=%d must be postive", n);
        return -1;
    }
    int maskSize = n;
    if (maskSize%2 == 0)
    {
        maskSize = maskSize + 1;
        RTSEIS_WARNMSG("n=%d should be odd; setting to maskSize=%d",
                       n, maskSize);
    }
    int ierr = pMedian_->initialize(maskSize, mode, precision);
    if (ierr != 0)
    {
        RTSEIS_ERRMSG("%s", "Failed to initialized filter");
        clear();
        return -1;
    }
    return 0;
}

int MedianFilter::setInitialConditions(const int nz, const double zi[])
{
    if (!pMedian_->isInitialized())
    {
        RTSEIS_ERRMSG("%s", "Class not initialized");
        return -1;
    }
    int nzRef = pMedian_->getInitialConditionLength();
    if (nz != nzRef || zi == nullptr)
    {
        if (nz != nzRef){RTSEIS_ERRMSG("nz=%d should equal %d", nz, nzRef);}
        if (zi == nullptr){RTSEIS_ERRMSG("%s", "zi is NULL");}
        return -1;
    }
    pMedian_->setInitialConditions(nz, zi);
    return 0;
}

int MedianFilter::resetInitialConditions(void)
{
    if (!pMedian_->isInitialized())
    {
        RTSEIS_ERRMSG("%s", "Class not initialized");
        return -1;
    }
    pMedian_->resetInitialConditions();
    return 0;
}

bool MedianFilter::isInitialized(void) const
{
    bool linit = pMedian_->isInitialized();
    return linit;
}

int MedianFilter::getInitialConditionLength(void) const
{
    if (!pMedian_->isInitialized())
    {
        RTSEIS_ERRMSG("%s", "Class not initialized");
        return -1;
    }
    int len = pMedian_->getInitialConditionLength();
    return len;
}

int MedianFilter::apply(const int n, const double x[], double y[])
{
    if (n <= 0){return 0;} // Nothing to do
    if (!pMedian_->isInitialized())
    {   
        RTSEIS_ERRMSG("%s", "Class not initialized");
        return -1; 
    }
    if (x == nullptr || y == nullptr)
    {
        RTSEIS_ERRMSG("%s", "x is NULL");
        RTSEIS_ERRMSG("%s", "y is NULL");
    }
    int ierr = pMedian_->apply(n, x, y);
    if (ierr != 0)
    {
        RTSEIS_ERRMSG("%s", "Failed to apply filter");
        return -1;
    }
    return 0;
}

int MedianFilter::apply(const int n, const float x[], float y[])
{
    if (n <= 0){return 0;} // Nothing to do
    if (!pMedian_->isInitialized())
    {
        RTSEIS_ERRMSG("%s", "Class not initialized");
        return -1; 
    }
    if (x == nullptr || y == nullptr)
    {
        RTSEIS_ERRMSG("%s", "x is NULL");
        RTSEIS_ERRMSG("%s", "y is NULL");
    }
    int ierr = pMedian_->apply(n, x, y);
    if (ierr != 0)
    {
        RTSEIS_ERRMSG("%s", "Failed to apply filter");
        return -1;
    }
    return 0;
}

int MedianFilter::getGroupDelay(void) const
{
    int grpDelay = pMedian_->getGroupDelay();
    return grpDelay;
}