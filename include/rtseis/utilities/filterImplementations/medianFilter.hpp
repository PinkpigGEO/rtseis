#ifndef RTSEIS_UTILITIES_FILTERIMPLEMENTATIONS_MEDIAN_HPP
#define RTSEIS_UTILITIES_FILTERIMPLEMENTATIONS_MEDIAN_HPP 1
#include <memory>

namespace RTSeis::Utilities::FilterImplementations
{
/*!
 * @class MedianFilter medianFilter.hpp "include/rtseis/utilities/filterImplementations/medianFilter.hpp"
 * @brief This is the core implementation for median filtering.
 * @copyright Ben Baker distributed under the MIT license.
 * @ingroup rtseis_utils_filters
 */
template<class T = double>
class MedianFilter
{
public:
    /*! @name Constructors
     * @{
     */
    /*!
     * @brief Copy constructor.
     */
    MedianFilter();
    /*!
     * @brief Copy constructor.
     * @param[in] median  Median class from which to initialize.
     */
    MedianFilter(const MedianFilter &median);
    /*! @} */

    /*! @name Operators
     * @{
     */
    /*!
     * @brief Copy operator.
     * @param[in] median  The median class to copy.
     * @result A deep copy of the median filter class.
     */
    MedianFilter& operator=(const MedianFilter &median);
    /*! @} */

    /*! @name Destructors
     * @{
     */
    /*! 
     * @brief Destructor.
     */
    ~MedianFilter();
    /* @} */

    /*!
     * @brief Initializes the median filter.
     * @param[in] n   The window size of the median filter.  This must
     *                be a positive and odd number.  If n is not odd
     *                then it's length will be increased by 1.
     * @param[in] mode  The processing mode.  By default this
     *                  is for post-processing.
     * @throws std::invalid_argument if any of the arguments are invalid.
     */
    void initialize(const int n,
                    const RTSeis::ProcessingMode mode = RTSeis::ProcessingMode::POST_PROCESSING);
    /*!
     * @brief Determines if the module is initialized.
     * @retval True indicates that the module is initialized.
     * @retval False indicates that the module is not initialized.
     */
    bool isInitialized() const noexcept;
    /*!
     * @brief Utility routine to determine the initial condition length.
     * @result A non-negative number is the length of the initial
     *         condition array.
     * @throws std::runtime_error if the class is not initialized.
     */
    int getInitialConditionLength() const;
    /*!
     * @brief Returns the group delay of the filter.  Note, that this
     *        shift is required to get a correspondence to Matlab.
     * @throws std::runtime_error if the class is not initialized.
     */
    int getGroupDelay() const;
    /*!
     * @brief Sets the initial conditions for the filter.  This should
     *        be called prior to filter application as it will reset
     *        the filter.
     * @param[in] nz   The median filter initial conditions.  This
     *                 should be equal to getInitialConditionLength().
     * @param[in] zi   The initial conditions.  This has dimension [nz].
     * @throws std::invalid_argument if nz is invalid or nz is positive and
     *         zi is NULL.
     * @throws std::runtime_error if the class is not initialized.
     */
    void setInitialConditions(const int nz, const double zi[]);
    /*!
     * @brief Appplies the median filter to the array x.
     * @param[in] n   Number of points in x.
     * @param[in] x   The signal to filter.  This has dimension [n].
     * @param[out] y  The filtered signal.  This has dimension [n].
     * @throws std::invalid_argument if n is positive and x or y is NULL.
     * @throws std::runtime_error if the class is not initialized.
     */
    void apply(const int n, const T x[], T *y[]);
    /*!
     * @brief Resets the initial conditions on the source delay line to
     *        the default initial conditions or the initial conditions
     *        set when MedianFilter::setInitialConditions() was called.
     * @throws std::runtime_error if the class is not initialized.
     */ 
    void resetInitialConditions();
    /*! 
     * @brief Clears the module and resets all parameters.
     */
    void clear() noexcept;
 private:
    class MedianFilterImpl;
    std::unique_ptr<MedianFilterImpl> pMedian_;
}; // End medianFilter 
} // End RTSeis
#endif
