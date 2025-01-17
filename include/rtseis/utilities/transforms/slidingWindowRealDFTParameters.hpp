#ifndef RTSEIS_UTILITIES_TRANSFORMS_SLIDINGWINDOWREALDFTPARAMETERS_HPP
#define RTSEIS_UTILITIES_TRANSFORMS_SLIDINGWINDOWREALDFTPARAMETERS_HPP 1
#include <memory>
#include <vector>
#include "rtseis/enums.h"
#include "rtseis/utilities/transforms/enums.hpp"

namespace RTSeis::Utilities::Transforms
{
/*!
 * @brief Defines the parameters for sliding window real DFT.
 * @author Ben Baker, University of Utah
 * @copyright Ben Baker distributed under the MIT license.
 * @date July 2019
 */
class SlidingWindowRealDFTParameters
{
public:
    /*! @name Constructors
     * @{
     */
    /*!
     * @brief Default constructor.
     */
    SlidingWindowRealDFTParameters();
    /*!
     * @brief Copy constructor.
     * @param[in] parameters  The parameters from which to initialize
     *                        this class.
     */
    SlidingWindowRealDFTParameters(const SlidingWindowRealDFTParameters &parameters);
    /*!
     * @brief Move constructor.
     * @param[in,out] parameters  The parameters from which to initialize this
     *                            class. On exit, parameters's behavior is
     *                            undefined.
     */
    SlidingWindowRealDFTParameters(SlidingWindowRealDFTParameters &&parameters) noexcept;
    /*! @} */

    /*! @brief Operators
     * @{
     */
    /*!
     * @brief Copy assignment operator.
     * @param[in] parameters  The parameters to copy. 
     * @result A deep copy of the parameters.
     */
    SlidingWindowRealDFTParameters& operator=(const SlidingWindowRealDFTParameters &parameters);
    /*!
     * @brief Move assignment operator.
     * @param[in,out] parameters  The parameters to move to this.
     *                            On exit parameters behavior is undefined.
     * @result The memory from parameters moved onto this.
     */
    SlidingWindowRealDFTParameters& operator=(SlidingWindowRealDFTParameters &&parameters) noexcept;
    /*! @} */

    /*! @name Destructors
     * @{
     */
    /*!
     * @brief Default destructor.
     */
    ~SlidingWindowRealDFTParameters();
    /*!
     * @brief Resets the class and releases all memory.
     */
    void clear() noexcept;
    /*! @} */

    /*! @name Required Parameters
     * @{
     */
    /*! @name Number of samples
     * @{
     */
    /*!
     * @brief Sets the number of samples in the signal.
     * @param[in] nSamples  The number of samples in the signal.
     * @throws std::invalid_argument if the number of samples is not positive.
     */
    void setNumberOfSamples(const int nSamples);
    /*!
     * @brief Gets the number of samples in the signal.
     * @result The number of samples in the signal.
     * @throws std::runtime_error if this was not set.
     */
    int getNumberOfSamples() const;
    /*! @} */

    /*! @name Window function
     * @{
     */
    /*!
     * @brief Sets the window function that will be applied to each sample.
     * @note This defines the number of samples in each segment and will 
     *       override the dft length and set the number of samples in the
     *       overlap to 0.
     * @param[in] windowLength  The window length.
     * @param[in] window        The window function.  This cannot be custom.
     * @throws std::invalid_argument if the window length is not positive or
     *         the window type is invalid.
     */
    void setWindow(const int windowLength,
                   const SlidingWindowWindowType window = SlidingWindowWindowType::BOXCAR);
    /*!
     * @brief Sets a custom window function that will be applied to each sample.
     *        This is useful if the user wishes to use more involved window
     *        functions such as Kaiser or Tukey windows or window functions that
     *        RTSeis does not provide.
     * @note This defines the number of saples in each segment and will
     *       override the dft length and set the number of samples in the
     *       overlap to 0.
     * @param[in] windowLength  The window length.
     * @param[in] window        The window function.  This is an array
     *                          of dimension [windowLength].
     * @throws std::invalid_argument if the window length is not positive
     *         or the window is NULL.
     */
    void setWindow(const int windowLength, const double window[]);
    /*!
     * @brief Gets the window length.
     * @result The length of the window which is equivalent to the number of
     *         samples in each segment.
     * @throws std::invalid_argument if the window was not set.
     */
    int getWindowLength() const;
    /*!
     * @brief Gets a copy of the window function.
     * @result The window function.
     * @throws std::runtime_error if the window function was not set.
     * @sa \c haveWindow()
     */
    std::vector<double> getWindow() const;
    /*!
     * @brief Gets the window type.
     * @result The type of window function.
     * @throws std::runtime_error if the window was not set.
     */
    SlidingWindowWindowType getWindowType() const; 
    /*! @} */
    /*! @} */

