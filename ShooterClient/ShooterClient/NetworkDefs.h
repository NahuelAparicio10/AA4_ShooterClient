#pragma once
#include <SFML/Network.hpp>
#include <cstdint>
#include <string>
#include <sstream>
enum PacketHeader : uint8_t {
    NORMAL = 0b00000001,
    CRITIC = 0b00000010,
    URGENT = 0b00000100
};

enum class PacketType : uint8_t {
    INVALID = 0,
    FIND_MATCH = 1,
    ACK_MATCH_FOUND = 2,
    CANCEL_SEARCH = 3,
    MATCH_FOUND = 4,
    VERSION = 5,
    UPDATE = 6,
    OK = 7,
    UPDATE_MAP = 8,
    SEARCH_ACK = 9,
    JOIN_GAME = 10,
    CREATE_MATCH = 11,
    MATCH_UNIQUE = 12,
    MATCH_USED = 13,
    PLAYER_MOVEMENT = 14,
    RECONCILE = 15,
    ACK_JOINED = 16,
    ACK_MATCH_CREATED = 17,
    CREATE_PLAYER = 18,
    ACK_PLAYERS_CREATED = 19,
    SHOOT_BULLET = 20,
    CREATE_BULLET = 21,
    DESTROY_BULLET = 22,
    PING = 23,
    MATCH_FINISHED = 24
};

struct CreateBulletPacket
{
    unsigned int shooterID;
    unsigned int bulletID;
    sf::Vector2f position;
    sf::Vector2f direction;

    std::string Serialize() const {
        return std::to_string(shooterID) + ":" +
            std::to_string(bulletID) + ":" +
            std::to_string(position.x) + ":" +
            std::to_string(position.y) + ":" +
            std::to_string(direction.x) + ":" +
            std::to_string(direction.y);
    }

    static CreateBulletPacket Deserialize(const std::string& data) {
        std::istringstream ss(data);
        CreateBulletPacket p;
        std::string value;
        std::getline(ss, value, ':'); p.shooterID = std::stoul(value);
        std::getline(ss, value, ':'); p.bulletID = std::stoul(value);
        std::getline(ss, value, ':'); p.position.x = std::stof(value);
        std::getline(ss, value, ':'); p.position.y = std::stof(value);
        std::getline(ss, value, ':'); p.direction.x = std::stof(value);
        std::getline(ss, value, ':'); p.direction.y = std::stof(value);
        return p;
    }
};

struct ShootBulletPacket
{
    unsigned int matchID;
    unsigned int playerID;
    sf::Vector2f position;
    sf::Vector2f direction;

    std::string Serialize() const {
        return std::to_string(matchID) + ":" +
            std::to_string(playerID) + ":" +
            std::to_string(position.x) + ":" +
            std::to_string(position.y) + ":" +
            std::to_string(direction.x) + ":" +
            std::to_string(direction.y);
    }

    static ShootBulletPacket Deserialize(const std::string& data) {
        std::istringstream ss(data);
        ShootBulletPacket p;
        std::string value;
        std::getline(ss, value, ':'); p.matchID = std::stoul(value);
        std::getline(ss, value, ':'); p.playerID = std::stoul(value);
        std::getline(ss, value, ':'); p.position.x = std::stof(value);
        std::getline(ss, value, ':'); p.position.y = std::stof(value);
        std::getline(ss, value, ':'); p.direction.x = std::stof(value);
        std::getline(ss, value, ':'); p.direction.y = std::stof(value);
        return p;
    }
};

struct InterpolationData {
    sf::Vector2f previous;
    sf::Vector2f current;
    float timer = 0.f;
};

struct MovementPacket {
    uint32_t matchID;
    uint32_t playerID;
    uint32_t tick;
    sf::Vector2f position;
    sf::Vector2f velocity;

    std::string Serialize() const {
        std::ostringstream ss;
        ss << matchID << ":" << playerID << ":" << tick << ":"
            << position.x << ":" << position.y << ":"
            << velocity.x << ":" << velocity.y;
        return ss.str();
    }

    static MovementPacket Deserialize(const std::string& content) {
        MovementPacket packet;
        std::istringstream ss(content);
        std::string segment;

        std::getline(ss, segment, ':'); packet.matchID = std::stoi(segment);
        std::getline(ss, segment, ':'); packet.playerID = std::stoi(segment);
        std::getline(ss, segment, ':'); packet.tick = std::stoi(segment);
        std::getline(ss, segment, ':'); packet.position.x = std::stoi(segment);
        std::getline(ss, segment, ':'); packet.position.y = std::stoi(segment);
        std::getline(ss, segment, ':'); packet.velocity.x = std::stoi(segment);
        std::getline(ss, segment, ':'); packet.velocity.y = std::stoi(segment);

        return packet;
    }
};

struct CreatePlayerPacket {
    unsigned int playerID;
    sf::Vector2f spawnPosition;

    static std::vector<CreatePlayerPacket> Deserialize(const std::string& msg, std::vector<CreatePlayerPacket> playerPacket) 
    {      
        
        std::istringstream ss(msg);
        std::string playerData;

        while (std::getline(ss, playerData, '|'))
        {
            std::stringstream ps(playerData);
            std::string idStr, xStr, yStr;

            std::getline(ps, idStr, ':');
            std::getline(ps, xStr, ':');
            std::getline(ps, yStr, ':');

            unsigned int id = std::stoul(idStr);
            float x = std::stof(xStr);
            float y = std::stof(yStr);

            CreatePlayerPacket packet{ id, { x, y } };

            playerPacket.push_back(packet);
        }

        return playerPacket;
    }

};

struct RawPacketJob {
    uint8_t headerMask;
    PacketType type;
    std::string content;
    std::optional<sf::IpAddress> sender;
    unsigned short port;
};

// Funci�n para crear un datagrama listo para enviar
inline std::size_t CreateRawDatagram(uint8_t headerMask, PacketType type, const std::string& content, char* outBuffer)
{
    outBuffer[0] = headerMask;
    outBuffer[1] = static_cast<uint8_t>(type);
    std::memcpy(outBuffer + 2, content.data(), content.size());
    return 2 + content.size();
}

inline void SendDatagram(sf::UdpSocket& socket, PacketHeader header, PacketType type, const std::string& content, const sf::IpAddress& ip, unsigned short port)
{
    char buffer[1024];
    std::size_t size = CreateRawDatagram(static_cast<uint8_t>(header), type, content, buffer);
    socket.send(buffer, size, ip, port);
}

// Funci�n para parsear datagramas binarios
inline bool ParseRawDatagram(const char* data, std::size_t size, RawPacketJob& out, const sf::IpAddress& ip, unsigned short port)
{
    if (size < 2) return false;

    out.headerMask = static_cast<uint8_t>(data[0]);
    out.type = static_cast<PacketType>(data[1]);

    if (out.type == PacketType::INVALID) return false;

    out.content = std::string(data + 2, size - 2);
    out.sender = ip;
    out.port = port;

    return true;
}

