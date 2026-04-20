#ifndef PARSING_UTILS_HPP
#define PARSING_UTILS_HPP

#include <string>
#include <vector>
#include <sstream>

/**
 * Utilitaires de parsing pour IRC
 * Fonctions réutilisables et robustes
 */
namespace ParsingUtils {

    // Supprime les whitespaces au début et à la fin
    std::string trim(const std::string& str);
    
    // Supprime le caractère '\r' à la fin d'une chaîne
    std::string chomp(const std::string& str);
    
    // Vérifie si une chaîne est un canal valide (commence par #, &, +, !)
    bool isValidChannelName(const std::string& name);
    
    // Vérifie si un nickname est valide (caractères autorisés)
    bool isValidNickname(const std::string& nickname);
    
    // Vérifie si une chaîne est un nombre
    bool isNumber(const std::string& str);
    
    // Convertit une chaîne en entier (avec gestion d'erreur)
    int toInt(const std::string& str, bool& ok);
    
    // Split une chaîne avec un délimiteur
    std::vector<std::string> split(const std::string& str, char delimiter);
    
    // Split une chaîne en limitant le nombre de morceaux
    std::vector<std::string> split(const std::string& str, char delimiter, size_t max_parts);
    
    // Extrait le nickname d'un prefix IRC (format: nick!user@host)
    std::string extractNicknameFromPrefix(const std::string& prefix);
    
    // Extrait le username d'un prefix IRC
    std::string extractUsernameFromPrefix(const std::string& prefix);
    
    // Extrait le host d'un prefix IRC
    std::string extractHostFromPrefix(const std::string& prefix);
    
    // Construit un prefix IRC à partir des composants
    std::string buildPrefix(const std::string& nickname, 
                           const std::string& username, 
                           const std::string& host);
    
    // Échappe les caractères spéciaux dans un message
    std::string escapeMessage(const std::string& msg);
    
    // Vérifie si une chaîne contient des caractères non imprimables
    bool hasNonPrintable(const std::string& str);
    
    // Nettoie une chaîne en supprimant les caractères non autorisés
    std::string sanitize(const std::string& str);
}

#endif
