#pragma once
#include <iostream>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/c_local_time_adjustor.hpp>
using namespace boost::posix_time;
namespace simple_time
{
	typedef boost::posix_time::ptime PosixTime;
    typedef boost::posix_time::time_duration TimeDuration;
	
	const static char* DATE_FORMAT_1 = "%Y-%m-%d %H:%M:%S";
	const static char* DATE_FORMAT_2 = "%Y%m%d%H";
	const static char* DATE_FORMAT_3 = "%H";
	const static char* DATE_FORMAT_4 = "%Y%m%d%H%M";
	const static char* DATE_FORMAT_5 = "%Y-%m-%d %H:%M";
	const static char* DATE_FORMAT_6 = "%Y%m%d";
	const static char* DATE_FORMAT_7 = "%Y-%m-%d-%H-%M-%S";
	const static char* DATE_FORMAT_8 = "%Y-%m-%d %H:00";
	const static char* DATE_FORMAT_9 = "%Y年%m月%d日%H时%M分";
	const static char* DATE_FORMAT_10 = "%Y%m%f%h00";

    static PosixTime  NowPosixTime()
    {
        return second_clock::local_time();
    }
	static PosixTime TimeFromString(const std::string& str)
	{
		PosixTime _ptime;
		try
		{
			_ptime = boost::posix_time::time_from_string(str);
		}
		catch (const std::exception& e)
		{
			std::cerr << e.what() << std::endl;
		}
		return _ptime;
	}

	static std::string TimeToFormatString(const PosixTime& time, const std::string& format = DATE_FORMAT_5)
	{
		std::string retStr;
		try
		{
			boost::posix_time::time_facet* tf = new boost::posix_time::time_facet(format.c_str());
			std::stringstream ss;
			ss.imbue(std::locale(std::cout.getloc(), tf));
			ss << time;
			retStr = ss.str();
		}
		catch (const std::exception& e)
		{
			std::cerr << e.what() << std::endl;
		}

		return retStr;
	}
}

using namespace simple_time;

