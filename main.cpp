#include <iostream>
#include <SFML/Network.hpp>
#include <thread>
#include <cstdlib>
#include <list>
#include <cstdio>
#include <conio.h>
#include <windows.h>

#define BLANK std::string(" ") * (messageEnCours.size() + pseudo.size() + 4)

std::string operator*(std::string const& str, unsigned int toMulti){
	std::string toReturn = std::string(str);
	for(unsigned int i = 1; i < toMulti; i++){
		toReturn += str;
	}
	return toReturn;
}

std::string pseudo;

std::map<sf::IpAddress, std::string> userlist;
std::list<sf::IpAddress> ignored;
sf::IpAddress monip;

std::thread* bgThread = nullptr;
std::thread* recThread = nullptr;
std::thread* userThread = nullptr;
std::thread* checkThread = nullptr;

std::string messageEnCours = "";

int discussCanal = 1;

unsigned int port = 2643;

sf::UdpSocket discussSocket;
sf::UdpSocket socketBg;

float version = 3.2;

std::string canal;

bool over = false;

bool mute = false;

void checkPresence();
void discuss();
void rec();
void sendPseudo();
sf::IpAddress searchIp(std::string const& toSearch);
bool isIgnored(sf::IpAddress const& toCheck);

int main(int argc, char *argv[]){
	system("color f0");
	HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);//Ici, on récupère un handle vers ce qui représente la sortie standard sous Windows.
	SetConsoleTextAttribute(consoleHandle, FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_RED | BACKGROUND_RED | BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_INTENSITY);
	std::cout << "Choisissez votre pseudo : ";
	getline(std::cin, pseudo);
  
	std::cout << "Entrez un nom de canal : ";
	getline(std::cin, canal);

	while(canal == "Admin" || canal == "null"){
		std::cout << "Ce canal est reserve. Entrez un nom de canal : ";
		getline(std::cin, canal);
	}
  
	if(discussSocket.bind(port) != sf::Socket::Done){
		std::cout << "Erreur d'ouverture du port. Veuillez verifier si une autre instance du programme n'est pas en cours d'execution." << std::endl;
		#ifdef __WIN32
		system("pause");
		#endif
		return 1;
	}
	//discussSocket.setBlocking(false);
  sf::Packet joinPack;
  joinPack << "join" << version << canal << pseudo;
  discussSocket.send(joinPack, sf::IpAddress::Broadcast, port);

  recThread = new std::thread(rec);
  userThread = new std::thread(sendPseudo);
  //checkThread = new std::thread(checkPresence);
  discuss();

  sf::Packet toQuit;
  toQuit << "quit" << canal;
  socketBg.send(toQuit, sf::IpAddress::Broadcast, port);

  userThread->join();
  recThread->join();
  //checkThread->join();

  delete(userThread);
  delete(recThread);
  //delete(checkThread);

  return 0;
}

void sendPseudo(){
  while(!over){
    sf::Packet pseuSend;
    pseuSend << "user" << canal << pseudo;
    discussSocket.send(pseuSend, sf::IpAddress::Broadcast, port);
    sf::sleep(sf::milliseconds(500));
  }
}

void checkPresence(){
	while(!over){
		for(auto it = userlist.begin(); it != userlist.end(); ++it){
			sf::Packet checker;
			checker << "check";
			discussSocket.send(checker, it->first, port);
			sf::Packet rep;
			sf::IpAddress repSender;
			unsigned short portRep;
			bool ok = false;
			sf::Clock clock;
			while(clock.getElapsedTime().asSeconds() < 3 || !ok){
			    discussSocket.receive(rep, repSender, portRep);
				if(repSender == it->first){
					std::string str;
					rep >> str;
					if(str == "ok"){
						ok = true;
					}
				}
			}
			if(!ok){
				userlist.erase(it);
			}
		}
	}
}

bool isIgnored(sf::IpAddress const& toCheck){
    for(auto it = ignored.begin(); it != ignored.end(); it++){
        if(*it == toCheck){
            return true;
        }
    }
    return false;
}

