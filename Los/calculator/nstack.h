#ifndef NSTACK_H
#define NSTACK_H
#include "lecsema.h"

class NStack
{
public:
    NStack(int size=1024, Lecsema *data=nullptr);
    void add(Lecsema * data);
    Lecsema * get();
    Lecsema * cherryPick();
    int length();
private:
    Lecsema ** replace();
    int m_realsize;
    int m_indexOfFirst;
    int m_indexOfLast;
    int m_size;
    Lecsema ** m_data;

};

#endif // NSTACK_H
