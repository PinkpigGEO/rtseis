#ifndef RTSEIS_UTILS_DESIGN_FILTERDESIGNER_HPP
#define RTSEIS_UTILS_DESIGN_FILTERDESIGNER_HPP
#include <memory>
#include "include/rtseis/utilities/design/fir.hpp"
#include "include/rtseis/utilities/design/iir.hpp"

namespace RTSeis
{
namespace Utilities
{
namespace FilterRepresentations
{
class BA; 
class SOS;
class ZPK;
class FIR;
};
namespace FilterDesign
{

/*!
 * @defgroup rtseis_utils_design_iir Filter Designer
 * @brief A class for filter design.  If designing many filters then using
 *        this class may be advantageous as it will save previous filter
 *        designs.
 * @copyright Ben Baker distributed under the MIT license.
 * @ingroup rtseis_utils_filterDesign
 */
class FilterDesigner
{
public:
    /*!
     * @name Constructors
     * @{
     */
    /*!
     * @brief Default constructor.
     */
    FilterDesigner(void); 
    /*!
     * @brief Copy constructor.
     * @param[in] design  Class from which to initialize this class.
     */
    FilterDesigner(const FilterDesigner &design); 
    /*! @} */

    /*! @name Operators
     * @{
     */
    /*!
     * @brief Copy assignment operator.
     * @param[in] design  Class to copy.
     * @result A deep copy of the filter designer class.
     */
    FilterDesigner& operator=(const FilterDesigner &design);
    /*! @} */

    /*! @name Destructors
     * @{
     */
    /*!
     * @brief Default destructor.
     */
    ~FilterDesigner(void);
    /*!
     * @brief Erases all existing filter designs.
     */
    void clear(void);
    /*! @} */

    /*! @name FIR Window-Based Filter Design
     * @{
     */
    /*!
     * @brief Designs an FIR lowpass filter.
     * @param[in] order   The filter order.  The number of taps is order+1.
     *                    This must be at least 4.
     * @param[in] r       The normalized cutoff frequency where 1 is the
     *                    Nyquist frequency.
     * @param[in] window  The FIR window design.
     * @param[out] fir    The lowpass filter that corresponds to the 
     *                    given design parameters.
     * @throws std::invalid_argument If any of the arguments are invalid.
     */
    void designLowpassFIRFilter(const int order,
                                const double r,
                                const FIRWindow window,
                                FilterRepresentations::FIR &fir) const;
    /*!
     * @brief Designs an FIR highpass filter.
     * @param[in] order   The filter order.  The number of taps is order+1.
     *                    This must be at least 4.
     * @param[in] r       The normalized cutoff frequency where 1 is the
     *                    Nyquist frequency.
     * @param[in] window  The FIR window design.
     * @param[out] fir    The highpass filter that corresponds to the 
     *                    given design parameters.
     * @throws std::invalid_argument If any of the arguments are invalid.
     */
    void designHighpassFIRFilter(const int order,
                                 const double r,
                                 const FIRWindow window,
                                 FilterRepresentations::FIR &fir) const;
    /*!
     * @brief Designs an FIR bandpass filter.
     * @param[in] order   The filter order.  The number of taps is order+1.
     *                    This must be at least 4.
     * @param[in] r       Normalized low cutoff frequency and high cutoff
     *                    frequencies where 1 is the Nyquist.  Here,
     *                    r.first is the low cutoff and r.second is the
     *                    high cutoff.
     * @param[in] window  The FIR window design.
     * @param[out] fir    If the bandpass filter that corresponds to the 
     *                    given design parameters.
     * @throws std::invalid_argument If any of the arguments are invalid.
     */
    void designBandpassFIRFilter(const int order,
                                 const std::pair<double,double> r,
                                 const FIRWindow window,
                                 FilterRepresentations::FIR &fir) const;
    /*!
     * @brief Designs an FIR bandstop filter.
     * @param[in] order   The filter order.  The number of taps is order+1.
     *                    This must be at least 4.
     * @param[in] r       Normalized low cutoff frequency and high cutoff
     *                    frequencies where 1 is the Nyquist.  Here,
     *                    r.first is the low cutoff and r.second is the
     *                    high cutoff.
     * @param[in] window  The FIR window design.
     * @param[out] fir    If the bandstop filter that corresponds to the 
     *                    given design parameters.
     * @throws std::invalid_argument If any of the arguments are invalid.
     */
    void designBandstopFIRFilter(const int order,
                                 const std::pair<double,double> r,
                                 const FIRWindow window,
                                 FilterRepresentations::FIR &fir) const;
    /*! @} */

