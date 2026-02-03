#include <cstdint>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

namespace backend {

struct User {
    std::string cnic;
    std::string password;
    bool voted = false;
    int votedFor = -1;
};

inline constexpr int kMaxUsers = 100;
inline constexpr int kCandidateCount = 3;
inline constexpr const char *kCandidates[kCandidateCount] = {
    "Candidate A",
    "Candidate B",
    "Candidate C"
};

inline const std::string kEncryptedDataFile = "voting_data/data_encrypted.txt";
inline const std::string kDecryptedDataFile = "voting_data/data_decrypted.txt";
inline const std::string kAdminPassword = "admin123";

std::string HashPassword(const std::string &password, const std::string &cnic);

namespace {

bool IsDigits(const std::string &value) {
    for (char ch : value) {
        if (ch < '0' || ch > '9') {
            return false;
        }
    }
    return true;
}

uint64_t Fnv1aHash(const std::string &value) {
    const uint64_t kOffsetBasis = 1469598103934665603ULL;
    const uint64_t kPrime = 1099511628211ULL;
    uint64_t hash = kOffsetBasis;
    for (unsigned char ch : value) {
        hash ^= ch;
        hash *= kPrime;
    }
    return hash;
}

std::string ToHex(uint64_t value) {
    std::stringstream ss;
    ss << std::hex << std::setw(16) << std::setfill('0') << value;
    return ss.str();
}

bool IsHexChar(char ch) {
    return (ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'f') || (ch >= 'A' && ch <= 'F');
}

bool LooksLikeHex(const std::string &value) {
    if (value.empty() || (value.size() % 2 != 0)) {
        return false;
    }
    for (char ch : value) {
        if (!IsHexChar(ch)) {
            return false;
        }
    }
    return true;
}

std::string XorCipher(const std::string &input, const std::string &key) {
    if (key.empty()) {
        return input;
    }
    std::string output = input;
    for (size_t i = 0; i < input.size(); ++i) {
        output[i] = static_cast<char>(input[i] ^ key[i % key.size()]);
    }
    return output;
}

std::string ToHexString(const std::string &data) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (unsigned char ch : data) {
        ss << std::setw(2) << static_cast<int>(ch);
    }
    return ss.str();
}

bool FromHexString(const std::string &hexInput, std::string &output) {
    if (!LooksLikeHex(hexInput)) {
        return false;
    }
    output.clear();
    for (size_t i = 0; i < hexInput.size(); i += 2) {
        std::string byteString = hexInput.substr(i, 2);
        char byte = static_cast<char>(std::stoi(byteString, nullptr, 16));
        output.push_back(byte);
    }
    return true;
}

std::string SerializeUsers(const std::vector<User> &users) {
    std::stringstream ss;
    for (const auto &user : users) {
        ss << user.cnic << "|"
           << user.password << "|"
           << (user.voted ? 1 : 0) << "|"
           << user.votedFor << "\n";
    }
    return ss.str();
}

void DeserializeUsers(const std::string &data, std::vector<User> &users, std::vector<int> &voteCounts) {
    users.clear();
    voteCounts.assign(kCandidateCount, 0);

    std::stringstream ss(data);
    std::string line;
    while (std::getline(ss, line)) {
        if (line.empty()) {
            continue;
        }

        size_t p1 = line.find('|');
        size_t p2 = line.find('|', p1 + 1);
        size_t p3 = line.find('|', p2 + 1);

        if (p1 == std::string::npos || p2 == std::string::npos || p3 == std::string::npos) {
            continue;
        }

        User user;
        user.cnic = line.substr(0, p1);
        user.password = line.substr(p1 + 1, p2 - p1 - 1);
        user.voted = (line.substr(p2 + 1, p3 - p2 - 1) == "1");
        user.votedFor = std::stoi(line.substr(p3 + 1));

        if (!LooksLikeHex(user.password) || user.password.size() != 16) {
            user.password = HashPassword(user.password, user.cnic);
        }

        users.push_back(user);

        if (user.votedFor >= 0 && user.votedFor < kCandidateCount) {
            voteCounts[user.votedFor] += 1;
        }
    }
}

}  // namespace

bool IsValidCnic(const std::string &cnic) {
    return cnic.size() == 13 && IsDigits(cnic);
}

bool FindUserIndex(const std::vector<User> &users, const std::string &cnic, int &indexOut) {
    for (size_t i = 0; i < users.size(); ++i) {
        if (users[i].cnic == cnic) {
            indexOut = static_cast<int>(i);
            return true;
        }
    }
    return false;
}

std::string HashPassword(const std::string &password, const std::string &cnic) {
    return ToHex(Fnv1aHash(cnic + ":" + password));
}

void SaveData(const std::vector<User> &users) {
    std::string plain = SerializeUsers(users);
    std::string encrypted = XorCipher(plain, kAdminPassword);
    std::string hexOutput = ToHexString(encrypted);

    std::ofstream encryptedOut(kEncryptedDataFile.c_str());
    if (encryptedOut) {
        encryptedOut << hexOutput;
    }

    std::ofstream decryptedOut(kDecryptedDataFile.c_str());
    if (decryptedOut) {
        decryptedOut << plain;
    }
}

void LoadData(std::vector<User> &users, std::vector<int> &voteCounts) {
    std::ifstream in(kEncryptedDataFile.c_str());
    if (!in) {
        return;
    }

    std::string fileContents((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    if (fileContents.empty()) {
        return;
    }

    std::string decoded;
    if (FromHexString(fileContents, decoded)) {
        std::string decrypted = XorCipher(decoded, kAdminPassword);
        DeserializeUsers(decrypted, users, voteCounts);

        std::ofstream decryptedOut(kDecryptedDataFile.c_str());
        if (decryptedOut) {
            decryptedOut << decrypted;
        }
    } else {
        DeserializeUsers(fileContents, users, voteCounts);

        std::ofstream decryptedOut(kDecryptedDataFile.c_str());
        if (decryptedOut) {
            decryptedOut << fileContents;
        }
    }
}

}  // namespace backend
