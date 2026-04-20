#include "Message.hpp"
#include "ParsingUtils.hpp"
#include <iostream>
#include <string>

int tests_passed = 0;
int tests_failed = 0;

void assert_test(bool condition, const std::string& test_name) {
    if (condition) {
        std::cout << "  ✓ " << test_name << std::endl;
        tests_passed++;
    } else {
        std::cout << "  ✗ " << test_name << " FAILED" << std::endl;
        tests_failed++;
    }
}

void test_basic_parsing() {
    std::cout << "\n--- Basic Parsing Tests ---" << std::endl;
    
    Message m1 = Message::parse("JOIN #channel");
    assert_test(m1.isValid(), "JOIN #channel valid");
    assert_test(m1.getCommand() == "JOIN", "Command = JOIN");
    assert_test(m1.paramCount() == 1, "1 param");
    assert_test(m1.getParam(0) == "#channel", "Param[0] = #channel");
    
    Message m2 = Message::parse("PRIVMSG #channel :Hello world");
    assert_test(m2.isValid(), "PRIVMSG valid");
    assert_test(m2.getCommand() == "PRIVMSG", "Command = PRIVMSG");
    assert_test(m2.getParam(0) == "#channel", "Param[0] = #channel");
    assert_test(m2.getTrailing() == "Hello world", "Trailing = Hello world");
    
    Message m3 = Message::parse(":nick!user@host PRIVMSG #channel :Hello");
    assert_test(m3.isValid(), "With prefix valid");
    assert_test(m3.getPrefix() == "nick!user@host", "Prefix extracted");
    assert_test(m3.getCommand() == "PRIVMSG", "Command extracted");
}

void test_edge_cases() {
    std::cout << "\n--- Edge Cases Tests ---" << std::endl;
    
    // Message avec trailing vide
    Message m1 = Message::parse("PRIVMSG nick :");
    assert_test(m1.isValid(), "Empty trailing valid");
    assert_test(m1.getTrailing() == "", "Empty trailing is empty string");
    
    // Commandes en minuscules
    Message m2 = Message::parse("join #channel");
    assert_test(m2.isValid(), "Lowercase command valid");
    assert_test(m2.getCommand() == "JOIN", "Command normalized to uppercase");
    
    // Espaces au début
    Message m3 = Message::parse("   JOIN #channel");
    assert_test(m3.isValid(), "Leading spaces valid");
    assert_test(m3.getCommand() == "JOIN", "Command extracted after spaces");
    
    // Espaces multiples entre paramètres
    Message m4 = Message::parse("JOIN   #channel");
    assert_test(m4.isValid(), "Multiple spaces valid");
    assert_test(m4.getParam(0) == "#channel", "Param extracted correctly");
    
    // Message avec seulement un préfixe (invalide)
    Message m5 = Message::parse(":nick!user@host");
    assert_test(!m5.isValid(), "Prefix only invalid");
    
    // Ligne vide
    Message m6 = Message::parse("");
    assert_test(!m6.isValid(), "Empty line invalid");
    
    // Message avec \r final
    Message m7 = Message::parse("JOIN #channel\r");
    assert_test(m7.isValid(), "CR at end valid");
    assert_test(m7.getCommand() == "JOIN", "Command correct with CR");
}

