/* File:      pita_lib.c
** Author(s): Fabrizio Riguzzi and Terrance Swift
** Contact:   fabrizio.riguzzi@unife.it, xsb-contact@cs.sunysb.edu
**
** Copyright (C) Copyright: Fabrizio Riguzzi and Terrance Swift
**                          ENDIF - University of Ferrara
**                          Centro de Inteligencia Artificial,
**                          Universidade Nova de Lisboa, Portugal
**
** XSB is free software; you can redistribute it and/or modify it under the
** terms of the GNU Library General Public License as published by the Free
** Software Foundation; either version 2 of the License, or (at your option)
** any later version.
**
** XSB is distributed in the hope that it will be useful, but WITHOUT ANY
** WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
** FOR A PARTICULAR PURPOSE.  See the GNU Library General Public License for
** more details.
**
** You should have received a copy of the GNU Library General Public License
** along with XSB; if not, write to the Free Software Foundation,
** Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
**
*/

/* 
This package uses the library cudd, see http://vlsi.colorado.edu/~fabio/CUDD/
for the relative license.
*/
#include "util.h"
#include "cudd.h"
#include <glib.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "cinterf.h"


unsigned long dividend;


typedef struct
  {
    int nVal,nBit;
    int firstBoolVar;
  } variable;


void init_my_predicates(void);
int compare(char *a, char *b);
gint my_equal(gconstpointer v,gconstpointer v2);
guint my_hash(gconstpointer key);
void  dealloc(gpointer key,gpointer value,gpointer user_data);
FILE *open_file (char *filename, const char *mode);

static variable * vars;
double * probs;
static DdManager * mgr;
static int nVars;
static int  boolVars;
GHashTable  * nodes; /* hash table that associates nodes with their probability
 if already computed, it is defined in glib */
int _debug = 0;

double Prob(DdNode *node);



void  initc(int reorder)
{
  int intBits;

  nVars=0;
  boolVars=0;
  mgr=Cudd_Init(nVars,0,CUDD_UNIQUE_SLOTS,CUDD_CACHE_SLOTS,0);

  if (reorder != CUDD_REORDER_NONE)
    Cudd_AutodynEnable(mgr,reorder);

  vars= (variable *) malloc(nVars * sizeof(variable));
  probs=(double *) malloc(0);
  intBits=sizeof(unsigned int)*8;
  dividend=-1;
  /* dividend is a global variable used by my_hash 
     it is equal to an unsigned int with binary representation 11..1 */ 
}

void reorderc(int method)
{

  Cudd_ReduceHeap(mgr,method,0);
}

void endc(void)
{
    Cudd_Quit(mgr);
  free(vars);
  free(probs);
}

double ret_probc(int node)
{
  DdNode * node_in;
  double out;
  
  node_in=(DdNode *) node;
  nodes=g_hash_table_new(my_hash,my_equal);
  out=Prob(node_in);
  g_hash_table_foreach (nodes,dealloc,NULL);
  g_hash_table_destroy(nodes);
  return out;
}

int add_varc(int nVal,prolog_term probabilities)
{
  variable * v;
  int i;
  double p,p0;

  nVars=nVars+1;
  vars=(variable *) realloc(vars,nVars * sizeof(variable));
  v=&vars[nVars-1];
  v->nVal=nVal;

  probs=(double *) realloc(probs,(((boolVars+v->nVal-1)* sizeof(double))));
  p0=1;
  for (i=0;i<v->nVal-1;i++)
  {
    p=p2c_float(p2p_car(probabilities));
    probs[boolVars+i]=p/p0;
    probabilities=p2p_cdr(probabilities);

    p0=p0*(1-p/p0);
  }
  v->firstBoolVar=boolVars;
  boolVars=boolVars+v->nVal-1;

  return  nVars-1;
}

int equalityc(int varIndex, int value)
{
  int i;
  variable v;
  DdNode * node,  * tmp,  *var ;

  v=vars[varIndex];
  i=v.firstBoolVar;
  node=NULL;
  tmp=Cudd_ReadOne(mgr);
  Cudd_Ref(tmp);
  for (i=v.firstBoolVar;i<v.firstBoolVar+value;i++)
  {
    var=Cudd_bddIthVar(mgr,i);
    node=Cudd_bddAnd(mgr,tmp,Cudd_Not(var));
    Cudd_Ref(node);
    Cudd_RecursiveDeref(mgr,tmp);
    tmp=node;
  }
  if (!(value==v.nVal-1))
  {
    var=Cudd_bddIthVar(mgr,v.firstBoolVar+value);
    node=Cudd_bddAnd(mgr,tmp,var);
    Cudd_Ref(node);
    Cudd_RecursiveDeref(mgr,tmp);
  }
  return (int) node;
}

