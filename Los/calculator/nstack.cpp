#include "nstack.h"
#include <iostream>
using namespace std;
NStack::NStack(int size, Lecsema * data)
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

Lecsema ** NStack::replace()
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

Lecsema * NStack::cherryPick()
{
    if(m_indexOfLast!=-1)
        return m_data[m_indexOfLast];
    else
        return NULL;
}

void NStack::add(Lecsema *data)
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

Lecsema * NStack::get()
{
    if(m_indexOfFirst==-1)
    {
        // cout<<"Ошибка, стек пуст."<<endl;
        return NULL;
    }
    Lecsema * forData;
    if(m_realsize<=0)
    {
        // cout<<"Массив пуст"<<endl;
        return NULL;
    }
    forData=m_data[m_indexOfLast];
    m_data[m_indexOfLast]=NULL;
    m_indexOfLast--;
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

int NStack::length()
{
    return m_realsize;
}
