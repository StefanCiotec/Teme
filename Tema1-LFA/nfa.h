//CIOTEC MARIAN_STEFAN 333CA

#ifndef NFA_H
#define NFA_H

#include<vector>
#include<iostream>
#include<string>

using namespace std;

/******************************************************************************
				Class NFA
				
		Clasa ce reprezinta un automat finit nedeterminist
*******************************************************************************/
		
class NFA {

public:
	//0 - states-1  -  multimea finita a starilor
	int states;
	//alfabetul din care sunt formate cuvintele acceptate de automat
	vector<char> alphabet;
	//starea de start
	int start;
	//starile de final
	vector<int> final;
	//matricea de tranzitii
	char **transition;

public:
	
	NFA () {}
	
	NFA(int st) {
		int i, j;
		states = st;
		transition = new char* [states];
		for(i = 0; i < states; i++) {
			transition[i] = new char [states];
		}
		//intial nu exista tranzitii
		for(i = 0; i < states; i++) {
			for(j = 0; j < states; j++) {
				transition[i][j] = 'N';
			}
		}
	}
	//constructor de copiere
	NFA(NFA *nfa) {
		int i,j;
		states = nfa->states;
		for(i = 0; i < nfa->alphabet.size(); i++) {
			alphabet.push_back(nfa->alphabet[i]);
		}
		start = nfa->start;
		for(i = 0; i < nfa->final.size(); i++) {
			final.push_back(nfa->final[i]);
		}
		transition = new char* [states];
		for(i = 0; i < states; i++) {
			transition[i] = new char [states];
		}
		for(i = 0; i < states; i++) {
			for(j = 0; j < states; j++) {
				transition[i][j] = nfa->transition[i][j];
			}
		}
	}
	
	//adauga un caracter la alfabet
	void addToAlphabet(char c) {
		alphabet.push_back(c);
	}
		
	//adauga starea initiala
	void addInitialState(int s) {
		start = s;
	}
	
	//adauga o stare finala
	void addFinalState(int f) {
		final.push_back(f);
	}
	
	//adauga un caracter la o tranzitie
	void addSymbolToTransition(int from, int to, char c) {
		transition[from][to] = c;
	}
	
	//construieste un NFA simplu cu 2 stari si o tranzitie
	NFA* build_simple_nfa(char c) {
		NFA* simple_nfa = new NFA(2);
		simple_nfa->addInitialState(0); 
		simple_nfa-> addFinalState(1);
		//adaug tranzitia de la 0 la 1
		if(c != 'O') {
			simple_nfa->addSymbolToTransition(0, 1, c);
		}
		return simple_nfa;
	}
	
	//reuniunea a doua NFA
	NFA* or_nfa(NFA* nfa1, NFA* nfa2) {
		int i,j;
		//numarul total de stari al automatului nou
		int total_states = nfa1->states + nfa2->states + 2;
		//noua stare finala a automatului
		int new_final = total_states - 1;
		
		NFA* new_nfa = new NFA(total_states);
		new_nfa->addInitialState(0);
		new_nfa->addFinalState(new_final);
		
		int start1 = 1; int start2 = nfa1->states + 1;
		new_nfa->addSymbolToTransition(0, start1, 'e');
		new_nfa->addSymbolToTransition(0, start2, 'e');
		
		for(i = 0; i < nfa1->states; i++) {
			for(j = 0; j < nfa1->states; j++) {
				new_nfa->addSymbolToTransition(i + start1, j + start1, 
								nfa1->transition[i][j]);
			}
		}
		for(i = 0; i < nfa2->states; i++) {
			for(j = 0; j < nfa2->states; j++) {
				new_nfa->addSymbolToTransition(i + start2, j + start2, 
								nfa2->transition[i][j]);
			}
		}
		//intre starile finale ale automatelor initiale si starea finala
		//a automatului nou adaug tranzitii pe sirul vid
		for(i = 0; i < nfa1->final.size(); i++) {
			new_nfa->addSymbolToTransition(nfa1->final[i] + start1, new_final, 'e');
		}
		for(i = 0; i < nfa2->final.size(); i++) {
			new_nfa->addSymbolToTransition(nfa2->final[i] + start2, new_final, 'e');
		}
		return new_nfa;
	}
		
		
	//concatenarea a doua NFA
	NFA* concat_nfa(NFA* nfa1, NFA* nfa2) {
		int i,j;
		int shift = nfa1->states - 1;
		//starile automatutlui nou
		int total_states = nfa2->states + shift;
		NFA* new_nfa = new NFA(total_states);
		//starea initiala a automatului nou
		new_nfa->addInitialState(nfa1->start);
		//starile finale ale automatului nou
		for(i = 0; i < nfa2->final.size(); i++) {
			new_nfa->addFinalState(nfa2->final[i] + shift);
		}
		
		for(i = 0; i < nfa2->states; i++) {
			for(j = 0; j < nfa2->states; j++) {
				new_nfa->addSymbolToTransition(i + shift, j + shift, 
									nfa2->transition[i][j]);
			}
		}
		for(i = 0; i < nfa1->states; i++) {
			for(j = 0; j < nfa1->states; j++) {
				new_nfa->addSymbolToTransition(i, j, nfa1->transition[i][j]);
			}
		}
		return new_nfa;
	}
	
