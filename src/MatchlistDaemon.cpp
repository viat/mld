/**
 * @file		MatchlistDaemon.cpp
 * @date		Jun 09, 2011
 * @author  	M.Sc. Julian Strobl
 * @brief   	Main class of MatchlistDaemon.
 * @details		Checks the entries in the 'matchlist' and fills the 'caller_blacklist' with its information.
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
// define
#include "_mlddefs.h"

// static configuration variables

/// Connection to PostgreSQL database.
static PGconn *conn;

/// Program name.
static const char *progname = "mld";

/// Parameter for SQL command.
static char **pvalue = NULL;

/// Logging directory.
static std::string LOG_DIR("");

/// Running directory.
static std::string PID_DIR("");

/// String for PostgreSQL connection.
static std::string CONNINFO("");

/// Polling database time
static long SLEEP(0);

/// How equal calls have to be to get blacklisted (in percent)
static long BLACKLIST_PERCENT(0);

/**
 * Parse the configuration file.
 * @param configfile Configuration file to be parsed.
 * @return true or false.
 */
int parseConfig(const std::string &configfile)
{
	std::ifstream infile;
	unsigned int linenum = 0;
	char line[256];
	char optionnam[256];
	char optionval[256];

	// open configuration file
	infile.open(configfile.c_str(), std::ifstream::in);

	if (infile.fail())
	{
		std::cerr << configfile << ": No such file or directory!" << std::endl;
		return false;
	}

	while (infile.getline(line, 256))
	{
		linenum++;

		// skip comments and empty lines
		if (line[0] == '#' || line[0] == '\n' || (line[0] == '\r' && line[1] == '\n') || line[0] == '\0')
			continue;

		// scan for option
		if (sscanf(line, "%s = %255[^\n]", optionnam, optionval) != 2)
		{
			std::cerr << "Syntax error in configuration file, line " << linenum << std::endl;
			return false;
		}

		// parse option
		if (strcmp(optionnam, "LOG_DIR") == 0)
			LOG_DIR = std::string(optionval);
		else if (strcmp(optionnam, "PID_DIR") == 0)
			PID_DIR = std::string(optionval);
		else if (strcmp(optionnam, "CONNINFO") == 0)
		{
			// copy option with "" to conninfo
			CONNINFO = std::string(optionval);

			// write to conninfo without quotes ("..." -> ...)
			CONNINFO.erase(0, 1);
			CONNINFO.erase(CONNINFO.size() - 1, 1);
		}
		else if (strcmp(optionnam, "SLEEP") == 0)
			SLEEP = atoi(optionval);
		else if (strcmp(optionnam, "BLACKLIST_PERCENT") == 0)
			BLACKLIST_PERCENT = atoi(optionval);
		else
			std::cout << "Wrong option in configuration file: '" << optionnam << "'! Option ignored!" << std::endl;

	}

	// close input file stream
	infile.close();

	// check for missing options
	if (LOG_DIR == "")
	{
		std::cerr << "Option 'LOG_DIR' is missing or empty string!" << std::endl;
		return false;
	}

	if (PID_DIR == "")
	{
		std::cerr << "Option 'LOG_DIR' is missing or empty string!" << std::endl;
		return false;
	}

	if (SLEEP <= 0)
	{
		std::cerr << "Option 'SLEEP' is missing or less equal zero!" << std::endl;
		return false;
	}

	if (BLACKLIST_PERCENT <= 0 || BLACKLIST_PERCENT > 100)
	{
		std::cerr << "Option 'BLACKLIST_PERCENT' is missing, less equal zero or greater than hundred!" << std::endl;
		return false;
	}

	return true;
}

/**
 * Clean up before quitting.
 * @return true or false.
 */
static long int free_memory()
{
	long int i = 0;

	for (i = 0; i < NPARAM; i++)
		free(pvalue[i]);

	return TRUE;
}

/**
 * This exits the daemon in a nicer way. The allocated memory will be freed,
 * the database connection closed before the daemon exits.
 * Its used in every error case.
 * @param conn database connection
 */
void exit_nicely(PGconn *conn)
{
	PGresult *cmd;

	cmd = PQexec(conn, "UPDATE matchlist SET processed = -1");
	if (PQresultStatus(cmd) != PGRES_COMMAND_OK)
	{
		L_e << "<ERROR> UPDATE failed: " << PQerrorMessage(conn);
	}
	PQclear(cmd);

	// close the connection to the database and cleanup
	PQfinish(conn);

	if (!free_memory())
		L_e << "<ERROR> Freeing memory failed!";

	// log
	L_i << "Memory freed! Quit!";

	// exit
	exit(1);
}

