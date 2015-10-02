#include <string>
#include <list>
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include "webattack.h"
using namespace std;

void dostuff(int, string*); 
void error(const char *msg) {
    exit(1);
}

int main(int argc, char *argv[]) {
    int sockfd, newsockfd, portno, pid;
    socklen_t client;
    struct sockaddr_in serv_addr = {0}, cli_addr;

    if (argc < 2) {
        cerr <<"ERROR, no port provided\n";
		return EXIT_FAILURE; 
    }
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        cerr <<"ERROR opening socket";
		return EXIT_FAILURE; 
    }

    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        error("ERROR on binding");
		return EXIT_FAILURE; 
    }
    listen(sockfd, 5);
    client = sizeof(cli_addr);

    newsockfd = accept(sockfd,  (struct sockaddr *) &cli_addr, &client);
    if (newsockfd < 0) {
        error("ERROR on accept");
		return EXIT_FAILURE; 
    }

    const string hostStartTag = "<HOST>";
    const string hostEndTag = "</HOST>";
    const string loginsStartTag = "<LOGINS>";
    const string loginsEndTag = "</LOGINS>";
    const string passwordsStartTag = "<PASSWORDS>";
    const string passwordsEndTag = "</PASSWORDS>";
    const string delimiter = "\n";
    bool hostFound = false;
    bool loginsFound = false;
    bool passwordsFound = false;
    string *fillIncommingData = new string;
    list<string> logins;
    list<string> passwords;
    string hostName;
    while (1) {
	    dostuff(newsockfd, fillIncommingData);
	    if (!hostFound) {
		    if (size_t hostStart = fillIncommingData->find(hostStartTag) != std::string::npos && !hostFound) {
			    size_t hostEnd = fillIncommingData->find(hostEndTag);
			    hostName = fillIncommingData->substr(hostStart + hostStartTag.length() - 1, hostEnd - hostStart - hostStartTag.length() + 1);
			    cout <<"found host = " <<hostName <<"\n";
			    hostFound = true;
		    }
	    }

	    if (!loginsFound) {
		    size_t loginsStartPos = fillIncommingData->find(loginsStartTag);

		    if (loginsStartPos != string::npos) {
			    size_t loginstEndPos = fillIncommingData->find(loginsEndTag);
			    int start = loginsStartPos+loginsStartTag.length();
			    int end = fillIncommingData->find(delimiter, start);
			    while (end < loginstEndPos) {
				    logins.push_back(fillIncommingData->substr(start, end - start));
				    start = end + delimiter.length();
				    end = fillIncommingData->find(delimiter, start);
			    }
			    loginsFound = true;
		    }
	    }
	    if (!passwordsFound) {
		    size_t passwordsStartPos = fillIncommingData->find(passwordsStartTag);

		    if (passwordsStartPos != string::npos) {
			    size_t passwordsEndPos = fillIncommingData->find(passwordsEndPos);
			    int start = passwordsStartPos+passwordsStartTag.length();
			    int end = fillIncommingData->find(delimiter, start);
			    while (end < passwordsEndPos) {
				    passwords.push_back(fillIncommingData->substr(start, end - start));
				    start = end + delimiter.length();
				    end = fillIncommingData->find(delimiter, start);
			    }
			    passwordsFound = true;
		    }
	    }
	    if (hostFound && loginsFound && passwordsFound) {
	    	string login;
	    	string password;
	        startAttack(hostName, logins, passwords, login, password);
	        if (!login.empty() && !password.empty()) {
		    string sendData = "found login " + login + ", found password " + password;
	            write(newsockfd, sendData.c_str(), sendData.length());
	        } else {
	            write(newsockfd, "There was no corresponding login and password", 45);
	        }
		hostFound = false;
		loginsFound = false;
		passwordsFound = false;
		fillIncommingData->clear();
		logins.clear();
		passwords.clear();
	    }
    }
    close(newsockfd);
    close(sockfd);
    delete fillIncommingData;
    return 0; 
}


void dostuff (int sock, string* fillIncommingData) {
	int n;
	char buffer[256]={0};
	n = read(sock, buffer, 255);
	if (n < 0) error("ERROR reading from socket");
	fillIncommingData->append(buffer);
	// n = write(sock,"I got your message",18);
	// if (n < 0) error("ERROR writing to socket");
}