	//Kleene - Star
	NFA* star_nfa(NFA* nfa) {
		int i, j;
		int shift = 1;
		//adaug doua noi stari
		int total_states = nfa->states + 2;
		int new_start = 0;
		int new_final = total_states - 1;
		NFA* new_nfa = new NFA(total_states);
		new_nfa->addInitialState(new_start);
		new_nfa->addFinalState(new_final);
		for(i = 0; i < nfa->states; i++) {
			for(j = 0; j < nfa->states; j++) {
				new_nfa->addSymbolToTransition(i + shift, j + shift, nfa->transition[i][j]);
						
			}
		}
		new_nfa->addSymbolToTransition(0, nfa->start + shift, 'e');
		new_nfa->addSymbolToTransition(0, new_final, 'e');
		for(i = 0; i < nfa->final.size(); i++) {
			new_nfa->addSymbolToTransition(nfa->final[i] + shift, nfa->start + shift, 'e');
			new_nfa->addSymbolToTransition(nfa->final[i] + shift, new_final, 'e');
		}	
		return new_nfa;
	}
	
	//operatorul +
	NFA* plus_nfa(NFA* nfa) {
		int i, j;
		int shift = 1;
		//adaug doua noi stari
		int total_states = nfa->states + 2;
		int new_start = 0;
		int new_final = total_states - 1;
		NFA* new_nfa = new NFA(total_states);
		new_nfa->addInitialState(new_start);
		new_nfa->addFinalState(new_final);
		for(i = 0; i < nfa->states; i++) {
			for(j = 0; j < nfa->states; j++) {
				new_nfa->addSymbolToTransition(i + shift, j + shift, nfa->transition[i][j]);
			}
		}
		new_nfa->addSymbolToTransition(0, nfa->start + shift, 'e');
		for(i = 0; i < nfa->final.size(); i++) {
			new_nfa->addSymbolToTransition(nfa->final[i] + shift, nfa->start + shift, 'e');
			new_nfa->addSymbolToTransition(nfa->final[i] + shift, new_final, 'e');
		}	
		return new_nfa;
	}
	
	//operatorul ?
	NFA* optional_nfa(NFA* nfa) {
		int i, j;
		int shift = 1;
		//adaug doua noi stari
		int total_states = nfa->states + 2;
		int new_start = 0;
		int new_final = total_states - 1;
		NFA* new_nfa = new NFA(total_states);
		new_nfa->addInitialState(new_start);
		new_nfa->addFinalState(new_final);
		for(i = 0; i < nfa->states; i++) {
			for(j = 0; j < nfa->states; j++) {
				new_nfa->addSymbolToTransition(i + shift, j + shift, nfa->transition[i][j]);
			}
		}
		new_nfa->addSymbolToTransition(0, nfa->start + shift, 'e');
		new_nfa->addSymbolToTransition(0, new_final, 'e');
		for(i = 0; i < nfa->final.size(); i++) {
			new_nfa->addSymbolToTransition(nfa->final[i] + shift, new_final, 'e');
		}	
		return new_nfa;
	}
		
	//construieste un NFA pe baza unui arbore de parsare
	NFA* createNew(Node* node) {
		NFA* new_nfa = build_nfa(node);
		return new_nfa;
	}
		
