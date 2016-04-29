#include "Label2.h"



Label2::Label2()
{
   isconstant = false;
}

Label2::Label2(int id)
{
    isconstant = false;
    this->id = id;
}

Label2::Label2(int id, bool c)
{
    isconstant = c;
    this->id = id;
}

Label2::~Label2()
{

}

string Label2::getName()
{
    return string("Label_") + to_string(id);
}

int Label2::getid()
{
    return id;
}

void Label2::setid(int id)
{
    this->id = id;
}

bool Label2::isConstant()
{
    return isconstant;
}

void Label2::setBitSize(int bs)
{
    size_bit = bs;
}

int Label2::getBitSize()
{
    return size_bit;
}

void Label2::setIsConstant(bool ic)
{
    isconstant = ic;
}


bool Label2::getIsConstant()
{
    return isconstant;
}