    /*! @name IIR Analog Prototype-Based Design
     * @{
     */
    /*!
     * @brief Designs an IIR lowpass filter stored as zeros, poles, and a gain.
     * @param[in] n      The order of the filter.
     * @param[in] r      The normalized cutoff frequency where 1 is the
     *                   Nyquist frequency.
     * @param[in] ftype  The filter prototype.
     * @param[out] zpk   The lowpass filter design.
     * @param[in] ldigital  If true then design a design a digital filter.
     *                      Otherwise, design an analog filter.
     * @throws std::invalid_argument if any parameters are incorrect.
     */
    void designLowpassIIRFilter(const int n, const double r,
                                const IIRPrototype ftype,
                                const double ripple,
                                FilterRepresentations::ZPK &zpk,
                                const bool ldigital = true);
    /*!
     * @brief Designs an IIR lowpass filter.
     * @param[in] n      The order of the filter.
     * @param[in] r      The normalized cutoff frequency where 1 is the
     *                   Nyquist frequency.
     * @param[in] ftype  The filter prototype.
     * @param[out] ba    The lowpass filter design.
     * @param[in] ldigital  If true then design a design a digital filter.
     *                      Otherwise, design an analog filter.
     * @throws std::invalid_argument if any parameters are incorrect.
     */
    void designLowpassIIRFilter(const int n, const double r,
                                const IIRPrototype ftype,
                                const double ripple,
                                FilterRepresentations::BA &ba,
                                const bool ldigital = true);
    /*!
     * @brief Designs an IIR lowpass filter stored as second-order sections.
     * @param[in] n      The order of the filter.
     * @param[in] r      The normalized cutoff frequency where 1 is the
     *                   Nyquist frequency.
     * @param[in] ftype  The filter prototype.
     * @param[out] sos   The lowpass filter design.
     * @param[in] ldigital  If true then design a design a digital filter.
     *                      Otherwise, design an analog filter.
     * @throws std::invalid_argument if any parameters are incorrect.
     */
    void designLowpassIIRFilter(const int n, const double r,
                                const IIRPrototype ftype,
                                const double ripple,
                                FilterRepresentations::SOS &sos,
                                const bool ldigital = true);

    /*! 
     * @brief Designs an IIR highpass filter stored as zeros, poles, and a gain.
     * @param[in] n      The order of the filter.
     * @param[in] r      The normalized cutoff frequency where 1 is the
     *                   Nyquist frequency.
     * @param[in] ftype  The filter prototype.
     * @param[out] zpk   The highpass filter design.
     * @param[in] ldigital  If true then design a design a digital filter.
     *                      Otherwise, design an analog filter.
     * @throws std::invalid_argument if any parameters are incorrect.
     */
    void designHighpassIIRFilter(const int n, const double r,
                                 const IIRPrototype ftype,
                                 const double ripple,
                                 FilterRepresentations::ZPK &zpk,
                                 const bool ldigital = true);
    /*! 
     * @brief Designs an IIR highpass filter.
     * @param[in] n      The order of the filter.
     * @param[in] r      The normalized cutoff frequency where 1 is the
     *                   Nyquist frequency.
     * @param[in] ftype  The filter prototype.
     * @param[out] ba    The highpass filter design.
     * @param[in] ldigital  If true then design a design a digital filter.
     *                      Otherwise, design an analog filter.
     * @throws std::invalid_argument if any parameters are incorrect.
     */
    void designHighpassIIRFilter(const int n, const double r,
                                 const IIRPrototype ftype,
                                 const double ripple,
                                 FilterRepresentations::BA &ba,
                                 const bool ldigital = true);
    /*! 
     * @brief Designs an IIR highpass filter stored as second-order sections.
     * @param[in] n      The order of the filter.
     * @param[in] r      The normalized cutoff frequency where 1 is the
     *                   Nyquist frequency.
     * @param[in] ftype  The filter prototype.
     * @param[out] sos   The highpass filter design.
     * @param[in] ldigital  If true then design a design a digital filter.
     *                      Otherwise, design an analog filter.
     * @throws std::invalid_argument if any parameters are incorrect.
     */
    void designHighpassIIRFilter(const int n, const double r,
                                 const IIRPrototype ftype,
                                 const double ripple,
                                 FilterRepresentations::SOS &sos,
                                 const bool ldigital = true);

    /*! @} */
private:
    class FilterDesignerImpl;
    mutable std::unique_ptr<FilterDesignerImpl> pImpl;
}; // end filter design cache
}; // end filter designer
}; // end utilities
}; // end rtseis

#endif
