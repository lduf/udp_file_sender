#ifndef stackType_h_
#define stackType_h_

/*! \brief Structure d'un élément d'une stack d'entier */
struct model_elem {
  int element ;
  int duplicate;
  struct model_elem* next;
};

typedef struct model_elem ELEMSTACK;

typedef ELEMSTACK *STACK; 
 
//! \def valuer vraie  
#define TRUE 1
//! \def valeur Fausse  
#define FALSE 0

#endif