	NFA* build_nfa(Node* node) {
		if(node->data == '|') {
			return or_nfa(build_nfa(node->right), build_nfa(node->left));
		}
		else if(node->data == 'P') {
			return concat_nfa(build_nfa(node->right), build_nfa(node->left));
		}
		else if(node->data == '*') {
			return star_nfa(build_nfa(node->left));
		}
		else if(node->data == '+') {
			return plus_nfa(build_nfa(node->left));
		}
		else if(node->data == '?') {
			return optional_nfa(build_nfa(node->left));
		}
		else {
			return build_simple_nfa(node->data);
		}
	}
	
	//afiseaza un automat nedeterminist
	void show_nfa() {
		int i, j, first = 1;
		//printare NFA
		string nfa_string("(");
		//afisare multime stari
		nfa_string += "{S0";
		for(i = 1; i < states; i++) {
			nfa_string += ",S";
			nfa_string += convertInt(i);
		}
		nfa_string += "},{";
		nfa_string += alphabet[0];
		//afisare alfabet
		for(i = 1; i < alphabet.size(); i++) {
			nfa_string += ",";
			nfa_string += alphabet[i];
		}
		nfa_string += "},{";
		for(i = 0; i < states; i++) {
			for(j = 0; j < states; j++) {
				if(transition[i][j] != 'N') {
					if(first == 1) {
						first = 0;
					}
					else {
						nfa_string += ",";
					}
					nfa_string += "(";
					nfa_string += "S";
					nfa_string += convertInt(i);
					nfa_string += ",";
					nfa_string += transition[i][j];
					nfa_string += ",";
					nfa_string += "S";
					nfa_string += convertInt(j);
					nfa_string += ")";
						
				}
			}
		}
		nfa_string += "},S";
		nfa_string += convertInt(start);
		nfa_string += ",";
		if(final.size() == 0) {
			nfa_string += "O";
		}
		else {
			nfa_string += "{S";
			nfa_string += convertInt(final[0]);
			for(i = 1; i < final.size(); i++) {
				nfa_string += ",";
				nfa_string += "S";
				nfa_string += convertInt(final[i]);
			}
			nfa_string += "}";
		}		
		nfa_string += ")";
		cout << nfa_string << endl;
		
		
	}
	
	~NFA() {
		for(int i = 0; i < states; i++) {
			delete [] transition[i];
		}
		delete [] transition;
	}				
};

/******************************************************************************/


class State {

public:
	char *name;
	int number;
	
	State(char *name, int number) {
		this->name = name;
		this->number = number;
	}
	~State() {
		delete name;
	}
};

/******************************************************************************
				Class DFA_State
			
		Reprezinta o stare a automatului finit determinist
*******************************************************************************/

class DFA_State {

public:
	//identificatorul starii
	int state;
	//stari reachable pe sirul vid
	vector<int> reachable;
	//1 daca toate tranzitiile din ea au fost explorate
	int marked;
	
	DFA_State(int _state) {
		state = _state;
		marked = 0;
	}
	
	DFA_State(int _state, vector<int> _reachable) {
		state = _state;
		for(int i = 0;  i < _reachable.size(); i++) {
			reachable.push_back(_reachable[i]);
		}
		//initial starea este nemarcata
		marked = 0;
	}
	//1 - daca starea contine o stare finala a NFA
	int containsFinal(NFA* nfa) {
		for(int i = 0; i < nfa->final.size(); i++) {
			if(contains(reachable, nfa->final[i])) {
				return 1;
			}
		}
		return 0;
	}
	//1 - reachable este identic cu v
	int containsVector(vector<int> v) {
		int i,j;
		if(reachable.size() != v.size()) {
			return 0;
		}
		else {
			vector<int> match;
			for(i = 0; i < v.size(); i++) {
				match.push_back(0);
			}
			for(i = 0; i < reachable.size(); i++) {
				for(j = 0; j < v.size(); j++) {
					if((reachable[i] == v[j]) && (match[j] == 0)) {
						match[j] = 1;
					}
				}
			}
			for(i = 0; i < match.size(); i++) {
				if(match[i] == 0) {
					return 0;
				}
			}
		}
		return 1;
	}
	
	~DFA_State(){}
};

/******************************************************************************/


/******************************************************************************
				Class DFA_Transition
			
		Reprezinta o tranzitie a automatului finit determinist
*******************************************************************************/
//retine o tranzitie a unui DFA
class DFA_Transition {

public:
	DFA_State* from;
	char in;
	DFA_State* to;
	
