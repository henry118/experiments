#include "ioservice.h"
#include <stdio.h>
#include <boost/bind.hpp>

using namespace shfe;

void printjob() {
    printf("%s\n", __func__);
    boost::this_thread::sleep(boost::posix_time::milliseconds(500));
}

int main() {
    IoService ios;
    ios.start();
    for (int i = 0; i < 20; ++i) {
        ios.async_run(boost::bind(printjob));
    }
    return 0;
}
