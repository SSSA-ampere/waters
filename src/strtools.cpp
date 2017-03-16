#include "strtools.h"

void Tokenize(const string &str,
                      vector<string> &tokens,
                      const string &delimiters)
{
    // Skip delimiters at beginning.
    string::size_type lastPos = str.find_first_not_of(delimiters, 0);
    
	// Find first "non-delimiter".
    string::size_type pos = str.find_first_of(delimiters, lastPos);

    while (string::npos != pos || string::npos != lastPos)
    {
        // Found a token, add it to the vector.
        tokens.push_back(str.substr(lastPos, pos - lastPos));
    
		// Skip delimiters.  Note the "not_of"
        lastPos = str.find_first_not_of(delimiters, pos);
        
		// Find next "non-delimiter"
        pos = str.find_first_of(delimiters, lastPos);
    }
}


int tous(string &s)
{
    int mul = 1000;
    int m2 = s.length()-2;

    int pos = s.find('?');

    if (pos != string::npos)
    {
        s[pos] = '\x00';
        m2 = pos-2;
    }

    if(s[m2] == 'm')
    {
        mul *= 1000;
    }

    return atoi(s.c_str())*mul;
}




string NthToken(const string &str, const string &delim, int n)
{
    vector<string> tmp_tokens;
    Tokenize(str, tmp_tokens, delim);
    return tmp_tokens[n];
}

/*
string NthToken(const char *str, char *delim, int n)
{
    return NthToken(string(str), string(delim), n);
}

string NthToken(string &str, char *delim, int n)
{
    return NthToken(str, string(delim), n);
}
*/



string firstToken(const char *str, const char *delim)
{
   return NthToken(str, delim, 0);
}


string FirsToken_AfterStr(const string &str, char *delim, char *AfterStr)
{
    string tmp = firstToken(str.c_str(), delim);
    return tmp.substr(tmp.find(AfterStr)+1);
}

