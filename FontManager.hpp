#ifndef FONTMANAGER_HPP
#define FONTMANAGER_HPP

#include <string>
#include <vector>
#include <QFont>

class FontManager {
private:
    QString fontName;
    int weight;
    bool italic;
    std::vector<QFont*> fonts;
public:
    FontManager(std::string fontName, int weight = -1, bool italic = false);
    ~FontManager();
    QFont& operator()(size_t size);
};

#endif // FONTMANAGER_HPP
