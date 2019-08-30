// NOTE: This is not part of the library, this file holds examples and tests

#include "uWS.h"
#include <iostream>
#include <chrono>
#include <cmath>
#include <thread>
#include <fstream>
#include <vector>
#include <set>
#include <unordered_set>
#include <unordered_map>
#include <map>
#include <atomic>
#include "unistd.h"

void testHTTP() {

    std::cout << "testHTTP() " <<std::endl;

    uWS::Hub h;
    std::atomic<int> expectedRequests(0);

    auto controlData = [&h, &expectedRequests](uWS::HttpResponse *res, char *data, size_t length, size_t remainingBytes) {
        if(res->httpSocket->getUserData() == nullptr){
            res->httpSocket->setUserData(new std::string);
        }
        std::string *buffer = (std::string *) res->httpSocket->getUserData();
        buffer->append(data, length);

        std::cout << "HTTP POST, chunk: " << length << ", total: " << buffer->length() << ", remainingBytes: " << remainingBytes << std::endl;

        if (!remainingBytes) {
            // control the contents
            for (unsigned int i = 0; i < buffer->length(); i++) {
                if ((*buffer)[i] != char('0' + i % 10)) {
                    std::cout << "FAILURE: corrupt data received in HTTP post!" << std::endl;
                    //exit(-1);
                    break;
                }
            }

            expectedRequests++;

            if(buffer != nullptr) {
                delete buffer;
                res->httpSocket->setUserData(nullptr);
            }
            res->end();
        }
    };

    h.onHttpData([&controlData](uWS::HttpResponse *res, char *data, size_t length, size_t remainingBytes) {
        std::string dataStr = "null";
        if(length > 0) {
            dataStr = std::string(data, length>10?10:length);
        }
        std::cout <<"3 [onHttpData] " <<  dataStr << ", length = " << length << ", remainingBytes = " <<remainingBytes << std::endl;
        controlData(res, data, length, remainingBytes);
    });

    h.onHttpRequest([&h, &expectedRequests, &controlData](uWS::HttpResponse *res, uWS::HttpRequest req, char *data, size_t length, size_t remainingBytes) {

        auto method = req.getMethod();
        const auto url = req.getUrl().toString();

        std::cout <<"2 [onHttpRequest] method = " << method << ", URL = <" << url <<">,  str = <"  << req.getMethodStr() <<">"<< std::endl;

        std::cout << "key == " <<req.headers->key <<std::endl;

        if (url == "/segmentedUrl") {
            if (method == uWS::HttpMethod::METHOD_GET && req.getHeader("host").toString() == "localhost") {
                expectedRequests++;
                res->end();
                return;
            }
        } else if (url == "/closeServer") {
            if (method == uWS::HttpMethod::METHOD_PUT) {
                res->setUserData((void *) 1234);
                // this will trigger a cancelled request
                h.getDefaultGroup<uWS::SERVER>().close();
                expectedRequests++;
                return;
            }
        } else if (url == "/postTest") {
            if (method == uWS::HttpMethod::METHOD_POST) {
                controlData(res, data, length, remainingBytes);
                return;
            }
        } else if (url == "/packedTest") {
            if (method == uWS::HttpMethod::METHOD_GET) {
                expectedRequests++;
                res->end();
                return;
            }
        } else if (url == "/firstRequest") {
            // store response in user data
            res->httpSocket->setUserData(res);
            return;
        } else if (url == "/secondRequest") {
            // respond to request out of order
            std::string secondResponse = "Second request responded to";
            res->end(secondResponse.data(), secondResponse.length());
            std::string firstResponse = "First request responded to";
            ((uWS::HttpResponse *) res->httpSocket->getUserData())->end(firstResponse.data(), firstResponse.length());
            return;
        } else {
             std::string dataStr = "null";
            if(length > 0) {
               dataStr = std::string(data, length>20?20:length);
            }
            std::cout << "ERROR no support!  data == <"<< dataStr << ">, len = " << length  << ", remainingBytes == " << remainingBytes << std::endl;
            if (remainingBytes == 0) {

               // res->write("world", 5);
                res->end("Hello", 5);
               return;
            }
            controlData(res, data, length, remainingBytes);
        }
    });

    h.onCancelledHttpRequest([&expectedRequests](uWS::HttpResponse *res) {
        if (res->getUserData() == (void *) 1234) {
            // let's say we want this one cancelled
            expectedRequests++;
        } else {
            std::cerr << "FAILURE: Unexpected cancelled request!" << std::endl;
            exit(-1);
        }
    });


    h.onHttpConnection([](uWS::HttpSocket<uWS::SERVER> *http){
        std::cout << "1 [onHttpConnection]" <<std::endl;
    });

    h.onHttpDisconnection([](uWS::HttpSocket<uWS::SERVER> *http){
        std::cout << "4 [onHttpDisconnection]" <<std::endl;
    });



    /////////////////////////////////////

    h.onConnection([&expectedRequests](uWS::WebSocket<uWS::SERVER> *ws, uWS::HttpRequest req) {

        std::cout <<"1 [onConnection]" << std::endl;
        if (req.getUrl().toString() == "/upgradeUrl") {
            if (req.getMethod() == uWS::HttpMethod::METHOD_GET && req.getHeader("upgrade").toString() == "websocket") {
                expectedRequests++;
                return;
            }
        }

        std::cerr << "FAILURE: Unexpected request!" << std::endl;
        exit(-1);
    });

    h.onDisconnection([](uWS::WebSocket<uWS::SERVER> *ws, int code, char *message, size_t length) {
        std::cout <<"4 [onDisconnection] code == " << code << std::endl;
        if(ws->getUserData() != nullptr) {
            delete (std::string *) ws->getUserData();
            ws->setUserData(nullptr);
        }
    });

    h.listen(3000);


    /*std::thread t([&expectedRequests]() {
        FILE *nc;

        int case_no = 1;
        std::cout <<"====> case "<< ++case_no << std::endl;
        // invalid data
        nc = popen("nc localhost 3000 > /dev/null 2>&1", "w");
        fputs("invalid http", nc);
        pclose(nc);

        std::cout <<"====> case "<< ++case_no << std::endl;
        nc = popen("nc localhost 3000 > /dev/null 2>&1", "w");
        fputs("\r\n\r\n", nc);
        pclose(nc);

        std::cout <<"====> case "<< ++case_no << std::endl;
        nc = popen("nc localhost 3000 > /dev/null 2>&1", "w");
        fputs("\r\n", nc);
        pclose(nc);


        std::cout <<"====> case "<< ++case_no << std::endl;
        nc = popen("nc localhost 3000 > /dev/null 2>&1", "w");
        fputs("\r\n\r", nc);
        pclose(nc);

        std::cout <<"====> case "<< ++case_no << std::endl;
        nc = popen("nc localhost 3000 > /dev/null 2>&1", "w");
        fputs("\r", nc);
        pclose(nc);

        std::cout <<"====> case "<< ++case_no << std::endl;
        nc = popen("nc localhost 3000 > /dev/null 2>&1", "w");
        fputs("\n", nc);
        pclose(nc);

        std::cout <<"====> case "<< ++case_no << std::endl;
        nc = popen("nc localhost 3000 > /dev/null 2>&1", "w");
        fputs("GET \r\n", nc);
        pclose(nc);

        std::cout <<"====> case "<< ++case_no << std::endl;
        nc = popen("nc localhost 3000 > /dev/null 2>&1", "w");
        fputs("GET / HTTP/1.1\r\n", nc);
        pclose(nc);


        std::cout <<"====> case "<< ++case_no << std::endl;
        nc = popen("nc localhost 3000 > /dev/null 2>&1", "w");
        fputs("GET / HTTP/1.1\r\nHost: localhost:3000", nc);
        pclose(nc);



        std::cout <<"====> case "<< ++case_no << std::endl;
        // segmented GET
        nc = popen("nc localhost 3000 > /dev/null 2>&1", "w");
        fputs("GET /segme", nc);
        fflush(nc);
        usleep(100000);

        std::cout <<"====> case "<< ++case_no << std::endl;
        fputs("ntedUrl HTTP/1.1\r", nc);
        fflush(nc);
        usleep(100000);


        std::cout <<"====> case "<< ++case_no << std::endl;
        fputs("\nHost: loca", nc);
        fflush(nc);
        usleep(100000);

        std::cout <<"====> case "<< ++case_no << std::endl;
        fputs("lhost\r\n\r\n", nc);
        fflush(nc);
        usleep(100000);
        pclose(nc);

        std::cout <<"====> case "<< ++case_no << std::endl;
        // segmented upgrade
        nc = popen("nc localhost 3000 > /dev/null 2>&1", "w");
        fputs("GET /upgra", nc);
        fflush(nc);
        usleep(100000);

        std::cout <<"====> case "<< ++case_no << std::endl;
        fputs("deUrl HTTP/1.1\r", nc);
        fflush(nc);
        usleep(100000);

        std::cout <<"====> case "<< ++case_no << std::endl;
        fputs("\nSec-WebSocket-Key: 123456789012341234567890\r", nc);
        fflush(nc);
        usleep(100000);

        std::cout <<"====> case "<< ++case_no << std::endl;
        fputs("\nUpgrade: websoc", nc);
        fflush(nc);
        usleep(100000);

        std::cout <<"====> case "<< ++case_no << std::endl;
        fputs("ket\r\n\r\n", nc);
        fflush(nc);
        usleep(100000);
        pclose(nc);


        std::cout <<"====> case "<< ++case_no << std::endl;
        // slow GET should get disconnected
        nc = popen("nc localhost 3000 > /dev/null 2>&1", "w");
        sleep(3);
        fputs("GET /slowRequest HTTP/1.1\r\n\r\n", nc);
        pclose(nc);

        // post tests with increading data length
        for (int j = 0; j < 10; j++) {
            nc = popen("nc localhost 3000 > /dev/null 2>&1", "w");
            fputs("POST /postTest HTTP/1.1\r\nContent-Length: ", nc);

            int contentLength = j * 1000000;
            std::cout << "POSTing " << contentLength << " bytes" << std::endl;

            fputs(std::to_string(contentLength).c_str(), nc);
            fputs("\r\n\r\n", nc);
            for (int i = 0; i < (contentLength / 10); i++) {
                fputs("0123456789", nc);
            }
            pclose(nc);
        }

        // todo: two-in-one GET, two-in-one GET, upgrade, etc

        // segmented second GET
        nc = popen("nc localhost 3000 > /dev/null 2>&1", "w");
        fputs("GET /packedTest HTTP/1.1\r\n\r\nGET /packedTest HTTP/", nc);
        fflush(nc);
        usleep(100000);
        fputs("1.1\r\n\r\n", nc);
        pclose(nc);

        std::cout <<"====> case "<< ++case_no << std::endl;
        nc = popen("nc localhost 3000 > /dev/null 2>&1", "w");
        fputs("GET /packedTest HTTP/1.1\r\n\r\nGET /packedTest HTTP/1.1\r\n\r\n", nc);
        pclose(nc);

        std::cout <<"====> case "<< ++case_no << std::endl;
        // out of order responses
        nc = popen("nc localhost 3000 > /dev/null 2>&1", "w");
        fputs("GET /firstRequest HTTP/1.1\r\n\r\nGET /secondRequest HTTP/1.1\r\n\r\n", nc);
        pclose(nc);


        std::cout <<"====> case "<< ++case_no << std::endl;
        // shutdown
        nc = popen("nc localhost 3000 > /dev/null 2>&1", "w");
        fputs("PUT /closeServer HTTP/1.1\r\n\r\n", nc);
        pclose(nc);
        if (expectedRequests != 18) {
            std::cerr << "FAILURE: expectedRequests differ: " << expectedRequests << std::endl;
            exit(-1);
        }
    });
    */

    h.run();
    //t.join();
}

