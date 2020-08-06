

#ifndef _CONC_COMPL_H_

#define _CONC_COMPL_H_

#ifdef CONC_COMPL

#define MAX_THREAD_DEPS		256

struct subgoal_frame ;

typedef struct thread_dep
{
	struct subgoal_frame *Subgoal;
	int last ;
}
ThreadDep ;

typedef
struct {
int		NumDeps ;
ThreadDep 	Deps[MAX_THREAD_DEPS];
} ThreadDepList ;

int EmptyThreadDepList( ThreadDepList *TDL ) ; 

void InitThreadDepList( ThreadDepList *TDL ) ; 

struct th_context ;

void UpdateDeps(struct th_context *th, int *busy, CPtr *leader) ;
int CheckForSCC( struct th_context * th ) ;
int MayHaveAnswers( struct th_context * th ) ;
void CompleteOtherThreads( struct th_context * th ) ;
void WakeOtherThreads( struct th_context * th ) ;
void WakeDependentThreads( struct th_context * th, struct subgoal_frame * subg ) ;
void CompleteTop( struct th_context * th, CPtr leader ) ;
CPtr sched_external( struct th_context *th, CPtr ExtCons ) ;
#endif

#endif
