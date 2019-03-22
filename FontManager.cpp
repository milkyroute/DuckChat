#include "FontManager.hpp"

FontManager::FontManager(std::string fontName, int weight, bool italic)
 : fontName(QString::fromStdString(fontName)),
weight(weight),
italic(italic){

}

QFont& FontManager::operator()(size_t size){
    while(size >= fonts.size()){
        fonts.push_back(nullptr);
    }
    if(fonts.at(size) == nullptr){
        fonts[size] = new QFont(fontName, (int) size, weight, italic);
    }
    return *fonts.at(size);
}

FontManager::~FontManager(){
    for(QFont* font : fonts){
        delete(font);
    }
}
