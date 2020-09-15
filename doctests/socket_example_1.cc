const uint16_t portnum = ((std::random_device()()) % 50000) + 1025;

// create a UDP socket and bind it to a local address
// std::cout << "socket_dt 0" << std::endl;
UDPSocket sock1{};
// std::cout << "socket_dt 1" << std::endl;
sock1.bind(Address("127.0.0.1", portnum));
// std::cout << "socket_dt 2" << std::endl;

// create another UDP socket and send a datagram to the first socket without connecting
UDPSocket sock2{};
// std::cout << "socket_dt 3" << std::endl;
sock2.sendto(Address("127.0.0.1", portnum), "hi there");
// std::cout << "socket_dt 4" << std::endl;

// receive sent datagram, connect the socket to the peer's address, and send a response
auto recvd = sock1.recv();
// std::cout << "socket_dt 5" << std::endl;
std::cout << recvd.payload << std::endl;
sock1.connect(recvd.source_address);
sock1.send("hi yourself");

auto recvd2 = sock2.recv();
std::cout << recvd2.payload << std::endl;

if (recvd.payload != "hi there" || recvd2.payload != "hi yourself") {
    throw std::runtime_error("wrong data received");
}
