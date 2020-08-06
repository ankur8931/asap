/******************************************************************************
 *                             model.h
 * This file contains constants, structures, etc useful for implementing the
 * finite state engine used to validate the xml.
 *
 ****************************************************************************/

#ifndef MODEL_H_INCLUDED
#define MODEL_H_INCLUDED

#define MAXOMITTED 32

#define CDATA_ELEMENT	((dtd_element *)1)

typedef struct _dtd_state
{ struct _state_transition *transitions;
  struct _state_expander *expander;
} dtd_state;

dtd_state      *new_dtd_state(void);
dtd_state *	make_dtd_transition(dtd_state *here, dtd_element *e);
int		same_state(dtd_state *final, dtd_state *here);
int 		find_omitted_path(dtd_state *state, dtd_element *e,
				  dtd_element **path);
dtd_state *	make_state_engine(dtd_element *e);
void		free_state_engine(dtd_state *state);
void		state_allows_for(dtd_state *state,
				 dtd_element **allow, int *n);

#endif /*MODEL_H_INCLUDED*/
