/*
 * list.h
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

#ifndef _LIST_H_
#define _LIST_H_

#include "message.h"

template <class T>
class List {
public:
  class Iterator;
  friend class Iterator;

protected:
  class Item {
  friend class List;
  friend class Iterator;

  private:
    T* obj;
    Item* prev;
    Item* next;
    
  public:
    Item(T* item) : obj(item), prev(NULL), next(NULL) {}
  };

public:
  class Iterator {
  protected:
    List<T>* list;
    Item* current;

  public:
    Iterator() : list(NULL), current(NULL) {}
    Iterator(List<T>* lst) : list(lst), current(NULL) {}
    T* GetHead() {
      current = list->head;
      if (current)
	return current->obj;
      else
	return NULL;
    }
    T* GetTail() {
      current = list->tail;
      if (current)
	return current->obj;
      else
	return NULL;
    }
    T* GetPrev() {
      ASSERT(current);
      current = current->prev;
      if (current)
	return current->obj;
      else
	return NULL;
    }
    T* GetNext() {
      ASSERT(current);
      current = current->next;
      if (current)
	return current->obj;
      else
	return NULL;
    }
    T* GetCurrent() {
      if (current)
	return current->obj;
      else
	return NULL;
    }
    void InsertBefore(T* obj) {
      ASSERT(current);
      if (current->prev == NULL)
	list->InsertHead(obj);
      else {
	Item* li = new Item(obj);
	li->next = current;
	li->prev = current->prev;
	current->prev->next = li;
	current->prev = li;
	list->size++;
      }
    }
    void InsertAfter(T* obj) {
      ASSERT(current);
      if (current->next == NULL)
	list->InsertTail(obj);
      else {
	Item* li = new Item(obj);
	li->prev = current;
	li->next = current->next;
	current->next->prev = li;
	current->next = li;
	list->size++;
      }
    }
    // remove current item and advance iterator next
    T* Remove() {
      ASSERT(current);
      Item* li = current;
      current = current->next;
      list->Remove(li);

      if (current)
	return current->obj;
      else
	return NULL;
    }
  };

protected:
  Item* head;
  Item* tail;
  int size;

private:
  void Remove(Item* li) {
    if (li->prev)
      li->prev->next = li->next;
    else
      head = li->next;

    if (li->next)
      li->next->prev = li->prev;
    else
      tail = li->prev;

    delete li;
    size--;
  }

public:
  List() : head(NULL), tail(NULL), size(0) {}

  T* GetHead() const {
    if (head)
      return head->obj;
    else
      return NULL;
  }
  T* GetTail() const {
    if (tail)
      return tail->obj;
    else
      return NULL;
  }
  int GetSize() const { return size; }

  void InsertHead(T* obj) {
    Item* li = new Item(obj);
    if (head) {
      head->prev = li;
      li->next = head;
    }
    else
      tail = li;
    head = li;
    size++;
  }
  void InsertTail(T* obj) {
    Item* li = new Item(obj);
    if (tail) {
      tail->next = li;
      li->prev = tail;
    }
    else
      head = li;
    tail = li;
    size++;
  }
  Bool Remove(T* obj) {
    Item* li = head;

    while (li) {
      if (li->obj == obj) {
	Remove(li);
	return True;
      }
      li = li->next;
    }
    return False;
  }
  T* RemoveHead() {
    ASSERT(head);
    T* obj = head->obj;
    Remove(head);
    return obj;
  }
  T* RemoveTail() {
    ASSERT(tail);
    T* obj = tail->obj;
    Remove(tail);
    return obj;
  }
};

#endif // _LIST_H_
