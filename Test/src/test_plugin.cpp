// enrico.gamberini@cern.ch

#include <dlfcn.h>
#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include <zmq.hpp>

#include "BoardReader.hpp"
#include "EventBuilder.hpp"
#include "../../runcontrol/message.hpp"

int main(int argc, char **argv)
{
    zmq::context_t context(1);
    zmq::socket_t socket(context, ZMQ_REP);
    socket.bind("tcp://*:5555");
    zmq::message_t request;
    socket.recv(&request);

    msgpack::object_handle unpacked_body;
    unpacked_body = msgpack::unpack(static_cast<const char *>(request.data()), request.size());
    configurationMsg_t msg;
    unpacked_body.get().convert(msg);

    std::cout << "Tag " << msg.tag << std::endl;
    std::cout << "Command " << msg.command << std::endl;
    std::cout << "Type " << msg.type << std::endl;

    // reply
    configurationMsg_t msg2;
    msg2.tag = "configuration";
    msg2.command = "configure";
    msg2.type = "BoardReader";

    msgpack::sbuffer packed;
    msgpack::pack(&packed, msg2);

    zmq::message_t zmsg(packed.size());
    std::memcpy(zmsg.data(), packed.data(), packed.size());

    socket.send(zmsg);

    return 0;

    if (argc == 1)
    {
        std::cout << "No plugin name entered" << std::endl;
        return 1;
    }
    std::cout << "Loading " << argv[1] << std::endl;
    std::string pluginName = "./lib" + (std::string)argv[1] + ".so";
    void *handle = dlopen(pluginName.c_str(), RTLD_LAZY);
    if (handle == 0)
    {
        std::cout << "Plugin name not valid" << std::endl;
        return 1;
    }

    BaseClass *(*create)();
    void (*destroy)(BaseClass *);

    create = (BaseClass * (*)()) dlsym(handle, "create_object");
    destroy = (void (*)(BaseClass *))dlsym(handle, "destroy_object");

    BaseClass *bc = (BaseClass *)create();

    bc->start();
    std::this_thread::sleep_for(std::chrono::seconds(10));
    bc->stop();
    destroy(bc);
}
