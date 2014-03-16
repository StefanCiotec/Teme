//CIOTEC MARIAN_STEFAN 333CA

#ifndef UTILS_H
#define UTILS_H

#include <iostream>
#include <string>
#include <vector>
#include <sstream>

using namespace std;
//reprezinta un nod al arborelui de parsare
class Node {

public:
	char data;
	Node *left;
	Node *right;
	
	Node(char data, Node *left, Node *right) {
		this->data = data;
		this->left = left;
		this->right = right;
	}
	~Node(){}
};

//verifica daca doua siruri de caractere sunt egale
int equals(char *s1, char *s2) {
	string copy1(s1);
	string copy2(s2);
	if(copy1.compare(copy2) == 0) {
		return 1;
	}
	return 0;
}
//verifica daca un vector de char contine un sir de caractere
int contains(vector<char*> v, char *w) {
	for(int i = 0; i < v.size(); i++) {
		if(equals(v[i], w)) {
			return 1;
		}
	}
	return 0;
}
//verifica daca un vector de int contine un anumit numar
int contains(vector<int> v, int n) {
	for(int i = 0; i < v.size(); i++) {
		if(v[i] == n) {
			return 1;
		}
	}
	return 0;
}

//intoarce pozitia pe care se afla
int getPositionInVector(vector<char*> v, char* w) {
	int i;
	for(i = 0; i < v.size(); i++) {
		if(equals(v[i], w)) {
			return i;
		}
	}
	return -1;
}

string convertInt(int number) {
   stringstream ss;
   ss << number;
   return ss.str();
}	
				
#endif	