void rec(){
  while(!over){
    sf::Packet recPack;
    sf::IpAddress sender;
    unsigned short sendPort;
    std::string command;
    discussSocket.receive(recPack, sender, sendPort);
    recPack >> command;
    //std::cout << "Commande recue : " << command << std::endl;
    if(command == "user" || command == "join"){
      bool error = false;
      if(command == "join"){
        float versionCible;
        recPack >> versionCible;
        if(versionCible < version){
            sf::Packet errorSend;
            errorSend << "error" << "Le programme n'est pas a jour!";
            discussSocket.send(errorSend, sender, port);
            error = true;
        }
      }
      if(!error){
	std::string sendercanal;
	recPack >> sendercanal;
	if(sendercanal == canal || sendercanal == "Admin"){
	  std::string sendpseudo;
	  recPack >> sendpseudo;
	  if(userlist[sender] != sendpseudo){
	    userlist[sender] = sendpseudo;
	  }
	  if(command == "join" && sendercanal != "Admin"){
	    std::cout << "\r" << BLANK << "\r" << sendpseudo << " nous a rejoint." << BLANK << std::endl;
		std::cout << pseudo << " - " << messageEnCours;
		FlashWindow(GetConsoleWindow(), TRUE);
	  }
	}
      }
    }else if(command == "check"){
		sf::Packet resp;
		resp << "ok";
		discussSocket.send(resp, sender, sendPort);
	}else if(command == "quit"){
      std::string sendercanal;
      recPack >> sendercanal;
      if(sendercanal == canal){
	std::cout << "\r" << BLANK << "\r" << userlist[sender] << " est parti de la discussion." << std::endl;
	std::cout << pseudo << " - " << messageEnCours;
	FlashWindow(GetConsoleWindow(), TRUE);
	for(auto it = userlist.begin(); it != userlist.end(); ++it){
	  if(it->first == sender){
	    userlist.erase(it);
	    break;
	  }
	}
      }
    }else if(command == "kickass"){
      over = true;
     }else if(command == "error"){
      over = true;
      std::string message;
      recPack >> message;
      std::cout << "\r" << BLANK << "\r" << message << std::endl;
	  std::cout << pseudo << " - " << messageEnCours;
    }else if(command == "shut up"){
      mute = !mute;
    }else if(command == "message"){
      std::string sendercanal;
      recPack >> sendercanal;
      std::string mes;
      recPack >> mes;
      if(sendercanal == canal && !isIgnored(sender)){
        std::cout << "\r" << BLANK << "\r" << userlist[sender] << " - " << mes << std::endl;
		std::cout << pseudo << " - " << messageEnCours;
		FlashWindow(GetConsoleWindow(), TRUE);
      }else if(sendercanal == "Admin"){
	std::cout << "\r" << BLANK << "\r" << "Admin " << userlist[sender] << " - " << mes << std::endl;
	std::cout << pseudo << " - " << messageEnCours;
	FlashWindow(GetConsoleWindow(), TRUE);
      }
    }else if(command == "everywhere"){
	  std::string sendercanal;
      recPack >> sendercanal;
      std::string mes;
      recPack >> mes;
	  if(!isIgnored(sender)){
		  std::cout << "\r" << BLANK << "\r" << "Everywhere : " << userlist[sender] << " - " << mes << std::endl;
		  std::cout << pseudo << " - " << messageEnCours;
		  FlashWindow(GetConsoleWindow(), TRUE);
	  }
	  
	}
  }
}



sf::IpAddress searchIp(std::string const& toSearch){
    for(auto it = userlist.begin(); it != userlist.end(); it++){
        if(it->second == toSearch){
            return it->first;
        }
    }
    return sf::IpAddress::None;
}

auto getIgnoredIt(sf::IpAddress address){
    for(auto it = ignored.begin(); it != ignored.end(); it++){
        if(*it == address){
            return it;
        }
    }
    return ignored.end();
}