int onec(void)
{
  DdNode * node;

  node =  Cudd_ReadOne(mgr);
  Cudd_Ref(node);
  return (int) node;
}

int zeroc(void)
{
  DdNode * node;

  node=Cudd_ReadLogicZero(mgr);
  Cudd_Ref(node);
  return (int) node;
}

int bdd_notc(int node)
{
  return (int) Cudd_Not((DdNode *)node);
}

int andc(int nodea, int nodeb)
{
  DdNode * node1, *node2,*nodeout;

  node1=(DdNode *)nodea;
  node2=(DdNode *)nodeb;
  nodeout=Cudd_bddAnd(mgr,node1,node2);
  Cudd_Ref(nodeout);
  return (int)nodeout;
}

int orc(int nodea, int nodeb)
{
  DdNode * node1,*node2,*nodeout;

  node1=(DdNode *)nodea;
  node2=(DdNode *)nodeb;
  nodeout=Cudd_bddOr(mgr,node1,node2);
  Cudd_Ref(nodeout);
  return (int)nodeout;
}

void create_dotc(int node,prolog_term filenameatom)
{
  char * onames[]={"Out"};
  char ** inames;
  DdNode * array[1];
  int i,b,index;
  variable v;
  char numberVar[10],numberBit[10];
  FILE * file;
  char * filename;

  filename=p2c_string(filenameatom);
  inames= (char **) malloc(sizeof(char *)*boolVars);
  index=0;
  for (i=0;i<nVars;i++)
  {
    v=vars[i];
    for (b=0;b<v.nVal-1;b++)
    {
      inames[b+index]=(char *) malloc(sizeof(char)*20);
      strcpy(inames[b+index],"X");
      sprintf(numberVar,"%d",i);
      strcat(inames[b+index],numberVar);
      strcat(inames[b+index],"_");
      sprintf(numberBit,"%d",b);
      strcat(inames[b+index],numberBit);
    }
    index=index+v.nVal-1;
  }
  array[0]=(DdNode *)node;
  file = open_file(filename, "w");
  
  Cudd_DumpDot(mgr,1,array,inames,onames,file);
  fclose(file);
  index=0;
  for (i=0;i<nVars;i++)
  {
    v=vars[i];
    for (b=0;b<v.nVal-1;b++)
      free(inames[b+index]);
    index=index+v.nVal-1;
  }
  free(inames);
}


double Prob(DdNode *node )
/* compute the probability of the expression rooted at node
nodes is used to store nodes for which the probability has alread been computed
so that it is not recomputed
 */
{
  int comp;
  int index;
  double res,resT,resF;
  double p;
  double * value_p;
  DdNode **key,*T,*F,*nodereg;
  double *rp;

  comp=Cudd_IsComplement(node);
  if (Cudd_IsConstant(node))
  {
    if (comp)
      return 0.0;
    else
      return 1.0;
  }
  else
  {
    nodereg=Cudd_Regular(node);  
    value_p=g_hash_table_lookup(nodes,&node);
    if (value_p!=NULL)
    {
      if (comp)
        return 1-*value_p;
      else
        return *value_p;
    }
    else
    {
      index=Cudd_NodeReadIndex(node);
      p=probs[index];
      T = Cudd_T(node);
      F = Cudd_E(node);
      resT=Prob(T);
      resF=Prob(F);
      res=p*resT+(1-p)*resF;
      key=(DdNode **)malloc(sizeof(DdNode *));
      *key=nodereg;
      rp=(double *)malloc(sizeof(double));
      *rp=res;
      g_hash_table_insert(nodes, key, rp);
      if (comp)
        return 1-res;
      else
        return res;
    }
  }
}


FILE * open_file(char *filename, const char *mode)
/* opens a file */
{
  FILE *fp;

  if ((fp = fopen(filename, mode)) == NULL) 
  {
      perror(filename);
      exit(1);
    }
    return fp;
}

gint my_equal(gconstpointer v,gconstpointer v2)
/* function used by GHashTable to compare two keys */
{
  DdNode *a,*b;
  a=*(DdNode **)v;
  b=*(DdNode **)v2;
  return (a==b);
}
guint my_hash(gconstpointer key)
/* function used by GHashTable to hash a key */
{
  unsigned int h;
  h=(unsigned int)((unsigned long) *((DdNode **)key) % dividend);
  return h;
}
void  dealloc(gpointer key,gpointer value,gpointer user_data)
{
  free(key);
  free(value);

}