void test_irc_commands() {
    std::cout << "\n--- IRC Commands Tests ---" << std::endl;
    
    // PASS
    Message m1 = Message::parse("PASS secret123");
    assert_test(m1.isValid() && m1.getCommand() == "PASS", "PASS command");
    assert_test(m1.getParam(0) == "secret123", "PASS param");
    
    // NICK
    Message m2 = Message::parse("NICK alice");
    assert_test(m2.isValid() && m2.getCommand() == "NICK", "NICK command");
    assert_test(m2.getParam(0) == "alice", "NICK param");
    
    // USER
    Message m3 = Message::parse("USER alice 0 * :Alice Dupont");
    assert_test(m3.isValid() && m3.getCommand() == "USER", "USER command");
    assert_test(m3.paramCount() == 4, "USER has 4 params");
    assert_test(m3.getTrailing() == "Alice Dupont", "USER realname");
    
    // JOIN
    Message m4 = Message::parse("JOIN #test key123");
    assert_test(m4.isValid() && m4.getCommand() == "JOIN", "JOIN command");
    assert_test(m4.getParam(0) == "#test", "JOIN channel");
    assert_test(m4.getKey() == "key123", "JOIN key");
    
    // KICK
    Message m5 = Message::parse("KICK #channel user :Spam");
    assert_test(m5.isValid() && m5.getCommand() == "KICK", "KICK command");
    assert_test(m5.getParam(0) == "#channel", "KICK channel");
    assert_test(m5.getParam(1) == "user", "KICK user");
    assert_test(m5.getTrailing() == "Spam", "KICK reason");
    
    // INVITE
    Message m6 = Message::parse("INVITE user #channel");
    assert_test(m6.isValid() && m6.getCommand() == "INVITE", "INVITE command");
    assert_test(m6.getParam(0) == "user", "INVITE user");
    assert_test(m6.getParam(1) == "#channel", "INVITE channel");
    
    // TOPIC
    Message m7 = Message::parse("TOPIC #channel :New topic");
    assert_test(m7.isValid() && m7.getCommand() == "TOPIC", "TOPIC command");
    assert_test(m7.getParam(0) == "#channel", "TOPIC channel");
    assert_test(m7.getTrailing() == "New topic", "TOPIC topic");
    
    // MODE
    Message m8 = Message::parse("MODE #channel +i");
    assert_test(m8.isValid() && m8.getCommand() == "MODE", "MODE command");
    
    // QUIT
    Message m9 = Message::parse("QUIT :Goodbye");
    assert_test(m9.isValid() && m9.getCommand() == "QUIT", "QUIT command");
}

void test_mode_parsing() {
    std::cout << "\n--- MODE Command Tests ---" << std::endl;
    
    // +i
    Message m1 = Message::parse("MODE #channel +i");
    assert_test(m1.getParam(0) == "#channel", "MODE channel");
    assert_test(m1.getParam(1) == "+i", "MODE +i");
    
    // -i
    Message m2 = Message::parse("MODE #channel -i");
    assert_test(m2.getParam(1) == "-i", "MODE -i");
    
    // +k with password
    Message m3 = Message::parse("MODE #channel +k secret");
    assert_test(m3.getParam(1) == "+k", "MODE +k");
    assert_test(m3.getParam(2) == "secret", "MODE password");
    
    // +o with nickname
    Message m4 = Message::parse("MODE #channel +o alice");
    assert_test(m4.getParam(1) == "+o", "MODE +o");
    assert_test(m4.getParam(2) == "alice", "MODE operator nick");
    
    // +l with limit
    Message m5 = Message::parse("MODE #channel +l 10");
    assert_test(m5.getParam(1) == "+l", "MODE +l");
    assert_test(m5.getParam(2) == "10", "MODE limit");
    
    // Combined modes
    Message m6 = Message::parse("MODE #channel +i-t +k secret +l 10");
    assert_test(m6.paramCount() >= 3, "Combined modes have params");
}

