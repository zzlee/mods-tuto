#include "ZzLog.h"
#include "ZzUtils.h"
#include "ZzClock.h"

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/videodev2.h>
#include <linux/media.h>
#include <linux/version.h>
#include <linux/limits.h>
#include <vector>
#include <fstream>

#include "uapi/qenv.h"

ZZ_INIT_LOG("01_qenv-ctl")

using namespace __zz_clock__;

namespace __01_qenv_ctl__ {
	struct App {
		int argc;
		char **argv;

		ZzUtils::FreeStack oFreeStack;

		App(int argc, char **argv);
		~App();

		int Run();
	};

	App::App(int argc, char **argv) : argc(argc), argv(argv) {
		LOGD("%s(%d):", __FUNCTION__, __LINE__);
	}

	App::~App() {
		LOGD("%s(%d):", __FUNCTION__, __LINE__);
	}

	int App::Run() {
		int err;

		switch(1) { case 1:
			ZzUtils::TestLoop([&](int ch) -> int {
				int err = 0;

				LOGD("ch=%d", ch);

				switch(ch) {
				default:
					LOGD("%s(%d): TODO", __FUNCTION__, __LINE__);
					break;
				}

				return err;
			}, 1000000LL, 1LL);

			err = 0;
		}

		oFreeStack.Flush();

		return err;
	}
}

using namespace __01_qenv_ctl__;

int main(int argc, char *argv[]) {
	LOGD("entering...");

	int err;
	{
		App app(argc, argv);
		err = app.Run();

		LOGD("leaving...");
	}

	return err;
}
