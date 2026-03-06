#include "Application.h"
#include <curl/curl.h>
#include <iostream>

int main()
{
    curl_global_init(CURL_GLOBAL_DEFAULT);
    try {
        Application app;
        app.run();
    } catch (const std::exception& e) {
        std::cerr << "[Fatal] " << e.what() << '\n';
        curl_global_cleanup();
        return 1;
    }
    curl_global_cleanup();
    return 0;
}
