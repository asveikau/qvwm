/*
 * hash.h
 *
 * Copyright (C) 1995-2001 Kenichi Kourai
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with qvwm; see the file COPYING.  If not, write to
 * the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef _HASH_H_
#define _HASH_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * HashItem class --
 *   Element of hash.
 */
template <class T>
class HashItem {
private:
  char* key;                     // hash key
  T* item;                       // hash element
  HashItem<T>* ptr;              // pointer to next element

public:
  HashItem(char* hashkey, T* hashitem, HashItem<T>* hptr)
    : key(hashkey), item(hashitem), ptr(hptr) {}

  char* GetKey() const { return key; }
  T* GetItem() const { return item; }
  HashItem<T>* GetPtr() const { return ptr; }
};

/*
 * Hash class
 */
template <class T>
class Hash {
private:
  HashItem<T>** table;            // hash table
  int size;                       // hash table size

public:
  Hash(int siz) : size(siz) {
    table = new HashItem<T>*[size];
    
    for (int i = 0; i < size; i++)
      table[i] = 0;
  }
  ~Hash() {
    HashItem<T> *item, *pItem;

    for (int i = 0; i < size; i++) {
      item = table[i];
      while (item != NULL) {
	pItem = item->GetPtr();
	delete item;
	item = pItem;
      }
    }
    delete [] table;
  }
  /*
   * SetHashItem --
   *   Set item in hash table with key.
   */
  void SetHashItem(char* key, T* item) {
    if (key == NULL)
      return;

    int n = strlen(key);
    int keyNum;

    keyNum = abs((key[0]-'A')+(key[n/2]-'A')*26+(key[n-1]-'A')*26*26) % size;

    table[keyNum] = new HashItem<T>(key, item, table[keyNum]);
  }
  /*
   * GetHashItem --
   *   Get item corresponding to key from hash table.
   */
  T* GetHashItem(const char* key) {
    if (key == NULL)
      return NULL;

    int n = strlen(key);
    int keyNum;
    HashItem<T>* hashItem;
    
    keyNum = abs((key[0]-'A')+(key[n/2]-'A')*26+(key[n-1]-'A')*26*26) % size;
    
    hashItem = table[keyNum];
    
    while (hashItem != NULL) {
      if (strcmp(hashItem->GetKey(), key) == 0)
	return hashItem->GetItem();
      hashItem = hashItem->GetPtr();
    }

    // no match
    return NULL;
  }
};

#endif // _HASH_H_