// todo: move this out to examples folder, it is not a test but a stragiht up example of EventSource support
void serveEventSource() {
    uWS::Hub h;

    std::string document = "<script>var es = new EventSource('/eventSource'); es.onmessage = function(message) {document.write('<p><b>Server sent event:</b> ' + message.data + '</p>');};</script>";
    std::string header = "HTTP/1.1 200 OK\r\nContent-Type: text/event-stream\r\n\r\n";

    // stop and delete the libuv timer on http disconnection
    h.onHttpDisconnection([](uWS::HttpSocket<uWS::SERVER> *s) {
        uS::Timer *timer = (uS::Timer *) s->getUserData();
        if (timer) {
            timer->stop();
            timer->close();
        }
    });

    // terminate any upgrade attempt, this is http only
    h.onHttpUpgrade([](uWS::HttpSocket<uWS::SERVER> *s, uWS::HttpRequest req) {
        s->terminate();
    });

    // httpRequest borde vara defaultsatt till att hantera upgrades, ta bort onupgrade! (sätter man request avsätts upgrade handlern)
    h.onHttpRequest([&h, &document, &header](uWS::HttpResponse *res, uWS::HttpRequest req, char *data, size_t length, size_t remainingBytes) {
        std::string url = req.getUrl().toString();

        if (url == "/") {
            // respond with the document
            res->end((char *) document.data(), document.length());
            return;
        } else if (url == "/eventSource") {

            if (!res->getUserData()) {
                // establish a text/event-stream connection where we can send messages server -> client at any point in time
                res->write((char *) header.data(), header.length());

                // create and attach a libuv timer to the socket and let it send messages to the client each second
                uS::Timer *timer = new uS::Timer(h.getLoop());
                timer->setData(res);
                timer->start([](uS::Timer *timer) {
                    // send a message to the browser
                    std::string message = "data: Clock sent from the server: " + std::to_string(clock()) + "\n\n";
                    ((uWS::HttpResponse *) timer->getData())->write((char *) message.data(), message.length());
                }, 1000, 1000);
                res->setUserData(timer);
            } else {
                // why would the client send a new request at this point?
                res->getHttpSocket()->terminate();
            }
        } else {
            res->getHttpSocket()->terminate();
        }
    });

    h.listen(3000);
    h.run();
}

