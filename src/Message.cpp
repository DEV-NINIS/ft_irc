#include "Message.hpp"
#include "ParsingUtils.hpp"
#include <iostream>
#include <cctype>
#include <algorithm>

// Constructeur par défaut
Message::Message() : _valid(false) {}

// Constructeur avec parsing direct
Message::Message(const std::string& raw_line) : _raw_line(raw_line) {
    *this = parse(raw_line);
}

Message Message::parse(const std::string& line) {
    Message msg;
    msg._raw_line = line;
    
    // ============================================================
    // 1. Nettoyage de base
    // ============================================================
    std::string clean = line;
    
    // Supprimer le \r final si présent
    if (!clean.empty() && clean[clean.size() - 1] == '\r') {
        clean.erase(clean.size() - 1);
    }
    
    // Supprimer les espaces au début (RFC permet les espaces avant un message)
    size_t start = 0;
    while (start < clean.size() && clean[start] == ' ') {
        start++;
    }
    
    if (start >= clean.size()) {
        msg._valid = false;
        msg._error_msg = "Empty line";
        return msg;
    }
    
    clean = clean.substr(start);
    
    // ============================================================
    // 2. Extraction du préfixe (commence par ':')
    // ============================================================
    size_t pos = 0;
    size_t len = clean.size();
    
    if (clean[pos] == ':') {
        pos++; // Skip ':'
        size_t end = clean.find(' ', pos);
        
        if (end == std::string::npos) {
            // Un message qui n'est qu'un préfixe est invalide
            msg._valid = false;
            msg._error_msg = "Prefix without command";
            return msg;
        }
        
        msg._prefix = clean.substr(pos, end - pos);
        pos = end + 1;
        
        // Ignorer les espaces supplémentaires après le préfixe
        while (pos < len && clean[pos] == ' ') {
            pos++;
        }
    }
    
    // ============================================================
    // 3. Extraction de la commande
    // ============================================================
    if (pos >= len) {
        msg._valid = false;
        msg._error_msg = "No command found";
        return msg;
    }
    
    size_t end = clean.find(' ', pos);
    if (end == std::string::npos) {
        // Pas de paramètres, juste la commande
        msg._command = clean.substr(pos);
        // Normaliser la commande en majuscules
        for (size_t i = 0; i < msg._command.size(); ++i) {
            msg._command[i] = std::toupper(msg._command[i]);
        }
        msg._valid = true;
        return msg;
    }
    
    msg._command = clean.substr(pos, end - pos);
    // Normaliser la commande en majuscules
    for (size_t i = 0; i < msg._command.size(); ++i) {
        msg._command[i] = std::toupper(msg._command[i]);
    }
    pos = end + 1;
    
    // Ignorer les espaces supplémentaires
    while (pos < len && clean[pos] == ' ') {
        pos++;
    }
    
    // ============================================================
    // 4. Extraction des paramètres
    // ============================================================
    // Limite de paramètres selon RFC (15 max)
    const size_t MAX_PARAMS = 15;
    bool has_trailing = false;
    
    while (pos < len && !has_trailing && msg._params.size() < MAX_PARAMS) {
        
        // Vérifier si c'est le début du trailing
        if (clean[pos] == ':') {
            // Le trailing peut être vide (ex: "PRIVMSG nick :")
            msg._trailing = clean.substr(pos + 1);
            has_trailing = true;
            break;
        }
        
        // Trouver la fin du paramètre courant
        end = clean.find(' ', pos);
        
        if (end == std::string::npos) {
            // Dernier paramètre
            std::string param = clean.substr(pos);
            if (!param.empty()) {
                msg._params.push_back(param);
            }
            break;
        }
        
        // Paramètre normal (peut être vide? Non, on ignore les vides consécutifs)
        if (end > pos) {
            std::string param = clean.substr(pos, end - pos);
            msg._params.push_back(param);
        }
        
        pos = end + 1;
        
        // Ignorer les espaces supplémentaires
        while (pos < len && clean[pos] == ' ') {
            pos++;
        }
    }
    
    // Si on a atteint la limite de paramètres, le reste est ignoré (selon RFC)
    
    msg._valid = true;
    return msg;
}

// ============================================================
// Getters
// ============================================================
const std::string& Message::getPrefix() const {
    return _prefix;
}

const std::string& Message::getCommand() const {
    return _command;
}

const std::vector<std::string>& Message::getParams() const {
    return _params;
}

const std::string& Message::getTrailing() const {
    return _trailing;
}

bool Message::isValid() const {
    return _valid;
}

const std::string& Message::getErrorMsg() const {
    return _error_msg;
}

const std::string& Message::getRawLine() const {
    return _raw_line;
}

// ============================================================
// Utilitaires
// ============================================================
size_t Message::paramCount() const {
    return _params.size();
}

bool Message::hasParam(size_t index) const {
    return index < _params.size();
}

std::string Message::getParam(size_t index) const {
    if (index < _params.size()) {
        return _params[index];
    }
    return "";
}

std::string Message::getParamSafe(size_t index) const {
    return getParam(index);
}

std::string Message::getChannel() const {
    if (_params.empty()) {
        return "";
    }
    const std::string& first = _params[0];
    if (!first.empty() && (first[0] == '#' || first[0] == '&' || 
        first[0] == '+' || first[0] == '!')) {
        return first;
    }
    return "";
}

std::string Message::getTarget() const {
    return getParam(0);
}

std::string Message::getMessage() const {
    if (!_trailing.empty()) {
        return _trailing;
    }
    if (!_params.empty()) {
        return _params.back();
    }
    return "";
}

std::string Message::getKey() const {
    // Pour la commande JOIN: le mot de passe est le 2ème paramètre
    if (_command == "JOIN" && _params.size() >= 2) {
        return _params[1];
    }
    // Pour la commande MODE +k: le mot de passe est après +k
    if (_command == "MODE" && _params.size() >= 3 && _params[1] == "+k") {
        return _params[2];
    }
    return "";
}

void Message::toUpperCase() {
    for (size_t i = 0; i < _command.size(); ++i) {
        _command[i] = std::toupper(_command[i]);
    }
}

// ============================================================
// Debug
// ============================================================
void Message::print() const {
    std::cout << "=== Message ===" << std::endl;
    std::cout << "Raw: " << _raw_line << std::endl;
    std::cout << "Valid: " << (_valid ? "yes" : "no") << std::endl;
    
    if (!_valid && !_error_msg.empty()) {
        std::cout << "Error: " << _error_msg << std::endl;
    }
    
    if (!_prefix.empty()) {
        std::cout << "Prefix: '" << _prefix << "'" << std::endl;
    }
    
    std::cout << "Command: '" << _command << "'" << std::endl;
    
    for (size_t i = 0; i < _params.size(); ++i) {
        std::cout << "Param[" << i << "]: '" << _params[i] << "'" << std::endl;
    }
    
    if (!_trailing.empty() || (_valid && _raw_line.find(":") != std::string::npos)) {
        std::cout << "Trailing: '" << _trailing << "'" << std::endl;
    }
    
    std::cout << "==============" << std::endl;
}

std::string Message::toString() const {
    std::string result;
    if (!_prefix.empty()) {
        result += ":" + _prefix + " ";
    }
    result += _command;
    
    for (size_t i = 0; i < _params.size(); ++i) {
        result += " " + _params[i];
    }
    
    if (!_trailing.empty()) {
        result += " :" + _trailing;
    }
    
    return result;
}