    /*! @name Optional parameters
     * @{
     */
    /*! @name Number of points in overlap
     * @{
     */
    /*!
     * @brief Sets the number of samples in the overlap.
     * @note By default this is 0.
     * @param[in] nSamplesInOverlap   The number of samples in the overlap.
     *                                For this class to be valid this must
     *                                be in the range
     *                                [0,\c getWindowLength()-1]. 
     * @throws std::runtime_error if the window was not set.
     * @throws std::invalid_argument if nSamplesInOverlap is invalid.
     * @sa \c getWindowLength()
     */
    void setNumberOfSamplesInOverlap(const int nSamplesInOverlap);
    /*!
     * @brief Gets the number of samples in the overlap.
     * @result The number of samples in the overlap.
     */
    int getNumberOfSamplesInOverlap() const noexcept;
    /*! @}*/

    /*! @name The segment-wise detrend strategy
     * @{
     */
    /*!
     * @brief Sets the detrend type to be applied to each segment of
     *        the sliding DFT.
     * @param[in] detrendType  The detrend strategy.
     */
    void setDetrendType(const SlidingWindowDetrendType detrendType) noexcept;
    /*!
     * @brief Gets the detrend type to be applied to each segment of
     *        the sliding DFT.
     * @result The detrend type to apply to each segment.
     */
    SlidingWindowDetrendType getDetrendType() const noexcept; 
    /*! @} */ 
    /*!
     * @brief Sets the DFT length.  This is useful when the window function
     *        is an inefficient transform length as it will cause the program
     *        to zero pad the signal prior to transforming.
     * @note By default this is equal to the window length.
     * @param[in] dftLength  The length of the DFT.  This must be greater
     *                       than or equal to \c getWindowLength().
     * @throws std::runtime_argument if the window length was not set.
     * @throws std::invalid_argument if the DFT length is invalid.
     * @sa \c getWindowLength()
     */
    void setDFTLength(const int dftLength);
    /*!
     * @brief Gets the length of the DFT to be applied to each segment.
     * @result The length of the DFT.
     * @throws std::runtime_error if the window function was not set.
     * @sa \c setWindow()
     * @sa \c setDFTLength()
     */
    int getDFTLength() const;
    /*!
     * @brief Defines the precision of the underlying computation.
     * @param[in] precision  The precision of the underlying computations.
     * @note By default this is double precision. 
     */
    void setPrecision(const RTSeis::Precision mPrecision) noexcept;
    /*!
     * @brief Returns the precision of the underlying computations.
     * @result The precision of the underlying sliding window real DFT 
     *         computation.
     */  
    RTSeis::Precision getPrecision() const noexcept;
    /*! @} */

    /*! @name Valid
     * @{
     */
    /*!
     * @brief Determines if the class is or isn't valid.
     * @result True indicates that the class is a valid set of parameters
     *         for the sliding window DFT.
     */
    bool isValid() const noexcept;
    /*! @} */
private:
    class SlidingWindowRealDFTParametersImpl;
    std::unique_ptr<SlidingWindowRealDFTParametersImpl> pImpl;
};
}
#endif
