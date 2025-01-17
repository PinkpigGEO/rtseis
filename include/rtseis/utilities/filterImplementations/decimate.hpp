#ifndef RTSEIS_UTILITIES_FILTERIMPLEMENTATIONS_DECIMATE_HPP
#define RTSEIS_UTILITIES_FILTERIMPLEMENTATIONS_DECIMATE_HPP
#include <memory>
#include "rtseis/enums.h"

namespace RTSeis::Utilities::FilterImplementations
{
/*!
 * @class Decimate decimate.hpp "include/rtseis/utilities/filterImplementations/decimate.hpp"
 * @brief Lowpass filters then downsamples a signal.
 * @copyright Ben Baker distributed under the MIT license.
 * @ingroup rtseis_utils_filters
 */
template<class T = double>
class Decimate
{
public:
    /*! @name Constructors
     * @{
     */
    /*!
     * @brief Constructor.
     */
    Decimate();
    /*!
     * @brief Copy constructor.
     * @param[in] decimate  The decimation class from which to initialize
     *                      this class.
     */
    Decimate(const Decimate &decimate); 
    /*! @} */

    /*! @name Operators
     * @{
     */
    /*!
     * @brief Copy assignment operator.
     * @param[in] decimate  The decimate class to copy.
     * @result A deep copy of the decimate class.
     */
    Decimate& operator=(const Decimate &decimate);
    /*! @} */

    /*! @name Destructors
     * @{
     */
    /*!
     * @brief Destructor
     */
    ~Decimate();
    /*!
     * @brief Resets the class.
     */
    void clear() noexcept;
    /*! @} */

    /*!
     * @brief Initializes the decimator.
     * @param[in] downFactor         The downsampling factor.  This will retain
     *                               every (downFactor-1)'th sample.  This must
     *                               be at least 2.
     * @param[in] filterLength       The length of the FIR filter.  This must
     *                               be at least 5.
     * @param[in] lremovePhaseShift  If true then this will remove the phase
     *                               shift introduced by the FIR filter.
     *                               This is relevant when the operation mode
     *                               is for post-processing.
     * @param[in] mode               The processing mode.
     * @throws std::invalid_argument if the downFactor is not positive, the
     *         filter length is too small.
     * @note This will design a Hamming window-based filter whose cutoff
     *       frequency is 1/downFactor.  Additionally, when post-processing
     *       and removing the phase shift, the algorithm will increase
     *       the filter length so that it's group delay + 1 is evenly
     *       divisible by the downsampling factor.
     */
    void initialize(const int downFactor,
                    const int filterLength = 30,
                    const bool lremovePhaseShift = true,
                    const RTSeis::ProcessingMode mode = RTSeis::ProcessingMode::POST_PROCESSING);
    /*!
     * @brief Determines if the class is initialized.
     * @result True indicates that the class is initialized.
     */
    bool isInitialized() const noexcept;
    /*!
     * @brief Gets the length of the initial condition array.
     * @result The length of the initial condition array.
     * @throws std::runtime_error if the class is not initialized.
     */
    int getInitialConditionLength() const;
    /*!
     * @brief Sets the initial conditions array.
     * @param[in] nz   The length of the initial condition array.
     *                 This must equal \c getInitialConditionLength().
     * @param[in] zi   The initial conditions.  This is an array of 
     *                 dimension [nz].
     * @throws std::invalid_argument if nz is invalid or nz is positive
     *         and zi is NULL.
     * @throws std::runtime_error if class is not initialized.
     */
    void setInitialConditions(const int nz, const double zi[]);
    /*!
     * @brief Estimates the space required to hold the downsampled signal.
     * @param[in] n   The length of the signal to downsample.  This must
     *                be non-negative.
     * @result The number of points required to store the output signal.
     * @throws std::runtime_error if the module is not initialized.
     * @throws std::invalid_argument if n is negative.
     */
    int estimateSpace(const int n) const;

    /*!
     * @brief Applies the decimator to the data.
     * @param[in] nx       The number data points in x.
     * @param[in] x        The signal to decimate.
     * @param[in] ny       The maximum number of samples in y.  One can
     *                     estimate ny by using estimateSpace(). 
     * @param[out] nyDown  The number of defined decimated points in y.
     * @param[out] y       The decimated signal.  This has dimension
     *                     [ny] however  only the first [nyDown] points
     *                     are defined.
     * @result 0 indicates success.
     * @throws std::invalid_argument if x or y is NULL.
     * @throws std::runtime_error if the module is not initialized.
     */
    void apply(const int nx, const T x[],
               const int ny, int *nyDown, T *y[]);
    /*!
     * @brief Resets the filter to its default initial conditions or the
     *        initial conditions set by \c setInitialConditions().
     *        This is useful after a gap.
     * @throws std::runtime_error if the class is not initialized.
     */
    void resetInitialConditions(); 

    /*! 
     * @brief Gets the downsampling factor.
     * @result The downsampling factor.
     * @throws std::runtime_error if the class is not initialized.
     */
    int getDownsamplingFactor() const;
    /*! 
     * @brief Gets the length of the FIR filter.
     * @result The number of FIR filter coefficients.
     * @throws std::runtime_error if the class is not initialized.
     */
    int getFIRFilterLength() const;
private:
    class DecimateImpl;
    std::unique_ptr<DecimateImpl> pImpl;
};
}
#endif
