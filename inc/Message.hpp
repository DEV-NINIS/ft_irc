#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include <string>
#include <vector>

/**
 * Classe représentant un message IRC parsé
 * Format: [":" prefix " "] command [" " params] [":" trailing]
 * 
 * Conforme à la RFC 1459
 * 
 * Exemples:
 *   "JOIN #channel"
 *   ":nick!user@host PRIVMSG #channel :Hello world"
 *   "MODE #channel +i"
 *   "PRIVMSG nick :"
 *   "   PRIVMSG nick :hello"  (espaces au début)
 */
class Message {
private:
    std::string _prefix;           
    std::string _command;          
    std::vector<std::string> _params;  
    std::string _trailing;         
    bool _valid;                   
    std::string _error_msg;        
    std::string _raw_line;         

public:
    Message();
    explicit Message(const std::string& raw_line);
    
    static Message parse(const std::string& line);
    
    // Getters
    const std::string& getPrefix() const;
    const std::string& getCommand() const;
    const std::vector<std::string>& getParams() const;
    const std::string& getTrailing() const;
    bool isValid() const;
    const std::string& getErrorMsg() const;
    const std::string& getRawLine() const;
    
    // Utilitaires
    size_t paramCount() const;
    bool hasParam(size_t index) const;
    std::string getParam(size_t index) const;
    std::string getParamSafe(size_t index) const;
    
    // Pour les commandes spécifiques
    std::string getChannel() const;
    std::string getTarget() const;
    std::string getMessage() const;
    std::string getKey() const;
    
    // Normalisation
    void toUpperCase();
    
    // Debug
    void print() const;
    std::string toString() const;
};

#endif
