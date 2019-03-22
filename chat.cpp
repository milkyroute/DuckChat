#include "chat.hpp"
#include "ui_chat.h"
#include <iostream>

Chat::Chat(FontManager &fm, Main *main, QWidget *parent)
    : QWidget(parent),
    ui(new Ui::Chat),
    fm(fm),
    main(main),
    notif(std::make_unique<QSystemTrayIcon>())
    {
    ui->setupUi(this);
    ui->chanLbl->setText(QString::fromStdString(std::string("Channel : ") + main->getChannel()));
    ui->usernameLbl->setText(QString::fromStdString(std::string("Username : ") + main->getUsername()));
    connect(ui->messageBar, SIGNAL(returnPressed()), main, SLOT(talk()));
    notif->show();
}

Chat::~Chat() {
    delete ui;
}

void Chat::addMessage(const std::string &message){
    ui->messageBox->addItem(QString::fromStdString(message));
    while(ui->messageBox->count() > 250){
       delete(ui->messageBox->takeItem(0));
    }
    ui->messageBox->update();
    ui->messageBox->scrollToBottom();
    if(!this->isActiveWindow())
        notif->showMessage("DuckChat", "Nouveau message !");
}

void Chat::addUser(const std::string &username){
    ui->userlist->addItem(QString::fromStdString(username));
}

void Chat::delUser(const std::string &username){
    for(int i = 0; i < ui->userlist->count(); i++){
        if(ui->userlist->item(i)->text().toStdString() == username){
            delete(ui->userlist->takeItem(i));
            break;
        }
    }
}

void Chat::toggleIgnore(const std::string &username){
    for(int i = 0; i < ui->userlist->count(); i++){
        if(ui->userlist->item(i)->text().toStdString() == username){
            ui->userlist->item(i)->setText(QString::fromStdString(username + " (Ignoré)"));
        }else if(ui->userlist->item(i)->text().toStdString() == username + " (Ignoré)"){
            ui->userlist->item(i)->setText(QString::fromStdString(username));
        }
    }
}

std::string Chat::getMessage(){
    std::string str = ui->messageBar->text().toStdString();
    ui->messageBar->setText("");
    return str;
}

void Chat::scrollToBottom(){
    ui->messageBox->scrollToBottom();
}

void Chat::setUsername(std::string const& newUsername){
    ui->usernameLbl->setText(QString::fromStdString(newUsername));
}

void Chat::setChannel(const std::string &newChannel){
    ui->usernameLbl->setText(QString::fromStdString(newChannel));
}

void Chat::renameUser(const std::string &old, const std::string &newStr){
    for(int i = 0; i < ui->userlist->count(); i++){
        if(ui->userlist->item(i)->text().toStdString() == old){
            ui->userlist->item(i)->setText(QString::fromStdString(newStr));
        }else if(ui->userlist->item(i)->text().toStdString() == old + " (Ignoré)"){
            ui->userlist->item(i)->setText(QString::fromStdString(newStr + " (Ignoré)"));
        }
    }
}
