/* vim:ts=4
 *
 * Copyleft 2012…2016  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

// System:
#include <unistd.h>
#include <signal.h>

// Standard:
#include <cstddef>
#include <iostream>
#include <iterator>
#include <vector>
#include <string>

// Local:
#include <xefis/config/version.h>
#include <xefis/utility/backtrace.h>
#include <xefis/core/services.h>

#include "fail.h"


namespace xf {

std::atomic<bool> g_hup_received { false };


void
fail (int signum)
{
	using std::endl;

	std::vector<const char*> features = xf::Services::features();
	std::clog << endl;
	std::clog << "------------------------------------------------------------------------------------------------" << endl;
	std::clog << "Xefis died by a signal." << endl << endl;
	std::clog << "       signal: " << signum << endl;
	std::clog << "  source info: " << endl;
	std::cout << "       commit: " << ::xf::version::commit << endl;
	std::cout << "       branch: " << ::xf::version::branch << endl;
	std::clog << "     features: ";
	std::copy (features.begin(), features.end(), std::ostream_iterator<const char*> (std::clog, " "));
	std::clog << endl;
	std::clog << "    backtrace:" << endl;
	std::clog << backtrace().resolve_sources() << endl;
	std::clog << "     CXXFLAGS: " << CXXFLAGS << endl << endl;
	// Force coredump if enabled:
	signal (signum, SIG_DFL);
	kill (getpid(), signum);
}

} // namespace xf