void test_prefix_extraction() {
    std::cout << "\n--- Prefix Extraction Tests ---" << std::endl;
    
    std::string prefix = "nick!user@host";
    assert_test(ParsingUtils::extractNicknameFromPrefix(prefix) == "nick", "Extract nickname");
    assert_test(ParsingUtils::extractUsernameFromPrefix(prefix) == "user", "Extract username");
    assert_test(ParsingUtils::extractHostFromPrefix(prefix) == "host", "Extract host");
    
    std::string prefix2 = "nick";
    assert_test(ParsingUtils::extractNicknameFromPrefix(prefix2) == "nick", "Nickname only");
    assert_test(ParsingUtils::extractUsernameFromPrefix(prefix2) == "", "No username");
    assert_test(ParsingUtils::extractHostFromPrefix(prefix2) == "", "No host");
    
    std::string prefix3 = "nick!user";
    assert_test(ParsingUtils::extractUsernameFromPrefix(prefix3) == "user", "Username without host");
    
    // Build prefix
    std::string built = ParsingUtils::buildPrefix("nick", "user", "host");
    assert_test(built == "nick!user@host", "Build prefix");
}

void test_validation_utils() {
    std::cout << "\n--- Validation Utils Tests ---" << std::endl;
    
    // Channel names
    assert_test(ParsingUtils::isValidChannelName("#channel"), "#channel valid");
    assert_test(ParsingUtils::isValidChannelName("&channel"), "&channel valid");
    assert_test(ParsingUtils::isValidChannelName("+channel"), "+channel valid");
    assert_test(ParsingUtils::isValidChannelName("!channel"), "!channel valid");
    assert_test(!ParsingUtils::isValidChannelName("channel"), "channel without prefix invalid");
    assert_test(!ParsingUtils::isValidChannelName("#"), "empty channel invalid");
    assert_test(!ParsingUtils::isValidChannelName("#bad char!"), "bad character invalid");
    
    // Nicknames
    assert_test(ParsingUtils::isValidNickname("alice"), "alice valid");
    assert_test(ParsingUtils::isValidNickname("alice123"), "alice123 valid");
    assert_test(ParsingUtils::isValidNickname("a_lice"), "a_lice valid");
    assert_test(ParsingUtils::isValidNickname("a-lice"), "a-lice valid");
    assert_test(!ParsingUtils::isValidNickname("123alice"), "starts with number invalid");
    assert_test(!ParsingUtils::isValidNickname("al ice"), "space invalid");
    assert_test(!ParsingUtils::isValidNickname(""), "empty invalid");
    
    // Numbers
    assert_test(ParsingUtils::isNumber("123"), "123 is number");
    assert_test(ParsingUtils::isNumber("0"), "0 is number");
    assert_test(!ParsingUtils::isNumber("12a"), "12a not number");
    assert_test(!ParsingUtils::isNumber(""), "empty not number");
}

void test_message_helpers() {
    std::cout << "\n--- Message Helpers Tests ---" << std::endl;
    
    Message m1 = Message::parse("PRIVMSG #channel :Hello");
    assert_test(m1.getChannel() == "#channel", "getChannel on PRIVMSG");
    assert_test(m1.getTarget() == "#channel", "getTarget on PRIVMSG");
    assert_test(m1.getMessage() == "Hello", "getMessage on PRIVMSG");
    
    Message m2 = Message::parse("PRIVMSG alice :Hello");
    assert_test(m2.getChannel() == "", "getChannel on private message returns empty");
    assert_test(m2.getTarget() == "alice", "getTarget on private message");
    
    Message m3 = Message::parse("JOIN #secret key123");
    assert_test(m3.getKey() == "key123", "getKey on JOIN");
    
    Message m4 = Message::parse("JOIN #secret");
    assert_test(m4.getKey() == "", "getKey on JOIN without key");
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "     MESSAGE PARSER - COMPLETE TESTS    " << std::endl;
    std::cout << "========================================" << std::endl;
    
    test_basic_parsing();
    test_edge_cases();
    test_irc_commands();
    test_mode_parsing();
    test_prefix_extraction();
    test_validation_utils();
    test_message_helpers();
    
    std::cout << "\n========================================" << std::endl;
    std::cout << "RESULTS: " << tests_passed << " passed, " << tests_failed << " failed" << std::endl;
    std::cout << "========================================" << std::endl;
    
    return (tests_failed == 0) ? 0 : 1;
}
