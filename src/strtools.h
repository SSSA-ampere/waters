#pragma once


#include <vector>
#include <string>

using namespace std;


// from http://www.oopweb.com/CPP/Documents/CPPHOWTO/Volume/C++Programming-HOWTO-7.html

void Tokenize(const string& str, vector<string>& tokens, const string& delimiters = " ");

int tous(string &s);

string NthToken(const string &str, const string &delim, int n);
//string NthToken(const string &str, char *delim, int n);
//string NthToken(const char *str, char *delim, int n);
string firstToken(const char *str, const char *delim);
string FirsToken_AfterStr(const string &str, const char *delim, const char *AfterStr);
