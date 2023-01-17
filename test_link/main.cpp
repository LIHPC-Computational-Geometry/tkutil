#include <iostream>
#include <string>
#include <TkUtil/Version.h>
#include <TkUtil/util_config.h>
#include <TkUtil/Exception.h>

int main() {
    const std::string string_version("2.3.5");
    TkUtil::Version tkutil_version(string_version);
    std::cout << "Version " << tkutil_version.getMajor() << "." << tkutil_version.getMinor() << std::endl;
    return 0;
}
