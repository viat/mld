/**
 * @file		_VIAT_Log.hpp
 * @brief		A logger definition that fits the needs in VIAT.
 * @date		Sep 24, 2012
 * @author  	M.Sc. Julian Strobl
 * @copyright  	Copyright (c) 2012 Julian Strobl<br>
 * 				Cologne University of Applied Sciences<br>
 * 				<br>
 * 				This program is free software: you can redistribute it and/or modify
 *				it under the terms of the GNU General Public License as published by
 *				the Free Software Foundation, either version 3 of the License, or
 * 				(at your option) any later version.<br>
 *				This program is distributed in the hope that it will be useful,
 *				but WITHOUT ANY WARRANTY; without even the implied warranty of
 *				MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *				GNU General Public License for more details.<br>
 *				You should have received a copy of the GNU General Public License
 *				along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef VIAT_LOG_HPP_
#define VIAT_LOG_HPP_

#include <ostream>
#include <boost/log/detail/prologue.hpp>
#include <boost/log/keywords/severity.hpp>
#include <boost/log/sources/global_logger_storage.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>

#ifdef _MSC_VER
#pragma warning(push)
// 'm_A' : class 'A' needs to have dll-interface to be used by clients of class 'B'
#pragma warning(disable: 4251)
// non dll-interface class 'A' used as base for dll-interface class 'B'
#pragma warning(disable: 4275)
#endif // _MSC_VER

#if !defined(BOOST_LOG_USE_CHAR)
#error Boost.Log: Trivial logging is available for narrow-character builds only. Use advanced initialization routines to setup wide-character logging.
#endif

namespace boost {

namespace BOOST_LOG_NAMESPACE {

namespace viat {

namespace aux {

    //! Initialization routine
    BOOST_LOG_EXPORT void init();

} // namespace aux

//! Trivial severity levels
enum severity_level
{
    trace,
    debug,
    info,
    warning,
    error,
    fatal
};

template< typename CharT, typename TraitsT >
inline std::basic_ostream< CharT, TraitsT >& operator<< (
    std::basic_ostream< CharT, TraitsT >& strm, severity_level lvl)
{
    switch (lvl)
    {
    case trace:
        strm << "trace"; break;
    case debug:
        strm << "debug"; break;
    case info:
        strm << "info"; break;
    case warning:
        strm << "warning"; break;
    case error:
        strm << "error"; break;
    case fatal:
        strm << "fatal"; break;
    default:
        strm << static_cast< int >(lvl); break;
    }

    return strm;
}

//! Trivial logger type
#if !defined(BOOST_LOG_NO_THREADS)
typedef sources::severity_logger_mt< severity_level > viat_logger;
#else
typedef sources::severity_logger< severity_level > viat_logger;
#endif

#if !defined(BOOST_LOG_BUILDING_THE_LIB)

//! Global logger instance
BOOST_LOG_DECLARE_GLOBAL_LOGGER_INIT(logger, viat_logger)
{
    return viat_logger(keywords::severity = info);
}

//! The macro is used to initiate logging
#define BOOST_LOG_VIAT(lvl)\
    BOOST_LOG_STREAM_WITH_PARAMS(::boost::log::viat::get_logger(),\
        (::boost::log::keywords::severity = ::boost::log::viat::lvl))

#endif // !defined(BOOST_LOG_BUILDING_THE_LIB)

} // namespace viat

} // namespace log

} // namespace boost

#ifdef _MSC_VER
#pragma warning(pop)
#endif // _MSC_VER


#endif /* VIAT_LOG_HPP_ */
