#include "nqueue.h"
#include <iostream>
#include <cstdlib>
using namespace std;


NQueue::NQueue( int size,Lecsema *data)
{
    m_size=size;
    m_data=new Lecsema* [size];
    m_realsize=0;
    for(int i=0;i<size;i++)
    {
        m_data[i]=NULL;
    }
    m_indexOfFirst=-1;
    m_indexOfLast=-1;
    if(data)
    {
        m_indexOfFirst=0;
        m_indexOfLast=0;
        m_data[0]=data;
        m_realsize++;
    }

}

Lecsema ** NQueue::replace()
{
    m_size*=2;
    m_data=(Lecsema**)realloc(m_data,(sizeof(Lecsema*)*m_size));
    if(!m_data)
    {
        cout<<"Шеф, у нас проблемы с перевыделением памяти"<<endl;
        return NULL;
    }
    return m_data;
}

void NQueue::add(Lecsema *data)
{

    if(m_realsize+1>=m_size)
    {
        if(m_indexOfFirst==-1)//Служит для перевыделения памяти под очередь из которой забрали все элементы.
        {
            m_size=1;
            m_indexOfFirst++;
        }
        // cout<<"Размер увеличен с "<<m_size;
        m_data=replace();
        // cout<<" до "<<m_size<<endl;
        m_indexOfLast++;
        m_data[m_indexOfLast]=data;
        m_realsize++;
        return;
    }
    else
    {
        if(m_indexOfFirst==-1)
        {
            m_indexOfFirst++;
        }
        m_indexOfLast++;
        m_data[m_indexOfLast]=data;
        m_realsize++;
        return;
    }

}

Lecsema * NQueue::get()
{
    if(m_indexOfFirst==-1)
    {
        // cout<<"Ошибка, очередь пуста."<<endl;
        return NULL;
    }
    Lecsema * forData;
    if(m_realsize<=0)
    {
        // cout<<"Массив пуст"<<endl;
        return NULL;
    }
    forData=m_data[m_indexOfFirst];
    m_data[m_indexOfFirst]=NULL;
    m_indexOfFirst++;
    m_realsize--;
    if(m_realsize==0)
    {
        m_indexOfFirst=-1;
        m_indexOfLast=-1;
        delete [] m_data;
        m_data=NULL;
        m_size=0;
//        cout<<"Конец"<<endl;

    }
    return forData;
}

Lecsema * NQueue::showLast()
{
    if(m_indexOfLast!=-1)
    {
//        cout<<"Дерунлся "<<m_data[m_indexOfLast]<<endl;
        return m_data[m_indexOfLast];
    }
    else
        return NULL;
}

int NQueue::length()
{
    return m_realsize;
}
