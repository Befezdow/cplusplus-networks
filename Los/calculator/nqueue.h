#ifndef NQUEUE_H
#define NQUEUE_H
#include "lecsema.h"


class NQueue
{
public:
    NQueue(int size=1024, Lecsema * data = NULL);
    void add(Lecsema *data);
    Lecsema * get();
    int length();
    Lecsema * showLast();
private:
    Lecsema **replace();
    int m_indexOfFirst;
    int m_indexOfLast;
    int m_realsize;
    int m_size;
    Lecsema ** m_data;

};

#endif // NQUEUE_H