	DFA_Transition(DFA_State* _from, char _in, DFA_State* _to) {
		from = _from;
		in = _in;
		to = _to;
	}
	~DFA_Transition() {
		delete from;
		delete to;
	}
};
	
	
	
/******************************************************************************/


/******************************************************************************
				Class DFA
				
		Reprezinta clasa automatului finit determinist
*******************************************************************************/
class DFA {

public:
	//alfabetul
	vector<char> alphabet;
	//starea de start
	DFA_State* start;
	//starile de final
	vector<DFA_State*> final;
	//totalitatea starilor
	vector<DFA_State*> states;
	//totalitatea tranzitiilor
	vector<DFA_Transition*> transition;
	
	DFA(vector<char> _alphabet) {
		for(int i = 0; i < _alphabet.size(); i++) {
			alphabet.push_back(_alphabet[i]);
		}
	}
	
	//intorce prima stare nemarcata
	DFA_State* getFirstUnmarked() {
		for(int i = 0; i < states.size(); i++) {
			if(states[i]->marked == 0) {
				return states[i];
			}
		}
		return NULL;
	}
	
	//in cazul in care contine o stare reprezentata printr-un vector reachable identic
	DFA_State* containsState(vector<int> reachable) {
		for(int i = 0; i < states.size(); i++) {
			if(states[i]->containsVector(reachable)) {
				return states[i];
			}
		}
		return NULL;
	}
	//verifica daca o stare este finala
	int isFinal(DFA_State *s) {
		for(int i = 0; i < final.size(); i++) {
			if(final[i]->state == s->state) {
				return 1;
			}
		}
		return 0;
	}
	//intoarce 1 daca exista tranzitie de la i la j pe c
	int hasTransition(int i, char c, int j) {
		for(int k = 0; k < transition.size(); k++) {
			if((transition[k]->from->state == i) && 
			   (transition[k]->to->state == j) &&
			   (transition[k]->in == c)) {
			   	return 1;
			}
		}
		return 0;
	}
	//intoarce starea in care automatul ajunge dupa tranzitie din
	//starea curenta pe caracterul de input
	DFA_State* nextState(DFA_State* source, char in) {
		for(int i = 0; i < transition.size(); i++) {
			if((transition[i]->from->state == source->state) &&
			   (transition[i]->in == in)) {
			   	return transition[i]->to;
			}
		}
		return NULL;
	}
	//functie care verifica apartenenta unui cuvant la un DFA
	int containsWord(string word) {
		int i;
		DFA_State *next = start;
		//daca nu este sirul vid
		if(word[0] != 'e') {
			for(i = 0; i < word.length(); i++) {
				next = nextState(next, word[i]);
			}
		}
		if(isFinal(next)) {
			return 1;
		}
		return 0;
	}		
	
	void show_dfa() {
		int i, first = 1;
		//printare DFA
		string dfa_string("(");
		//afisare multime stari
		dfa_string += "{S";
		dfa_string += convertInt(states[0]->state);
		for(i = 1; i < states.size(); i++) {
			dfa_string += ",S";
			dfa_string += convertInt(states[i]->state);
		}
		dfa_string += "},{";
		dfa_string += alphabet[0];
		//afisare alfabet
		for(i = 1; i < alphabet.size(); i++) {
			dfa_string += ",";
			dfa_string += alphabet[i];
		}
		dfa_string += "},{";
		for(i = 0; i < transition.size(); i++) {
			if(first == 1) {
				first = 0;
			}
			else {
				dfa_string += ",";
			}
			dfa_string += "d(";
			dfa_string += "S";
			dfa_string += convertInt(transition[i]->from->state);
			dfa_string += ",";
			dfa_string += transition[i]->in;
			dfa_string += ")=";
			dfa_string += "S";
			dfa_string += convertInt(transition[i]->to->state);
		}
		dfa_string += "},S";
		dfa_string += convertInt(start->state);
		dfa_string += ",";
		if(final.size() == 0) {
			dfa_string += "O";
		}
		else {
			dfa_string += "{S";
			dfa_string += convertInt(final[0]->state);
			for(i = 1; i < final.size(); i++) {
				dfa_string += ",";
				dfa_string += "S";
				dfa_string += convertInt(final[i]->state);
			}
			dfa_string += "}";
		}		
		dfa_string += ")";
		cout << dfa_string << endl;
	}
		
	
	~DFA() {
		delete start;
	}
	
};

/******************************************************************************/	

