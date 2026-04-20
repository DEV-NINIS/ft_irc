#include "ParsingUtils.hpp"
#include <cctype>
#include <algorithm>
#include <cstdlib>

namespace ParsingUtils {

// ============================================================
// Trim - Supprime les espaces au début et à la fin
// ============================================================
std::string trim(const std::string& str) {
    size_t start = 0;
    size_t end = str.length();
    
    while (start < end && std::isspace(static_cast<unsigned char>(str[start]))) {
        start++;
    }
    
    while (end > start && std::isspace(static_cast<unsigned char>(str[end - 1]))) {
        end--;
    }
    
    return str.substr(start, end - start);
}

// ============================================================
// Chomp - Supprime le caractère '\r' à la fin
// ============================================================
std::string chomp(const std::string& str) {
    if (str.empty()) {
        return str;
    }
    
    size_t len = str.length();
    if (str[len - 1] == '\r') {
        return str.substr(0, len - 1);
    }
    return str;
}

// ============================================================
// Validation des noms de canaux
// Caractères autorisés: # & + ! suivi de lettres, chiffres, et certains symboles
// ============================================================
bool isValidChannelName(const std::string& name) {
    if (name.empty()) {
        return false;
    }
    
    // Un canal doit commencer par un des ces préfixes
    char first = name[0];
    if (first != '#' && first != '&' && first != '+' && first != '!') {
        return false;
    }
    
    // Longueur max généralement 50 caractères
    if (name.length() > 50) {
        return false;
    }
    
    // Vérifier les caractères autorisés
    for (size_t i = 0; i < name.length(); ++i) {
        char c = name[i];
        if (c == ' ' || c == ',' || c == ':' || c == '\r' || c == '\n' || c == '\0') {
            return false;
        }
        // Caractères spéciaux autorisés: -, _, [, ], {, }, \, |, ^
        if (std::isalnum(static_cast<unsigned char>(c)) || 
            c == '-' || c == '_' || c == '[' || c == ']' ||
            c == '{' || c == '}' || c == '\\' || c == '|' || c == '^') {
            continue;
        }
        return false;
    }
    
    return true;
}

// ============================================================
// Validation des nicknames
// Format: lettres, chiffres, et certains caractères spéciaux
// ============================================================
bool isValidNickname(const std::string& nickname) {
    if (nickname.empty()) {
        return false;
    }
    
    // Longueur max généralement 9-30 caractères selon les serveurs
    if (nickname.length() > 30) {
        return false;
    }
    
    // Un nickname ne peut pas commencer par un chiffre (selon certaines RFC)
    if (std::isdigit(static_cast<unsigned char>(nickname[0]))) {
        return false;
    }
    
    // Caractères autorisés dans un nickname
    for (size_t i = 0; i < nickname.length(); ++i) {
        char c = nickname[i];
        if (std::isalnum(static_cast<unsigned char>(c)) || 
            c == '-' || c == '_' || c == '[' || c == ']' ||
            c == '{' || c == '}' || c == '\\' || c == '|' || c == '^') {
            continue;
        }
        return false;
    }
    
    return true;
}

// ============================================================
// Vérifie si une chaîne est un nombre
// ============================================================
bool isNumber(const std::string& str) {
    if (str.empty()) {
        return false;
    }
    
    for (size_t i = 0; i < str.length(); ++i) {
        if (!std::isdigit(static_cast<unsigned char>(str[i]))) {
            return false;
        }
    }
    
    return true;
}

// ============================================================
// Conversion string -> int avec gestion d'erreur
// ============================================================
int toInt(const std::string& str, bool& ok) {
    ok = true;
    
    if (!isNumber(str)) {
        ok = false;
        return 0;
    }
    
    // Éviter les overflow simples
    if (str.length() > 10) {
        ok = false;
        return 0;
    }
    
    char* endptr = NULL;
    long result = std::strtol(str.c_str(), &endptr, 10);
    
    if (*endptr != '\0') {
        ok = false;
        return 0;
    }
    
    if (result < 0 || result > 2147483647) {
        ok = false;
        return 0;
    }
    
    return static_cast<int>(result);
}

// ============================================================
// Split avec un délimiteur
// ============================================================
std::vector<std::string> split(const std::string& str, char delimiter) {
    std::vector<std::string> result;
    std::stringstream ss(str);
    std::string item;
    
    while (std::getline(ss, item, delimiter)) {
        result.push_back(item);
    }
    
    return result;
}

// ============================================================
// Split avec limite de morceaux
// ============================================================
std::vector<std::string> split(const std::string& str, char delimiter, size_t max_parts) {
    std::vector<std::string> result;
    std::stringstream ss(str);
    std::string item;
    size_t count = 0;
    
    while (count < max_parts - 1 && std::getline(ss, item, delimiter)) {
        result.push_back(item);
        count++;
    }
    
    if (count < max_parts) {
        // Récupérer le reste de la chaîne
        std::string remaining;
        if (std::getline(ss, remaining, '\0')) {
            result.push_back(remaining);
        }
    }
    
    return result;
}

// ============================================================
// Extraction du nickname depuis un prefix (format: nick!user@host)
// ============================================================
std::string extractNicknameFromPrefix(const std::string& prefix) {
    size_t pos = prefix.find('!');
    if (pos == std::string::npos) {
        return prefix;
    }
    return prefix.substr(0, pos);
}

// ============================================================
// Extraction du username depuis un prefix
// ============================================================
std::string extractUsernameFromPrefix(const std::string& prefix) {
    size_t bang = prefix.find('!');
    size_t at = prefix.find('@');
    
    if (bang == std::string::npos || at == std::string::npos || bang + 1 >= at) {
        return "";
    }
    
    return prefix.substr(bang + 1, at - bang - 1);
}

// ============================================================
// Extraction du host depuis un prefix
// ============================================================
std::string extractHostFromPrefix(const std::string& prefix) {
    size_t pos = prefix.find('@');
    if (pos == std::string::npos || pos + 1 >= prefix.length()) {
        return "";
    }
    return prefix.substr(pos + 1);
}

// ============================================================
// Construction d'un prefix IRC
// ============================================================
std::string buildPrefix(const std::string& nickname, 
                        const std::string& username, 
                        const std::string& host) {
    std::string result = nickname;
    if (!username.empty()) {
        result += "!" + username;
    }
    if (!host.empty()) {
        result += "@" + host;
    }
    return result;
}

// ============================================================
// Échappe les caractères spéciaux dans un message
// ============================================================
std::string escapeMessage(const std::string& msg) {
    std::string result;
    for (size_t i = 0; i < msg.length(); ++i) {
        if (msg[i] == '\r') {
            result += "\\r";
        } else if (msg[i] == '\n') {
            result += "\\n";
        } else if (msg[i] == '\0') {
            result += "\\0";
        } else {
            result += msg[i];
        }
    }
    return result;
}

// ============================================================
// Vérifie la présence de caractères non imprimables
// ============================================================
bool hasNonPrintable(const std::string& str) {
    for (size_t i = 0; i < str.length(); ++i) {
        unsigned char c = static_cast<unsigned char>(str[i]);
        if (c < 32 && c != '\r' && c != '\n') {
            return true;
        }
    }
    return false;
}

// ============================================================
// Nettoie une chaîne (supprime les caractères non autorisés)
// ============================================================
std::string sanitize(const std::string& str) {
    std::string result;
    for (size_t i = 0; i < str.length(); ++i) {
        unsigned char c = static_cast<unsigned char>(str[i]);
        // Garder les caractères imprimables (32-126) et quelques autres
        if (c >= 32 && c <= 126) {
            result += str[i];
        } else if (c == '\r' || c == '\n') {
            // Ignorer ces caractères
        } else {
            result += '?'; // Remplacer par '?'
        }
    }
    return result;
}

} // namespace ParsingUtils
