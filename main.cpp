#include "Main.hpp"
#include <QApplication>
#include <QWidget>
#include <QMessageBox>
#include "InitFrame.hpp"
#include "chat.hpp"
#include <iostream>

Main::Main() {
    initFrame = (new InitFrame(font))->activateButton(this);
    initFrame->show();

}

Main::~Main(){
    if(initFrame != nullptr){
        delete(initFrame);
    }
    if(chatFrame != nullptr){
        delete(chatFrame);
    }

    over = true;

    if(recThread != nullptr){
        recThread->join();
        delete(recThread);
    }
    if(userThread != nullptr){
       userThread->join();
       delete(userThread);
    }

    if(socket.get() != nullptr){
        QNetworkDatagram quitting(QByteArray((std::string("quit") + SEP + channel).c_str()), QHostAddress::Broadcast, port);
        socket->writeDatagram(quitting);
    }
}

void Main::validate(){
    if(initFrame->getChannel() == "null"){
        QMessageBox::critical(initFrame, "Channel réservé", "L'utilisation du channel \"null\" est réservée.");
    }else if(initFrame->getChannel().empty() || initFrame->getUsername().empty()){
        QMessageBox::critical(initFrame, "Vide","Le pseudonyme ou le salon ne peuvent être vides.");
    }else{
        username = initFrame->getUsername();
        channel = initFrame->getChannel();
    }




    chatFrame = new Chat(font, this);
    chatFrame->setGeometry(initFrame->pos().x() - 50, initFrame->pos().y() - 50, 800, 500);
    chatFrame->setWindowIcon(QIcon("duckchat_icon.ico"));
    chatFrame->show();



    delete(initFrame);
    initFrame = nullptr;

    socket = std::make_unique<QUdpSocket>();

    if(!socket->bind(port)){
        QMessageBox::critical(chatFrame, "Erreur réseau", "Impossible de lier le socket au port de discussion. Vérifier qu'une autre instance du programme n'est pas en cours d'exécution.");
        qApp->quit();
    }

    QNetworkDatagram joinPack(QByteArray((std::string("join") + SEP + std::to_string(version) + SEP + channel + SEP + username).c_str()), QHostAddress::Broadcast, port);

    socket->writeDatagram(joinPack);

    recThread = new std::thread(rec, this);
    userThread = new std::thread(sendPseudo, this);
}

std::vector<std::string> Main::split(std::string const &str, char const &splitter, size_t partsNb) {
            size_t instances = 0; //Counts splitter's instances
            for(unsigned int i = 0; i < str.size(); i++) { //Scans the string to find the number of splitter's intances
                if(str[i] == splitter) {
                    instances++;
                    if(instances >= partsNb && partsNb > 0){
                        break;
                    }
                }
            }
            std::vector<std::string> toReturn(instances + 1);                      //Create an array containing all the splits
            for(unsigned int i = 0, enCours = 0; i < str.size(); i++) { //Browses the array to split it
                if(str[i] == splitter && enCours < instances + 1) {
                    enCours++;
                } else {
                    toReturn[enCours] += str[i];
                }
            }

            return toReturn;
}

bool Main::isIgnored(const QHostAddress &add){
    for(QHostAddress addr : ignored){
        if(addr == add){
            return true;
        }
    }
    return false;
}

