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
    ACK_PLAYERS_CREATED = 19
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
            << position.x << "," << position.y << ":"
            << velocity.x << "," << velocity.y;
        return ss.str();
    }

    static MovementPacket Deserialize(const std::string& content) {
        MovementPacket packet;
        std::istringstream ss(content);
        std::string segment;

        std::getline(ss, segment, ':'); packet.matchID = std::stoi(segment);
        std::getline(ss, segment, ':'); packet.playerID = std::stoi(segment);
        std::getline(ss, segment, ':'); packet.tick = std::stoi(segment);
        std::getline(ss, segment, ':');
        sscanf_s(segment.c_str(), "%f,%f", &packet.position.x, &packet.position.y);
        std::getline(ss, segment, ':');
        sscanf_s(segment.c_str(), "%f,%f", &packet.velocity.x, &packet.velocity.y);

        return packet;
    }
};

struct CreatePlayerPacket {
    unsigned int playerID;
    sf::Vector2f spawnPosition;

    static CreatePlayerPacket Deserialize(const std::string& msg) {
        std::istringstream ss(msg);
        std::string token;
        std::vector<std::string> tokens;

        while (std::getline(ss, token, ':')) {
            tokens.push_back(token);
        }

        if (tokens.size() != 3) {
            throw std::runtime_error("Invalid CREATE_PLAYER packet format: " + msg);
        }

        CreatePlayerPacket packet;
        packet.playerID = std::stoul(tokens[0]);
        packet.spawnPosition.x = std::stof(tokens[1]);
        packet.spawnPosition.y = std::stof(tokens[2]);

        return packet;
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