/**
 * UNIX signal handler.
 * @param sig Signal. 'SIGHUP' will be ignored. 'SIGTERM' exits the daemon.
 */
void signal_handler(int sig)
{
	switch (sig)
	{
	case SIGHUP:
		L_i << "<WARN> Hangup signal detected, but ignored";
		break;
	case SIGTERM:
		L_i << "Terminate signal catched. Quitting " << progname << "[" << getpid() << "]";
		exit_nicely(conn);
		break;
	}
}

/**
 * Transforms program to a UNIX-Daemon. Closes file descriptors, etc..
 * @return TRUE if bringing daemon up succeeded. FALSE otherwise.
 */
static long int daemonize()
{
	// init
	long int lfp = 0;
	char str[10];

	if (getppid() == 1)
		return FALSE;

	if (fork() > 0) // parent exits
		exit(0);

	// obtain a new process group
	setsid();

	// set file permissions
	umask(027);

	// change running directory
	if (chdir(PID_DIR.c_str()))
	{
		L_e << "<ERROR> Error opening running directory '" << PID_DIR << "': " << strerror(errno);
		return FALSE;
	}

	lfp = open(LOCK_FILE, O_RDWR | O_CREAT, 0640);
	if (lfp < 0)
	{
		// cannot open
		L_e << "<ERROR> Cannot open '" << PID_DIR << LOCK_FILE << "': " << strerror(errno);
		return FALSE;
	}
	if (lockf(lfp, F_TLOCK, 0) < 0)
	{
		// cannot lock
		L_e << "<ERROR> Daemon already running";
		return FALSE;
	}

	// first instance continues
	sprintf(str, "%d\n", getpid());
	// record pid to lockfile
	write(lfp, str, strlen(str));

	// Set STDOUT and STDERR to unbuffered streams for direct printing to a file.
	setbuf(stdout, NULL);
	setbuf(stderr, NULL);

	// ignore child
	signal(SIGCHLD, SIG_IGN);
	// ignore tty signals
	signal(SIGTSTP, SIG_IGN);
	signal(SIGTTOU, SIG_IGN);
	signal(SIGTTIN, SIG_IGN);
	// catch hangup signal
	signal(SIGHUP, signal_handler);
	// catch kill signal
	signal(SIGTERM, signal_handler);

	return TRUE;
}

/**
 * Allocates needed memory for SQL parameters.
 * @return true or false.
 */
static long int allocateMemory()
{
	unsigned int i = 0;

	// parameters for sql statements
	if (!(pvalue = (char**) calloc(NPARAM * (NUMDECIMALPLACES + 1), sizeof(**pvalue))))
	{
		L_e << "<ERROR> pvalue: Memory allocation failed!";
		return FALSE;
	}
	for (i = 0; i < NPARAM; i++)
		pvalue[i] = (char*) calloc((NUMDECIMALPLACES + 1), sizeof(*pvalue));

	return TRUE;
}

/**
 * Reads from 'matchlist' and fills the caller_blacklist.
 * @return true or false.
 */
