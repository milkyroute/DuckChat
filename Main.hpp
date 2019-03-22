#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <unordered_map>
#include <QMainWindow>
#include <QWidget>
#include <QtNetwork>
#include <QtCore>
#include <string>
#include <list>
#include <thread>
#include "FontManager.hpp"
#include <QSystemTrayIcon>

#define SEP '\u001D'

class InitFrame;
class Chat;

class Main : public QObject{
    Q_OBJECT
private:

    InitFrame* initFrame = nullptr;
    Chat* chatFrame = nullptr;



    FontManager font = FontManager("Sans");

    std::string username;

    std::thread* recThread = nullptr;
    std::thread* userThread = nullptr;

    std::unique_ptr<QUdpSocket> socket;
    std::unordered_map<std::string, std::string> userlist;
    std::list<QHostAddress> ignored;
    QHostAddress myIp;

    quint16 port = 2643;
    float version = 4.0f;
    std::string channel;
    bool over = false;
    bool mute = false;
    unsigned short align1, align2, align3;

public:
    explicit Main();
    virtual ~Main();

    std::string getUsername() {return username;}
    std::string getChannel() {return channel;}

    bool isIgnored(QHostAddress const& add);

    static std::vector<std::string> split(std::string const& str, char const &splitter, size_t partsNb = 0);
    static void rec(Main* that);
    static void sendPseudo(Main* that);

public slots:
    void validate();
    void talk();
};

#endif // MAINWINDOW_HPP