void serveHttp() {
    uWS::Hub h;

    std::string document = "<h2>Well hello there, this is a basic test!</h2>";

    h.onHttpRequest([&document](uWS::HttpResponse *res, uWS::HttpRequest req, char *data, size_t length, size_t remainingBytes) {
        res->end(document.data(), document.length());
    });

    h.listen(3000);
    std::cout << "listen 3000" << std::endl;
    h.run();
}

void testReceivePerformance() {
    // Binary "Rock it with HTML5 WebSocket"
    unsigned char webSocketMessage[] = {0x82, 0x9c, 0x37, 0x22, 0x79, 0xa5, 0x65, 0x4d, 0x1a, 0xce, 0x17, 0x4b, 0x0d, 0x85, 0x40, 0x4b
    ,0x0d, 0xcd, 0x17, 0x6a, 0x2d, 0xe8, 0x7b, 0x17, 0x59, 0xf2, 0x52, 0x40, 0x2a, 0xca, 0x54, 0x49
    ,0x1c, 0xd1};

//    // Text "Rock it with HTML5 WebSocket"
//    unsigned char webSocketMessage[] = {0x81, 0x9c, 0x37, 0x22, 0x79, 0xa5, 0x65, 0x4d, 0x1a, 0xce, 0x17, 0x4b, 0x0d, 0x85, 0x40, 0x4b
//    ,0x0d, 0xcd, 0x17, 0x6a, 0x2d, 0xe8, 0x7b, 0x17, 0x59, 0xf2, 0x52, 0x40, 0x2a, 0xca, 0x54, 0x49
//    ,0x1c, 0xd1};

//    // Pong "Rock it with HTML5 WebSocket"
//    unsigned char webSocketMessage[] = {0x8a, 0x9c, 0x37, 0x22, 0x79, 0xa5, 0x65, 0x4d, 0x1a, 0xce, 0x17, 0x4b, 0x0d, 0x85, 0x40, 0x4b
//    ,0x0d, 0xcd, 0x17, 0x6a, 0x2d, 0xe8, 0x7b, 0x17, 0x59, 0xf2, 0x52, 0x40, 0x2a, 0xca, 0x54, 0x49
//    ,0x1c, 0xd1};

    // time this!
    int messages = 1000000;
    size_t bufferLength = sizeof(webSocketMessage) * messages;
    char *buffer = new char[bufferLength + 4];
    char *originalBuffer = new char[bufferLength];
    for (int i = 0; i < messages; i++) {
        memcpy(originalBuffer + sizeof(webSocketMessage) * i, webSocketMessage, sizeof(webSocketMessage));
    }

    uWS::Hub h;

    h.onConnection([originalBuffer, buffer, bufferLength, messages, &h](uWS::WebSocket<uWS::SERVER> *ws, uWS::HttpRequest req) {
        for (int i = 0; i < 100; i++) {
            memcpy(buffer, originalBuffer, bufferLength);

            auto now = std::chrono::high_resolution_clock::now();
            uWS::WebSocketProtocol<uWS::SERVER, uWS::WebSocket<uWS::SERVER>>::consume(buffer, bufferLength, ws);
            int us = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - now).count();

            std::cout << "Messages per microsecond: " << (double(messages) / double(us)) << std::endl;
        }

        h.getDefaultGroup<uWS::SERVER>().close();
    });

    h.onMessage([](uWS::WebSocket<uWS::SERVER> *ws, char *message, size_t length, uWS::OpCode opCode) {

    });

    h.listen(3000);
    h.connect("ws://localhost:3000", nullptr);
    h.run();

    delete [] buffer;
    delete [] originalBuffer;
}

void testAsync() {
    uWS::Hub h;
    uS::Async *a = new uS::Async(h.getLoop());

    a->start([](uS::Async *a) {
        a->close();
    });

    std::thread t([&h]() {
        h.run();
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    a->send();
    t.join();
    std::cout << "Falling through Async test" << std::endl;
}


int main(int argc, char *argv[])
{
    //serveEventSource();
    //serveHttp();
    //serveBenchmark();

    // These will run on Travis OS X
    //testReceivePerformance();
    testHTTP();

    //testAutobahn();
}
