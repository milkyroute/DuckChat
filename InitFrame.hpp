#include <QWidget>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <memory>
#include "FontManager.hpp"
#include "Main.hpp"

class InitFrame : public QWidget {
    Q_OBJECT
private:
    std::unique_ptr<QLineEdit> username;
    std::unique_ptr<QLineEdit> channel;
    std::unique_ptr<QLabel> usernameLbl;
    std::unique_ptr<QLabel> chanLbl;
    std::unique_ptr<QPushButton> validate;

public:
    InitFrame(FontManager &font, QWidget *parent = nullptr);
    InitFrame* activateButton(Main *mainwindow){
        connect(validate.get(), SIGNAL(clicked()), mainwindow, SLOT(validate()));
        connect(channel.get(), SIGNAL(returnPressed()), mainwindow, SLOT(validate()));
        connect(username.get(), SIGNAL(returnPressed()), mainwindow, SLOT(validate()));
        return this;
    }
    std::string getUsername() const{
        return username->text().toStdString();
    }
    std::string getChannel() const{
        return channel->text().toStdString();
    }
};