int getStateNumber(vector<State*> my_states, char *name) {
	int i;
	for(i = 0; i < my_states.size(); i++) {
		if(equals(name, my_states[i]->name)) {
			return my_states[i]->number;
		}
	}
	return 0;
}

//transforma un AFN cu tranzitii si pe siruri de caractere
//intr-un AFN cu tranztii numai pe un singur caracter
NFA* transformToMyNFA(vector<char*> states, vector<char> alphabet, vector<char*> from, 
			vector<vector<char> > input, vector<char*> to, char* start, vector<char*> final) {
	
	int i, j, k, l, state_no = 1;
	NFA* my_nfa = new NFA(states.size());
	vector<State*> my_states;
	//adaug celelalte stari
	for(i = 0; i < states.size(); i++) {
		my_states.push_back(new State(states[i], i));
		//adaug daca este stare intiala
		if(equals(start, states[i])) {
			my_nfa->addInitialState(i);
		}
		//adaug daca este stare finala
		if(contains(final, states[i])) {
			my_nfa->addFinalState(i);
		}
	}
	//adaug tranzitiile
	for(i = 0; i < from.size(); i++) {
		int source = getStateNumber(my_states, from[i]);
		int dest = getStateNumber(my_states, to[i]);
		//daca am un singur caracter de adaugat si nu exista tranzitie
		//pastez configuratia si adaug tranzitia pe caracterul respectiv
		if((input[i].size() == 1) && (my_nfa->transition[source][dest] == 'N')) {
			my_nfa->addSymbolToTransition(source, dest, input[i][0]);
		}
		//altfel creez un automat nou cu vechile tranzitii si stari plus
		//tranzitii noi si stari
		else {

			NFA* old_nfa = new NFA(my_nfa);
			my_nfa = new NFA(old_nfa->states + input[i].size());
			my_nfa->addInitialState(0);
			//adaugare stare finala
			for(k = 0; k < old_nfa->final.size(); k++) {
				if(old_nfa->final[k] <= source) {
					my_nfa->addFinalState(old_nfa->final[k]);
				}
				else {
					my_nfa->addFinalState(old_nfa->final[k] + input[i].size());
				}
				
			}
			j = input[i].size();
			//copiere tranzitii din automatul anterior
			for(k = 0; k < old_nfa->states; k++) {
				for(l = 0; l < old_nfa->states; l++) {
					if(k <= source) {
						if(l <= source) {
							my_nfa->transition[k][l] = 
								old_nfa->transition[k][l];	
						} else {
							my_nfa->transition[k][l+j] = 
								old_nfa->transition[k][l];
						}
					}
					else {
						if(l <= source) {
							my_nfa->transition[k+j][l] = 
								old_nfa->transition[k][l];
						}
						else {
							my_nfa->transition[k+j][l+j] =
								 old_nfa->transition[k][l];
						}
					}
				}
			}
			//stabilire tranzitii noi - pentru fiecare caracter din sir apare
			//o stare si o tranzitie noua
			for(j = 0; j < input[i].size(); j++) {
				my_nfa->addSymbolToTransition(source+j, source+j+1, input[i][j]);
			}
			if(dest <= source) {
				my_nfa->addSymbolToTransition(source+j, dest, 'e');
			}
			else {
				my_nfa->addSymbolToTransition(source+j, dest+j, 'e');
			}
			//shiftare states number cu input[i].size() cu exceptia starii de start
			for(k = 0; k < my_states.size(); k++) {
				if(my_states[k]->number > source) {
					my_states[k]->number += j;
				}
			}						
		}						
	}
	my_nfa->alphabet = alphabet;
	return my_nfa;
}

//intoarce un set de stari din NFA in care se poate ajunge
//numai prin tranzitii pe sirul vid din setul de stari curente
vector<int> e_reachable(NFA* nfa, vector<int> from) {
	int i, s;
	vector<int> result;
	vector<int> stack;
	for(i = 0; i < from.size(); i++) {
		result.push_back(from[i]);
		stack.push_back(from[i]);
	}
	while(!stack.empty()) {
		int t = stack.back();
		stack.pop_back();
		for(s = 0; s < nfa->states; s++) {
			if(nfa->transition[t][s] == 'e') {
				if(!contains(result, s)) {
					result.push_back(s);
					stack.push_back(s);
				}
			}
		}
	}
	return result;
}

