/**
 * @file 		_mlddefs.h
 * @brief   	Collection of includes.
 * @date		Dec 07, 2010
 * @author  	B.Sc. Julian Strobl
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

// STD
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fstream>

// DAEMON
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

// LOG
#include <syslog.h>

// POSTGRESQL
#include "libpq-fe.h"

// TIME
#include <time.h>
#include <sys/timeb.h>

// LOG
#include <boost/log/trivial.hpp>
#include <boost/log/filters.hpp>
#include <boost/log/utility/init/from_settings.hpp>
#include "boost/log/utility/init/common_attributes.hpp"

#include "_VIAT_Log.hpp"

#define L_t BOOST_LOG_VIAT(trace)   << "[trace] [" 		<< __FILE__ << ":" << __LINE__ << "] "
#define L_d BOOST_LOG_VIAT(debug)   << "[debug] [" 		<< __FILE__ << ":" << __LINE__ << "] "
#define L_i BOOST_LOG_VIAT(info)    << "[info] [" 		<< __FILE__ << ":" << __LINE__ << "] "
#define L_w BOOST_LOG_VIAT(warning) << "[warning] [" 	<< __FILE__ << ":" << __LINE__ << "] "
#define L_e BOOST_LOG_VIAT(error)   << "[error] [" 		<< __FILE__ << ":" << __LINE__ << "] "
#define L_f BOOST_LOG_VIAT(fatal)   << "[fatal] [" 		<< __FILE__ << ":" << __LINE__ << "] "

namespace logging = boost::log;
namespace sinks = boost::log::sinks;
namespace src = boost::log::sources;
namespace flt = boost::log::filters;
namespace attrs = boost::log::attributes;
namespace keywords = boost::log::keywords;

#ifndef DAEMONDEFS_H_
#define DAEMONDEFS_H_

/// Definition of FALSE
#define FALSE            	0
/// Definition of TRUE
#define TRUE             	1
/// Define number of decimal places
#define NUMDECIMALPLACES	20
/// Define maximal number of SQL-Parameters
#define NPARAM				3
/// Define polling database time
#define SLEEP				100
/// Definition of LOCK_FILE for process identification number. Used to make killing of process easier and to allow only one instance of Index Daemon.
#define LOCK_FILE        	"mld.pid"

#endif /* DAEMONDEFS_H_ */