static long int dbProcessMatchlist()
{
	unsigned long int call_id = 0;
	unsigned long int matched_call_id = 0;
	unsigned long int caller_id = 0;

	long int i = 0;

	// build parameter for sql
	snprintf(pvalue[0], NUMDECIMALPLACES + 1, "%lu", BLACKLIST_PERCENT);

	PGresult *res = PQexecParams(conn, "SELECT call_id, matched_call_id FROM matchlist WHERE processed = -1 AND (actual_mismatches*100)/length_query <= $1 ORDER BY call_id, matched_call_id", 1, NULL,
			(const char**) pvalue, NULL, NULL, 0);
	if (PQresultStatus(res) != PGRES_TUPLES_OK)
	{
		L_e << "<ERROR> SELECT failed: " << PQerrorMessage(conn);
		exit_nicely(conn);
	}

	if (PQntuples(res))
	{
		for (i = 0; i < PQntuples(res); i++)
		{
			PGresult *result;

			call_id = (unsigned long int) atoi(PQgetvalue(res, i, 0));
			matched_call_id = (unsigned long int) atoi(PQgetvalue(res, i, 1));

			// build parameter for sql
			snprintf(pvalue[0], NUMDECIMALPLACES + 1, "%lu", call_id);
			snprintf(pvalue[1], NUMDECIMALPLACES + 1, "%lu", matched_call_id);

			result = PQexecParams(conn, "SELECT caller.id FROM caller WHERE caller.id IN (SELECT call.caller_id FROM call WHERE call.id = $1)", 1, NULL,
					(const char**) pvalue, NULL, NULL, 0);
			if (PQresultStatus(result) != PGRES_TUPLES_OK)
			{
				L_e << "<ERROR> SELECT failed: " << PQerrorMessage(conn);
				exit_nicely(conn);
			}
			caller_id = (unsigned long int) atoi(PQgetvalue(result, 0, 0));
			PQclear(result);

			// build parameter for sql
			snprintf(pvalue[0], NUMDECIMALPLACES + 1, "%lu", caller_id);
			snprintf(pvalue[1], NUMDECIMALPLACES + 1, "%lu", call_id);

			// delete old entry
			result = PQexecParams(conn, "DELETE FROM caller_blacklist WHERE caller_id = $1", 1, NULL, (const char**) pvalue, NULL, NULL, 0);
			if (PQresultStatus(result) != PGRES_COMMAND_OK)
			{
				L_e << "<ERROR> DELETE failed: " << PQerrorMessage(conn);
				exit_nicely(conn);
			}
			PQclear(result);

			result =
					PQexecParams(
							conn,
							"INSERT INTO caller_blacklist (caller_id, call_id, timestamp, reason) VALUES ( $1, $2, (SELECT CURRENT_TIMESTAMP(0)), (SELECT value FROM config WHERE name = 'viat_blacklist_reason' LIMIT 1) )",
							2, NULL, (const char**) pvalue, NULL, NULL, 0);
			if (PQresultStatus(result) != PGRES_COMMAND_OK)
			{
				L_e << "<ERROR> INSERT failed: " << PQerrorMessage(conn);
				exit_nicely(conn);
			}
			PQclear(result);

			// build parameter for sql
			snprintf(pvalue[0], NUMDECIMALPLACES + 1, "%lu", call_id);
			snprintf(pvalue[1], NUMDECIMALPLACES + 1, "%lu", matched_call_id);

			result = PQexecParams(conn, "UPDATE matchlist SET processed = 1 WHERE matchlist.call_id = $1 AND matchlist.matched_call_id = $2", 2, NULL,
					(const char**) pvalue, NULL, NULL, 0);
			if (PQresultStatus(result) != PGRES_COMMAND_OK)
			{
				L_e << "<ERROR> UPDATE failed: " << PQerrorMessage(conn);
				exit_nicely(conn);
			}
			PQclear(result);
		}
	}
	PQclear(res);

	return TRUE;
}

/**
 * Initiates Boost logging.
 * @param logfile Logging output file.
 */
void init_logging(std::string logfile)
{
	logging::add_common_attributes();

	logging::settings setts;

	setts["Sink:File"]["Destination"] = "TextFile";
	setts["Sink:File"]["FileName"] = logfile;
	setts["Sink:File"]["AutoFlush"] = true;
	setts["Sink:File"]["DisableLogging"] = false;
	setts["Sink:File"]["Format"] = "[%TimeStamp%]: %_%";

	logging::init_from_settings(setts);
}

/**
 * Main method of MatchlistDaemon.
 * @param argc number of arguments.
 * @param argv argument as strings.
 * @return EXIT_SUCCESS if everything went fine.
 */
int main(int argc, char **argv)
{
	// init
	std::string configfile("");

	// config file as parameter?
	if (argc > 1)
		configfile = std::string(argv[1]);
	else
		configfile = std::string("/etc/viat/mld.conf");

	// parse config file
	if (!parseConfig(configfile))
		return EXIT_FAILURE;

	// logging
	init_logging(LOG_DIR + "mld.log");

	// daemon
	if (!daemonize())
		return EXIT_FAILURE;

	// make a connection to the database
	conn = PQconnectdb(CONNINFO.c_str());
	// check to see that the backend connection was successfully made
	if (PQstatus(conn) != CONNECTION_OK)
	{
		L_e << "<ERROR> Connection to database failed: " << PQerrorMessage(conn);
		exit_nicely(conn);
	}

	// log
	L_i << "Connection as user '" << PQuser(conn) << "' to database '" << PQhost(conn) << ":" << PQport(conn) << "/" << PQdb(conn) << "' succeeded.";

	if (!allocateMemory())
		return FALSE;

	while (TRUE)
	{
		if (!dbProcessMatchlist())
			return FALSE;

		usleep(SLEEP * 1000);
	}

	// exit
	exit_nicely(conn);

	return EXIT_SUCCESS;
}
