#pragma once


// TODO: Method for locking framerates
// Traditional Options: 
//		30, 40, 60, 120
// Screen Refresh Rate Based
// Example for 144 hz monitor:
//		18, 24, 36, 48, 72, 144

namespace Magnefu
{
	void		time_service_init();     // Called once at startup
	void		time_service_shutdown(); // Called at shutdown

	i64			time_now(); // Current time

	double		time_microseconds(i64 time); // Microseconds from current time
	double		time_milliseconds(i64 time);  // Milliseonds from current time
	double		time_seconds(i64 time);		  // Seconds from current time

	i64         time_from(i64 starting_time); // Get time difference from start to current time.

	double      time_from_microseconds(i64 starting_time); // Convenience method.
	double      time_from_milliseconds(i64 starting_time); // Convenience method.
	double      time_from_seconds(i64 starting_time);      // Convenience method.

	double      time_delta_seconds(i64 starting_time, i64 ending_time);
	double      time_delta_milliseconds(i64 starting_time, i64 ending_time);

#ifdef MF_DEBUG
	#define MF_ENABLE_TIMINGS
#endif

#ifdef MF_ENABLE_TIMINGS
	
	#define MF_START_TIME(x)			{ x = ::Magnefu::time_now(); }
	#define MF_END_TIME(x)				{ ::Magnefu::time_from(x); }
	#define MF_END_TIME_MICRO(x)		{ ::Magnefu::time_from_microseconds(x); }
	#define MF_END_TIME_MILLI(x)		{ ::Magnefu::time_from_milliseconds(x); }
	#define MF_END_TIME_SECONDS(x)		{ ::Magnefu::time_from_seconds(x); }

#else
#define MF_START_TIME(x)
#define MF_END_TIME_MILLI(x)
#endif
}