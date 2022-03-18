#include <iostream>
#include <string>
#include <map>
#include "Current.cpp"
#define HOST			"ec2-3-8-116-10.eu-west-2.compute.amazonaws.com"
using namespace std;

int stringToInt (string s)
{
	if (s == "exit")
	{
		return 0;
	}else if(s == "register"){
		return 1;
	}else if (s == "login"){
		return 2;
	} else if (s =="enter_library"){
		return 3;
	} else if (s == "get_books"){
		return 4;
	} else if (s == "get_book"){
		return 5;
	} else if (s == "add_book"){
		return 6;
	} else if (s == "delete_book"){
		return 7;
	} else if (s == "logout"){ return 8;}
	return 10;
}

int main(int argc, char **argv)
{
	Current current;
	Back *backMsg = NULL;
	string comanda;
	map<string, string> args;
	string id;

	cout << "Introduceti o comanda:\n";

	while (true) {
		cin >> comanda;
		int cmd = stringToInt (comanda);
	switch(cmd) {
		case 0:
			if (backMsg != NULL) {
				delete backMsg;
			}
			return 0;
		break;
		
		case 1:
			cout << "username=";
			cin >> args["username"];

			cout << "password=";
			cin >> args["password"];

			backMsg = current.post(HOST, "/api/v1/tema/auth/register", args);
			if (backMsg == NULL) {
				cout << "Nu se poate conecta la server!\n\n";
			}

			if (backMsg->code == 201) {
				cout << "Successfully register\n\n";
			} else {
				if (backMsg->payload.empty()) {
					cout << "Nu s-a putu creea  cont nou!\n\n";
				} else {
					cout << "EROARE! " << backMsg->payload["error"].get<string>() << "\n\n";
				}
			}
		break;
		case 2:
		cout << "username=";
			cin >> args["username"];

			cout << "password=";
			cin >> args["password"];

			backMsg = current.post(HOST, "/api/v1/tema/auth/login", args);
			if (backMsg == NULL) {
				cout << "EROARE conectare la site\n\n";
			}

			if (backMsg->code == 200) {
				cout << "Logare efectuata!\n\n";
			} else {
				if (backMsg->payload.empty()) {
					cout << "Nu s-a putut loga!\n\n";
				} else {
					cout << "EROARE " << backMsg->payload["error"].get<string>() << "\n\n";
				}
			}
		break;
		
		case 3:
			backMsg = current.get(HOST, "/api/v1/tema/library/access");
			if (backMsg == NULL) {
				cout << "[EROARE conectare la site\n\n";
			}
			// totul ok
			if (backMsg->code == 200) {
				cout << "Intrat in biblioteca!\n\n";
				current.tkn = backMsg->payload["token"].get<string>();
			} else {
				if (backMsg->payload.empty()) {
					cout << "EROARE nu s-a putut intra in biblioteca!\n\n";
				} else {
					cout << "EROARE " << backMsg->payload["error"].get<string>() << "\n\n";
				}
			}
		break;
		// totul ok
		case 4:
			backMsg = current.get(HOST, "/api/v1/tema/library/books");
			if (backMsg == NULL) {
				cout << "EROARE conectare la site!\n\n";
			}

			if (backMsg->code == 200) {
				cout << backMsg->payload.dump(4) << "\n\n";
			} else {
				if (backMsg->payload.empty()) {
					cout << "EROARE, nu s-au putu recupera cartile!\n\n";
				} else {
					cout << "EROARE " << backMsg->payload["error"].get<string>() << "\n\n";
				}
			}
		break;
		
		case 5:
			cout << "id=";
			cin >> id;

			backMsg = current.get(HOST, "/api/v1/tema/library/books/" + id);
			if (backMsg == NULL) {
				cout << "EROARE conectare la site!\n\n";
			}

			if (backMsg->code == 200) {
				cout << backMsg->payload.dump(4) << "\n\n";
			} else {
				if (backMsg->payload.empty()) {
					cout << "EROARE, nu s-au putu recupera cartea!\n\n";
				} else {
					cout << "EROARE " << backMsg->payload["error"].get<string>() << "\n\n";
				}
			}
		break;
		
		case 6:
			cout << "title=";
			cin >> args["title"];

			cout << "author=";
			cin >> args["author"];

			cout << "genre=";
			cin >> args["genre"];

			cout << "publisher=";
			cin >> args["publisher"];

			cout << "page_count=";
			cin >> args["page_count"];

			backMsg = current.post(HOST, "/api/v1/tema/library/books", args);
			if (backMsg == NULL) {
				cout << "EROARE conectare la site\n\n";
			}
			// totul ok
			if (backMsg->code == 200) {
				cout << "Carte adaugata in biblioteca!\n\n";
			} else {
				if (backMsg->payload.empty()) {
					cout << "EROARE, nu s-a putu adauga in biblioteca!\n\n";
				} else {
					cout << "[ERROR] " << backMsg->payload["error"].get<string>() << "\n\n";
				}
			}
		break;
		
		case 7:
			cout << "id=";
			cin >> id;

			backMsg = current.delt(HOST, "/api/v1/tema/library/books/" + id);
			if (backMsg == NULL) {
				cout << "EROARE conectare la site!\n\n";
			}
			// totul ok
			if (backMsg->code == 200) {
				cout << "Carte stersa cu id " << id << "\n\n";
			} else {
				if (backMsg->payload.empty()) {
					cout << "EROARE nu s-a putu sterge cartea";
				} else {
					cout << "[ERROR] " << backMsg->payload["error"].get<string>() << "\n\n";
				}
			}
		break;
		
		case 8:
			backMsg = current.get(HOST, "/api/v1/tema/auth/logout");
			if (backMsg == NULL) {
				cout << "EROARE conectare la site!\n\n";
			}
			// totul ok
			if (backMsg->code == 200) {
				cout << "Successfully logout\n\n";
				current.tkn.clear();
				current.cookies.clear();
			} else {
				if (backMsg->payload.empty()) {
					cout << "EROARE Nu s-a putu deloga!\n\n";
				} else {
					cout << "EROARE " << backMsg->payload["error"].get<string>() << "\n\n";
				}
			}
		break;
		
		default:
			cout << "Comanda invalid, incearca din nou\n\n";
	}
}
	return 0;
}