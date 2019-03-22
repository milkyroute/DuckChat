#ifndef CHAT_HPP
#define CHAT_HPP

#include <QWidget>
#include "FontManager.hpp"
#include "Main.hpp"

namespace Ui {
class Chat;
}

class Chat : public QWidget
{
    Q_OBJECT
private:
    Ui::Chat *ui;
    FontManager& fm;
    Main *main;
    std::unique_ptr<QSystemTrayIcon> notif;

public:
    explicit Chat(FontManager &fm, Main *main, QWidget *parent = nullptr);
    ~Chat();

    void addMessage(std::string const& message);
    void addUser(std::string const& username);
    void delUser(std::string const& username);
    void renameUser(std::string const& old, std::string const& newStr);
    void toggleIgnore(std::string const& username);
    void scrollToBottom();
    void setUsername(std::string const& newUsername);
    void setChannel(std::string const& newChannel);

    std::string getMessage();

};

#endif // CHAT_HPP