void Main::rec(Main* that){
    while(!that->over){
        QNetworkDatagram recPack;
        QHostAddress sender;
        std::vector<std::string> recStr;
        while(!(recPack = that->socket->receiveDatagram()).isValid() && !that->over){
          std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        if(that->over){
            break;
        }
        if(!recPack.isNull()){
            recStr = split(recPack.data().toStdString(), SEP);
            sender = recPack.senderAddress();
            if(recStr[0] == "user" || recStr[0] == "join"){
                bool error = false;
                if(recStr[0] == "join"){
                    float targVersion = std::stof(recStr[1]);
                    if(targVersion < that->version){
                        QNetworkDatagram sorry(QByteArray((std::string("error") + SEP + "outdated").c_str()), QHostAddress::Broadcast, that->port);
                        that->socket->writeDatagram(sorry);
                        error = true;
                    }
                }
                if(!error){
                    if(recStr[2] == that->channel){
                        if(that->userlist[sender.toString().toStdString()] != recStr[3]){
                            that->userlist[sender.toString().toStdString()] = recStr[3];
                            that->chatFrame->addUser(that->userlist[sender.toString().toStdString()]);
                        }
                        if(recStr[0] == "join"){
                            that->chatFrame->addMessage("* " + recStr[3] + " a rejoint la discussion.");
                        }
                    }
                }
            }else if(recStr[0] == "check"){
                QNetworkDatagram answer(QByteArray(std::string("ok").c_str()), QHostAddress::Broadcast, that->port);
                that->socket->writeDatagram(answer);
            }else if(recStr[0] == "quit"){
                if(recStr[1] == that->channel){
                    that->chatFrame->addMessage("* " + that->userlist[sender.toString().toStdString()] + " a quitté la discussion.");
                }
                that->chatFrame->delUser(that->userlist[sender.toString().toStdString()]);
                for(auto it = that->userlist.begin(); it != that->userlist.end(); ++it){
                    if(it->first == sender.toString().toStdString()){
                        that->userlist.erase(it);
                        break;
                    }
                }
            }else if(recStr[0] == "error"){
                that->over = true;
                if(recStr[1] == "outdated"){
                    QMessageBox::critical(that->chatFrame, "Erreur", "Cette version de discussion a tenté de communiquer avec une version supérieure. Veuillez mettre à jour cette version.");
                }
                qApp->quit();
            }else if(recStr[0] == "message"){
                if(recStr[1] == that->channel && !that->isIgnored(sender)){
                    that->chatFrame->addMessage("<" + that->userlist[sender.toString().toStdString()] + "> " + recStr[2]);
                }
            }/*else if(recStr[0] == "rename"){
                if(recStr[1] == that->channel){
                    that->chatFrame->renameUser(that->userlist[sender.toString().toStdString()], recStr[2]);
                    that->chatFrame->addMessage("* " + that->userlist[sender.toString().toStdString()] + " est désormais connu(e) sous le nom de " + recStr[2] + ".");
                    that->userlist[sender.toString().toStdString()] = recStr[2];
                }
            }*/
        }


    }
}

void Main::sendPseudo(Main* that){
    QNetworkDatagram unSend(QByteArray((std::string("user") + SEP + std::to_string(that->version) + SEP + that->channel + SEP + that->username).c_str()), QHostAddress::Broadcast, that->port);
    while(!that->over){
        that->socket->writeDatagram(unSend);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}

void Main::talk(){
    std::string message = chatFrame->getMessage();
    if(message[0] == '!'){
        if(message == "!exit"){
            over = true;
            QApplication::exit();
        }/*else if(split(message, ' ')[0] == "!nick"){
            std::vector<std::string> command = split(message, ' ');
            if(command.size() != 2){
                chatFrame->addMessage("[!nick] - Usage : ");
                chatFrame->addMessage("!nick Nouveau Pseudo");
            }else{
                std::string newUn;
                for(size_t i = 1; i < command.size(); i++){
                    newUn += command[i];
                    if(i != command.size() - 1){
                        newUn += " ";
                    }
                }
                username = newUn;
                QNetworkDatagram rename(QByteArray((std::string("rename") + SEP + channel + SEP + username).c_str()), QHostAddress::Broadcast, port);
                socket->writeDatagram(rename);
            }
        }*/else if(message == "!help"){
            chatFrame->addMessage("[!help] - Liste des commandes : ");
            chatFrame->addMessage("!exit : Quitter le chat");
            chatFrame->addMessage("!help : Affiche ce message");
            //chatFrame->addMessage("!nick Nouveau Pseudo : Changer de pseudonyme");
        }
        else{
            chatFrame->addMessage("[Client] - Commande inconnue.");
        }
    }else if(message != " " && message != ""){
        QNetworkDatagram msg(QByteArray((std::string("message") + SEP + channel + SEP + message).c_str()), QHostAddress::Broadcast, port);
        socket->writeDatagram(msg);
    }
}


int main(int argc, char *argv[]) {
       QApplication app(argc, argv);

       Main win;


       return app.exec();
}
