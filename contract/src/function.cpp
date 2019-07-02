#include "eossiege.hpp"

// 1: win  -1: lost  0: tie
int EOSSiege::battleresult(soldier soldier1, soldier soldier2)
{
  if(soldier1 == none && soldier2 == none)
  {
    return 0;
  }
  else if(soldier1 == none && soldier2 != none)
  {
    return -1;
  }
  else if(soldier1 != none && soldier2 == none)
  {
    return 1;
  }
  else
  {
    if(soldier1 == infantry)
    {
      if(soldier2 == shieldman)
      {
        return 1;
      }
      else if(soldier2 == archer || soldier2 == cavalry)
      {
        return -1;
      }
      else
      {
        return 0;
      }
    }
    else if(soldier1 == spearman)
    {
      if(soldier2 == cavalry)
      {
        return 1;
      }
      else if(soldier2 == shieldman || soldier2 == archer)
      {
        return-1;
      }
      else
      {
        return 0;
      }
    }
    else if(soldier1 == shieldman)
    {
      if(soldier2 == spearman || soldier2 == archer)
      {
        return 1;
      }
      else if(soldier2 == infantry || soldier2 == cavalry)
      {
        return -1;
      }
      else
      {
        return 0;
      }
    }
    else if(soldier1 == archer)
    {
      if(soldier2 == infantry || soldier2 == spearman)
      {
        return 1;
      }
      else if(soldier2 == shieldman)
      {
        return -1;
      }
      else
      {
        return 0;
      }
    }
    else if(soldier1 == cavalry)
    {
      if(soldier2 == infantry || soldier2 == shieldman)
      {
        return 1;
      }
      else if(soldier2 == spearman)
      {
        return -1;
      }
      else
      {
        return 0;
      }
    }
    else
    {
      return 0;
    }
  }
}

void EOSSiege::split(const string& s, char c, vector<string>& v) 
{
    string::size_type i = 0;
    string::size_type j = s.find(c);
    
    while (j != string::npos) 
    {
        v.push_back(s.substr(i, j - i));
        i = ++j;
        j = s.find(c, j);
        
        if (j == string::npos) v.push_back(s.substr(i, s.length()));
      
    }
}




