#pragma once

#include <cstdint>
#include <cstring>
#include <string>
#include <unistd.h>

namespace network
{

// ── Binary I/O Helpers ────────────────────────────────────────────────
// All multi-byte integers are big-endian (network byte order).
// These functions operate on raw file descriptors.

inline ssize_t ReadBytes(int fd, void* buf, size_t len)
{
    size_t total = 0;
    char*  ptr   = static_cast<char*>(buf);
    while (total < len)
    {
        ssize_t n = read(fd, ptr + total, len - total);
        if (n <= 0) return n; // 0 = closed, -1 = error
        total += static_cast<size_t>(n);
    }
    return static_cast<ssize_t>(total);
}

inline ssize_t WriteBytes(int fd, const void* buf, size_t len)
{
    const char* ptr = static_cast<const char*>(buf);
    size_t      total = 0;
    while (total < len)
    {
        ssize_t n = write(fd, ptr + total, len - total);
        if (n <= 0) return -1;
        total += static_cast<size_t>(n);
    }
    return static_cast<ssize_t>(total);
}

// ── Read/Write uint16_t (big-endian) ─────────────────────────────────

inline bool ReadUint16(int fd, uint16_t& value)
{
    uint8_t buf[2];
    if (ReadBytes(fd, buf, 2) != 2) return false;
    value = static_cast<uint16_t>(buf[0]) << 8 | buf[1];
    return true;
}

inline bool WriteUint16(int fd, uint16_t value)
{
    uint8_t buf[2];
    buf[0] = static_cast<uint8_t>(value >> 8);
    buf[1] = static_cast<uint8_t>(value & 0xFF);
    return WriteBytes(fd, buf, 2) == 2;
}

// ── Read/Write uint8_t ───────────────────────────────────────────────

inline bool ReadUint8(int fd, uint8_t& value)
{
    uint8_t buf;
    if (ReadBytes(fd, &buf, 1) != 1) return false;
    value = buf;
    return true;
}

inline bool WriteUint8(int fd, uint8_t value)
{
    return WriteBytes(fd, &value, 1) == 1;
}

// ── Read/Write length-prefixed string ────────────────────────────────
// Format: [Length:2][Data:Length]

inline bool ReadString(int fd, std::string& str, uint16_t maxLen)
{
    uint16_t len = 0;
    if (!ReadUint16(fd, len)) return false;
    if (len > maxLen) return false;
    str.resize(len);
    if (len > 0 && ReadBytes(fd, &str[0], len) != static_cast<ssize_t>(len))
        return false;
    return true;
}

inline bool WriteString(int fd, const std::string& str)
{
    if (!WriteUint16(fd, static_cast<uint16_t>(str.size()))) return false;
    if (str.size() > 0 && WriteBytes(fd, str.data(), str.size()) !=
                            static_cast<ssize_t>(str.size()))
        return false;
    return true;
}

// ── Read/Write fixed-size char array ─────────────────────────────────
// Used for PawnLocator (4 bytes), SquareCode (2 bytes), etc.

inline bool ReadFixedArray(int fd, char* buf, size_t len)
{
    return ReadBytes(fd, buf, len) == static_cast<ssize_t>(len);
}

inline bool WriteFixedArray(int fd, const char* buf, size_t len)
{
    return WriteBytes(fd, buf, len) == static_cast<ssize_t>(len);
}

// ── Read/Write PID ───────────────────────────────────────────────────

inline bool ReadPid(int fd, uint16_t& pid)
{
    return ReadUint16(fd, pid);
}

inline bool WritePid(int fd, uint16_t pid)
{
    return WriteUint16(fd, pid);
}

} // namespace network