//intoarce un set de stari din NFA in care se poate ajunge
//numai prin trazitii pe un anumit input din setul de stari curente
vector<int> input_reachable(NFA* nfa, vector<int> from, char in) {
	int i,j;
	vector<int> result;
	for(i = 0; i < from.size(); i++) {
		for(j = 0; j < nfa->states; j++) {
			if(nfa->transition[from[i]][j] == in) {
				if(!contains(result, j)) {
					result.push_back(j);
				}
			}
		}
	}
	return result;
}

/******************************************************************************
			Transforma un NFA intr-un DFA
*******************************************************************************/

DFA* transform_NFA_to_DFA(NFA* nfa) {
	int i;
	vector<int> from;
	//numarul de stari care au fost create si adaugate la DFA
	int state_no = 0;
	//setul de stari reachable pe sir vid
	vector<int> reachable;
	//retine starea curenta si starea urmatoare
	DFA_State *current_state, *new_state;
	DFA_Transition* new_transition;
	
	//noul AFD
	DFA* dfa = new DFA(nfa->alphabet);
	from.push_back(nfa->start);
	reachable = e_reachable(nfa, from);
	from.clear();
	new_state = new DFA_State(state_no, reachable);
	dfa->states.push_back(new_state);
	dfa->start = new_state;
	state_no++;
	reachable.clear();
	//cat timp exista cel putin o stare nemarcata
	while((current_state = dfa->getFirstUnmarked()) != NULL) {
		current_state->marked = 1;
		//in cazul in care contine o stare finala a NFA
		if(current_state->containsFinal(nfa)) {
			dfa->final.push_back(current_state);
		}
		for(i = 0; i < nfa->alphabet.size(); i++) {
			char in = nfa->alphabet[i];
			vector<int> vec = input_reachable(nfa, current_state->reachable, in);
			reachable = e_reachable(nfa, vec);
			new_state = dfa->containsState(reachable);
			//daca nu exista starea se adauga o noua stare la DFA
			//si o noua tranzitie
			if(new_state == NULL) {
				new_state = new DFA_State(state_no, reachable);
				state_no++;
				reachable.clear();
				dfa->states.push_back(new_state);
				new_transition = new DFA_Transition(current_state, in, new_state);
				dfa->transition.push_back(new_transition);
			}
			//altfel se dauga doar tranzitie
			else {
				new_transition = new DFA_Transition(current_state, in, new_state);
				dfa->transition.push_back(new_transition);
			}
		}
	}
	return dfa;
}


/******************************************************************************
			Transforma un DFA intr-un NFA
*******************************************************************************/

NFA* transform_DFA_to_NFA(DFA* dfa) {
	int i;
	NFA* nfa = new NFA(dfa->states.size());
	for(i = 0; i < dfa->alphabet.size(); i++) {
		nfa->addToAlphabet(dfa->alphabet[i]);
	}
	for(i = 0; i < dfa->transition.size(); i++) {
		nfa->addSymbolToTransition(dfa->transition[i]->from->state, 
					   dfa->transition[i]->to->state, 
					   dfa->transition[i]->in);
	}
	nfa->addInitialState(dfa->start->state);
	for(i = 0; i < dfa->final.size(); i++) {
		nfa->addFinalState(dfa->final[i]->state);
	}
	return nfa;
}
	
//realizeaza SAU intre doua REGEX
vector<char> alternation(vector<char> partial1, vector<char> partial2) {
	vector<char> result;
	if(partial1.size() == 1) {
		if(partial1[0] == 'O') {
			result = partial2;
			return result;
		}
		else {
			result.push_back(partial1[0]);
		}
	} else {
		result.push_back('(');
		for(int i = 0; i < partial1.size(); i++) {
			result.push_back(partial1[i]);
		}
		result.push_back(')');
	}
	result.push_back('|');
	if(partial2.size() == 1) {
		if(partial2[0] == 'O') {
			result = partial1;
			return result;
		}
		else {
			result.push_back(partial2[0]);
		}
	} else {
		result.push_back('(');
		for(int i = 0; i < partial2.size(); i++) {
			result.push_back(partial2[i]);
		}
		result.push_back(')');
	}
	return result;
}

