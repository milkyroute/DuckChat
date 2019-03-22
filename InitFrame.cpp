#include "InitFrame.hpp"
#include <memory>

InitFrame::InitFrame(FontManager &font, QWidget *parent)
    : QWidget (parent),
    username(std::make_unique<QLineEdit>(this)),
    channel(std::make_unique<QLineEdit>(this)),
    usernameLbl(std::make_unique<QLabel>(this)),
    chanLbl(std::make_unique<QLabel>(this)),
    validate(std::make_unique<QPushButton>(this)){

    username->setGeometry(20, 30, 2*161, 30);
    username->setFont(font(12));

    channel->setGeometry(20, 100, 2*161, 30);
    channel->setFont(font(12));

    usernameLbl->setGeometry(20, 10, 2*161, 20);
    usernameLbl->setText("Pseudonyme : ");
    usernameLbl->setFont(font(12));

    this->setWindowIcon(QIcon("duckchat_icon.ico"));

    chanLbl->setGeometry(20, 80, 2*161, 20);
    chanLbl->setText("Channel : ");
    chanLbl->setFont(font(12));

    validate->setGeometry(240, 150, 100, 40);
    validate->setText("Valider");
    validate->setFont(font(15));

    this->setFixedSize(360, 200);

    QIcon icon;
    icon.addFile(QString::fromUtf8(":/icon/duckchat_icon.ico"), QSize(), QIcon::Normal, QIcon::Off);
    this->setWindowIcon(icon);

}
