#pragma once

#include <string>
#include <vector>

using namespace std;

class Runnable2;

class Label2
{
public:

    Label2();
    Label2(int id);
    //Label2(int id, bool c);


    ~Label2();

    string getName();
    int getid();
    void setid(int id);
    bool isConstant();

    void setBitSize(int bs);
    int getBitSize();

    void setIsConstant(bool ic);
    bool getIsConstant();

    vector<Runnable2 *> runnablesRead_list;
    vector<Runnable2 *> runnablesWrite_list;

	int getRamLoc();
	void setRamLoc(int r);

private:

    void init();

    int id;
    bool isconstant;
    int size_bit;

	int ram;
};
