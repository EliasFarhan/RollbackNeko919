#pragma once

#include <queue>
#include "SFML/Network.hpp"
#include "asteroid/game.h"
#include "asteroid/packet_type.h"
#include "asteroid/game_manager.h"

namespace neko::net
{
struct ClientInfo
{
    ClientId clientId = 0;
    unsigned long long timeDifference = 0;
    sf::IpAddress udpRemoteAddress;
    unsigned short udpRemotePort = 0;


};
class ServerNetworkManager : public asteroid::PacketSenderInterface, public SystemInterface
{
public:
    enum class PacketSocketSource
    {
        TCP,
        UDP
    };
    void SendReliablePacket(std::unique_ptr<asteroid::Packet> packet) override;

    void SendUnreliablePacket(std::unique_ptr<asteroid::Packet> packet) override;

    void Init() override;

    void Update(seconds dt) override;

    void Destroy() override;

    void SetTcpPort(unsigned short i);

    bool IsOpen();

private:
    void ProcessReceivePacket(std::unique_ptr<asteroid::Packet> packet, 
        PacketSocketSource packetSource, 
        sf::IpAddress address = "localhost", 
        unsigned short port = 0);
    void ReceivePacket(sf::Packet& packet, PacketSocketSource packetSource,
        sf::IpAddress address = "localhost",
        unsigned short port = 0);
    enum ServerStatus
    {
        OPEN = 1u << 0u,
        STARTED = 1u << 1u,
        FIRST_PLAYER_CONNECT = 1u << 2u,
    };
    sf::UdpSocket udpSocket_;
    sf::TcpListener tcpListener_;
    std::array<sf::TcpSocket, asteroid::maxPlayerNmb> tcpSockets_;

    std::array<ClientInfo, asteroid::maxPlayerNmb> clientMap_{};
    //Server game manager
    asteroid::GameManager gameManager_;

    unsigned short tcpPort_ = 12345;
    unsigned short udpPort_ = 12345;
    Index lastSocketIndex_ = 0;
    PlayerNumber lastPlayerNumber_ = 0;
    std::uint8_t status_ = 0;
};
}