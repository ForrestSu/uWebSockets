/* This is a simple HTTP(S) web server much like Python's SimpleHTTPServer */

#include <App.h>

/* Helpers for this example */

int main(int argc, char **argv)
{

    int option;

    int port = 8060;

    /* Either serve over HTTP or HTTPS */

    /* HTTP */
    auto hub = uWS::App();
    hub.filter([](uWS::HttpResponse<false> *resp, int rc) {

        if (rc == 1) {
            std::cout << "Conn 0x" << (intptr_t)resp <<std::endl;
        } else if(rc == -1) {
            std::cout << "DisConn 0x" << (intptr_t)resp <<std::endl;
        }
    });
    hub.get("/*", [](uWS::HttpResponse<false> *resp, uWS::HttpRequest *req) {
        std::cout << "getQuery " << req->getQuery() <<std::endl;
        std::cout << "getUrl " << req->getUrl() <<std::endl;
        resp->end("asd");
    }).listen(port, [port](auto *token) {
        if (token) {
            std::cout << "Listen on " << port << std::endl;
        }
    }).run();

    std::cout << "Failed to listen to port " << port << std::endl;
}
