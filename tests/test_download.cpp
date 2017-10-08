#include <iostream>
#include <sstream>
#include <cstdio>
#include <thread>
#include <map>
#include <fast-event-system/sync.h>
#include <fast-event-system/async_fast.h>
#include <teelogging/teelogging.h>
#include <asyncply/run.h>
#include <cppunix/parallel_scheduler.h>
#include "../serialize.h"
#include "../deserialize.h"
#include "../server.h"
#include "../client.h"
#include "../util.h"
#include "../download.h"

#define CHAT 300

int main()
{
	cu::parallel_scheduler sch;
	sch.spawn([](auto& yield)
	{
		download(yield, "http://ipv4.download.thinkbroadband.com/5MB.zip", "5MB.zip");
	});
	sch.run_until_complete();
	return 0;
}