//realizeaza concatenarea intre doua REGEX
vector<char> concat(vector<char> partial1, vector<char> partial2) {
	vector<char> result;
	if(partial1.size() == 1) {
		if(partial1[0] == 'O') {
			result.push_back('O');
			return result;
		} else if(partial1[0] == 'e') {
			result = partial2;
			return result;
		}
		else {
			result.push_back(partial1[0]);
		}
	} else {
		result.push_back('(');
		for(int i = 0; i < partial1.size(); i++) {
			result.push_back(partial1[i]);
		}
		result.push_back(')');
	}
	if(partial2.size() == 1) {
		if(partial2[0] == 'O') {
			result.clear();
			result.push_back('O');
			return result;
		} else if(partial2[0] == 'e') {
			result.clear();
			result = partial1;
			return result;
		}
		else {
			result.push_back(partial2[0]);
		}
	} else {
		result.push_back('(');
		for(int i = 0; i < partial2.size(); i++) {
			result.push_back(partial2[i]);
		}
		result.push_back(')');
	} 
	return result;
}
	
//aplica operatorul Kleene-Star pe un REGEX
vector<char> star(vector<char> partial) {
	vector<char> result;
	if(partial.size() == 1) {
		if(partial[0] == 'O') {
			result.push_back('e');
		}
		else {
			result.push_back(partial[0]);
			result.push_back('*');
		}
	}
	else {
		result.push_back('(');
		for(int i = 0; i < partial.size(); i++) {
			result.push_back(partial[i]);
		}
		result.push_back(')');
		result.push_back('*');
	}
	return result;
}

/******************************************************************************
			Transforma un DFA intr-un REGEX
*******************************************************************************/

vector<char> transform_DFA_to_REGEX(DFA* dfa) {
	int m = dfa->states.size();
	vector<char> in_vector;
	vector<vector<vector<char> > > A(m, vector<vector<char> >(m));
	vector<vector<char> > B(m);
	
	if(dfa->final.size() == 0) {
		B[0].push_back('O');
	}
	else {
		for(int i = 0; i < m; i++) {
			if(dfa->isFinal(dfa->states[i])) {
				B[i].push_back('e');
			}
			else {
				B[i].push_back('O');
			}
		}
		for(int i = 0; i < m; i++) {
			for(int j = 0; j < m; j++) {
				A[i][j].push_back('O');
			}
		}
	
		for(int i = 0; i < m; i++) {
			for(int j = 0; j < m; j++) {	
				for(int k = 0; k < dfa->alphabet.size(); k++) {
					char in = dfa->alphabet[k];
					if(dfa->hasTransition(i, in, j)) {
						in_vector.push_back(in);
						A[i][j] = alternation(A[i][j], in_vector);
						in_vector.clear();
					}
				}
			}
		}
	
		for(int n = m - 1; n >= 0; n--) {
			B[n] = concat(star(A[n][n]), B[n]);
			for(int j = 0; j < n; j++) {
				A[n][j] = concat(star(A[n][n]), A[n][j]);
			}
			for(int i = 0; i < n; i++) {
				B[i] = alternation(B[i], concat(A[i][n], B[n]));
				for(int j = 0; j < n; j++) {
					A[i][j] = alternation(A[i][j], concat(A[i][n], A[n][j]));
				}
			}
		}
	}
	return B[0];
}

//formeaza un DFA pe baza inputului
DFA* create_DFA(vector<char*> states, vector<char> alphabet, vector<char*> from, 
			vector<vector<char> > input, vector<char*> to, char* start, vector<char*>final)	{

	int i;
	vector<DFA_State*> dfa_states;
	vector<DFA_Transition*> dfa_transitions;
	DFA_State* dfa_start;
	vector<DFA_State*> dfa_final;
	//creare stari
	for(i = 0; i < states.size(); i++) {
		dfa_states.push_back(new DFA_State(i));
	}
	//creare tranzitii
	for(i = 0; i < from.size(); i++) {
		//indicii starilor
		int f = getPositionInVector(states, from[i]);
		int t = getPositionInVector(states, to[i]);
		dfa_transitions.push_back(new DFA_Transition(dfa_states[f], input[i][0], dfa_states[t]));
	}
	dfa_start = dfa_states[getPositionInVector(states, start)];
	for(i = 0; i < final.size(); i++) {
		dfa_final.push_back(dfa_states[getPositionInVector(states, final[i])]);
	}
	DFA* dfa = new DFA(alphabet);
	dfa->start = dfa_start;
	dfa->states = dfa_states;
	dfa->transition = dfa_transitions;
	dfa->final = dfa_final;
	return dfa;
}
					
#endif
