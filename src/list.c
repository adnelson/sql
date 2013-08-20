#include "../include/list.h"


void printNode(ListNode_t *node);
void changeSize(List_t *list, size_t change);
void setFront(List_t *list, ListNode_t *node);
void setBack(List_t *list, ListNode_t *node);
void changeSize(List_t *l, size_t change);
void setFront(List_t *list, ListNode_t *node);
void setBack(List_t *list, ListNode_t *node);
void list_addNode(List_t *list, ListNode_t *node, ListNode_t *prev, ListNode_t *next);

void list_init(List_t *list, void (*del) (void *)) {
   memset(list, 0, sizeof(List_t));
   pthread_mutex_init(&list->lock, NULL);
   list->del = del;
}

void setFront(List_t *list, ListNode_t *node) {
   list->front = node;
   if (node) node->prev = NULL;
}

void setBack(List_t *list, ListNode_t *node) {
   list->back = node;
   if (node) node->next = NULL;
}

void *list_removeNode(List_t *list, ListNode_t *node) {
   pthread_mutex_lock (&list->lock);
   
   /* if there's a previous pointer on this node, then set it to the next */
   if (node->prev) node->prev->next = node->next;
   /* if there isn't, then this was the front of the list */
   else setFront(list, node->next);
   
   /* if there's a next pointer, then set it to the previous */
   if (node->next) node->next->prev = node->prev;
   /* if there isn't, then this was the back of the list */
   else setBack(list, node->prev);
   
   void *data = node->data;
   free(node);
   
   list->size--;
   
   if (list->size == 1) {
      if (list->front)
         list->back = list->front;
      else
         list->front = list->back;
   }
   
   pthread_mutex_unlock(&list->lock);
   
   return data;
}

void list_removeNodeDelete(List_t *list, ListNode_t *node) {
   void *data = list_removeNode(list, node);
   if (list->del) list->del(data);
}

void list_addBetween(List_t *list, void *data, ListNode_t *prev, ListNode_t *next) {
   ListNode_t *node = listNode_init(data, prev, next);
   if (node) list_addNode(list, node, prev, next);
}


bool list_addInOrder(List_t *l, void *data){
   ListNode_t *curr;

   /* If no compare function, return false */
   if (!l->compare) return false;
     curr = l->front;
     while (curr) {
   if (l->compare(data,curr->data) < 0){
         list_addBetween(l, data, curr->prev, curr);
      return true;
   }
   curr = curr->next;
     } /* finished w/o adding anything, biggest, add to end */
     return list_addBack(l,data);
}

void list_addNode(List_t *list, ListNode_t *node, ListNode_t *prev, ListNode_t *next) {
   pthread_mutex_lock (&list->lock);

   if (prev)
      prev->next = node;
   else
      setFront(list, node);

   if (next)
      next->prev = node;
   else
      setBack(list, node);

   list->size++;
   pthread_mutex_unlock(&list->lock);
}

void changeSize(List_t *list, size_t change) {
   pthread_mutex_lock (&list->lock);
   list->size += change;
   pthread_mutex_unlock(&list->lock);
}

void *list_removeFront(List_t *list) {
   if (list->size > 0)
      return list_removeNode(list, list->front);
   return NULL;
}

void list_destroy(List_t *list) {
   ListNode_t *p = list->front, *tmp;
   while (p) {
      if (list->del) { list->del(p->data); }
      tmp = p;
      p = p->next;
      free(tmp);
   }
}

ListNode_t *listNode_init(void *data, ListNode_t *prev, ListNode_t *next) {
   ListNode_t *node = (ListNode_t *)malloc(sizeof(ListNode_t));
   if (node) {
      node->data = data;
      node->next = next;
      node->prev = prev;
   }
   return node;
}

bool list_addBack(List_t *list, void *data) {
   ListNode_t *node = listNode_init(data, list->back, NULL);

   if (node) {
      list_addNode(list, node, list->back, NULL);
      return true;
   }
   return false;
}

bool list_addFront(List_t *list, void *data) {
   ListNode_t *node = listNode_init(data, NULL, list->front);
   if (node) {
      list_addNode(list, node, NULL, list->back);
      return true;
   }
   return false;
}

void *list_removeBack(List_t *list) {
   if (list->size > 0)
      return list_removeNode(list, list->back);
   return NULL;
}

void *list_findByInt(List_t *list, int (*toInt) (void *), int i) {
   ListNode_t *node = list->front;
   while (node) {
      if (i == toInt(node->data))
         return node->data;
      node = node->next;
   }
   return NULL;
}

bool list_removeByInt(List_t *list, int (*toInt) (void *), int i) {
   ListNode_t *node = list->front, *prev = NULL;
   while (node) {
      if (i == toInt(node->data)) {
         if (prev)
            prev->next = node->next;
         else
            list->front = node->next;
         if (!node->next)
            list->back = prev;
         if (list->del)
            list->del(node->data);
         list->size--;
         free(node);
         return true;
      }
      prev = node;
      node = node->next;
   }
   return false;
}

void *list_findByString(List_t *list, void (*toString) (char *,void *), const char *str) {
   char buf[1024];
   ListNode_t *node = list->front;
   if (!str) return NULL;
   while (node) {
      toString(buf, node->data);
      if (!strcmp(str, buf))
         return node->data;
      node = node->next;
   }
   return NULL;
}

void printNode(ListNode_t *node) {
   printf("Node %p, prev = %p, next = %p, data = %p\n", node, node->prev, node->next, node->data);
}

void list_printCustom(List_t *list, char * (*toString) (void *), bool freeAfter) {
   ListNode_t *node = list->front;
   printf("List_t of size %lu, front = %p, back = %p\n", list->size, list->front, list->back);
   while (node) {
      printNode(node);
      if (toString) {
         char *str = toString(node->data);
         printf("string = %s\n", str);
         if (freeAfter) free(str);
      }
      node = node->next;
   }
}

void list_print(List_t *list, bool verbose) {
   ListNode_t *node = list->front;
   if (verbose)
      printf("List_t of size %lu, front = %p, back = %p\n", 
               list->size, list->front, list->back);
   while (node) {
      if (verbose)
         printNode(node);
      if (list->toString) {
         char *str = list->toString(node->data);
         printf("%s\n", str);
      } else if (list->print) {
         list->print(node->data);
      } else {
         printf("(no print function defined)\n");
      }
      node = node->next;
   }
}

bool list_removeByString(List_t *list, void (*toString) (char *,void *), const char *str) {
   ListNode_t *node = list->front, *prev = NULL;
   char buf[50];
   while (node) {
      toString(buf, node->data);
      if (!strcmp(str, buf)) {
         if (prev)
            prev->next = node->next;
         else
            list->front = node->next;
         if (!node->next)
            list->back = prev;
         if (list->del)
            list->del(node->data);
         list->size--;
         free(node);
         return true;
      }
      prev = node;
      node = node->next;
   }
   return false;
}

void *list_findByPointer(List_t *list, void *data) {
   ListNode_t *cur = list->front;
   while (cur) {
      if (cur->data == data)
         return data;
      cur = cur->next;
   }
   return NULL;
}

void *list_removeByPointer(List_t *list, void *ptr) {
   ListNode_t *cur = list->front;
   while (cur) {
      if (cur->data == ptr)
         return list_removeNode(list, cur);
      cur = cur->next;
   }
   return NULL;
}


void list_removeByPointerFree(List_t *list, void *ptr) {
   void *data = list_removeByPointer(list, ptr);
   if (data && list->del)
      list->del(data);
}

void list_setPrintFunc(List_t *l, void (*print) (void *)) {
   l->print = print;
}