void getmessage(std::string& messageRaw){
	int got = 0;
	while(got != 13 && !over){
		if(!mute){
			got = getch();
			if(got != 13 && got > 31){
				messageEnCours += got;
			}else if(got == 8 && messageEnCours.size() != 0){
				messageEnCours.pop_back();
			}
			std::cout << "\r" << BLANK << "\r" << pseudo << " - " << messageEnCours;
		}else{
			std::cout << "\r" << BLANK << "\r" << pseudo << " - " << "---MUTE---";
		}
	}
	messageRaw = messageEnCours;
	messageEnCours = "";
	std::cout << std::endl;
	if(over){
		messageRaw = "";
	}
}

void discuss(){
  sf::sleep(sf::milliseconds(500));
  ignored.push_back(searchIp(pseudo));
  while(!over){
    /*while(mute){
      std::cout << "\r" << "Vous etes muets." << std::flush;
      sf::sleep(sf::milliseconds(1000));
      }*/
    std::string messageRaw;
    std::cout << "\r" << pseudo << " - ";
    getmessage(messageRaw);
    if(messageRaw[0] == '!'){
      if(messageRaw == "!exit"){
        over = true;
      }else if(messageRaw == "!list"){
        for(auto it = userlist.begin(); it != userlist.end(); it++){
            std::cout << "Utilisateur : " << it->second << " Ip : " << it->first.toString() << std::endl;
        }
      }/*else if(messageRaw == "!kick"){
            std::string toKick;
            std::cout << "Pseudo a kick : ";
            getline(std::cin, toKick);
            sf::Packet toSend;
            toSend << "kick" << toKick;
            socketBg.send(toSend, sf::IpAddress::Broadcast, portBg);
      }*/else if(messageRaw == "!ignore"){
          std::string toIgnore;
          std::cout << "Pseudo a ignorer : ";
          getline(std::cin, toIgnore);
          sf::IpAddress ipIgnore = searchIp(toIgnore);
          if(ipIgnore != sf::IpAddress::None && !isIgnored(ipIgnore)){
              ignored.push_back(ipIgnore);
          }else{
              std::cout << "Pseudo introuvable ou deja ignore." << std::endl;
          }
      }else if(messageRaw == "!listen"){
          std::string toListen;
          std::cout << "Pseudo à reecouter : ";
          getline(std::cin, toListen);
          if(isIgnored(searchIp(toListen))){
            auto itor = getIgnoredIt(searchIp(toListen));
            ignored.erase(itor);
          }else{
              std::cout << "Ce pseudo n'a pas ete ignore." << std::endl;
          }
      }else if(messageRaw == "!msg"){
		std::string destinataire;
		std::cout << "Destinataire : ";
		getline(std::cin, destinataire);
		sf::IpAddress senderAddr = searchIp(destinataire);
		if(senderAddr == sf::IpAddress::None){
		  std::cout << "Cette personne n'existe pas." << std::endl;
		}
	}else if(messageRaw == "!everywhere"){
	  std::string messagePerso;
	  std::cout << "Message : \n";
	  getline(std::cin, messagePerso);
	  sf::Packet mesPerso;
	  mesPerso << "everywhere" << canal << messagePerso;
	  discussSocket.send(mesPerso, sf::IpAddress::Broadcast, port);
	}else if(messageRaw == "!join"){
		sf::Packet toQuit;
        toQuit << "quit" << canal;
        socketBg.send(toQuit, sf::IpAddress::Broadcast, port);
		canal = "null";
		std::string newcanal;
	    std::cout << "Nouveau canal :";
		getline(std::cin, newcanal);
		while(newcanal == "Admin" || newcanal == "null"){
			std::cout << "Canal réservé. Nouveau canal : ";
			getline(std::cin, newcanal);
		}
		canal = newcanal;
		sf::Packet joinPack;
		joinPack << "join" << version << canal << pseudo;
		discussSocket.send(joinPack, sf::IpAddress::Broadcast, port);
	}else if(messageRaw == "!canal"){
		std::cout << "Canal actuel : " << canal << std::endl;
	}else{
       std::cout << "Commande inconnue." << std::endl;
    }

}else if(messageRaw != " " && messageRaw != ""){
      sf::Packet message;
      message << "message" << canal << messageRaw;
      discussSocket.send(message, sf::IpAddress::Broadcast, port);
    }
  }
}
