
Logic programming in its various guises: using the well-founded or
stable semantics, can provide a useful mechanism for representing
knowledge, particularly when a program requires default knowledge.
However, many aspects of knowledge can be advantageously represented
as @em{ ontologies}.  In our viewpoint ontologies describe various
naive sets or @em{ classes} that are ordered by a subclass ordering,
along with various members of the sets.  Each class @em{C} has a @em{
description}.  In the simplest case, a description can consist just of
@em{C}'s name and a specification of where @em{C} lies on the subclass
hierarchy.  In a more elaborate case, @em{C} may also be described by
@em{ existential relations} that are defined for every element of @em{C}.
Conversely, @em{C} may also be described by @em{ schema relations} that
describe the typing of any of @em{C}'s relations.  For instance, if in a
given ontology, @em{C} is the class of @tt{ people} it may be described
as a subclass of @tt{ animals} (along with other subclasses); an
existential relation @tt{ has_mother} can be defined for elements of
@em{C} and whose range is the class of @tt{ female_people}.
Intuitively, this relation indicates that elements of @em{C} must have a
mother relation to a female.  @em{C} may also be described by the schema
relation @tt{ has_brother} whose range is @tt{ male_people} again
defined for all elements of @em{C} but intuitively indicating that $if$
an element of @em{C} has a brother, that brother must be male
@footnote{Support for more complex class expressions over the
description logic @em{ALCQ} is under development.}.

Because existential and schema relations are defined for all elements
of a class, a simple semantics of inheritance is obtained: an
existential or schema relation defined for @em{C} is defined for all
subclasses and all elements of @em{C}.  Of course relations can be
defined directly on these subclasses, and analgously, @em{ attributes}
can be defined on objects of @em{C}.

Representing knowledge by means of ontologies has several advantages.
First, knowledge obtained from various web taxonomies or ontologies
can be mapped directly into XSB.  Second non-programmers often feel
more comfortable reviewing or updating knowledge defined through
ontologies than they do logic programs.  Knowledge is somewhat
object-oriented; and can be updated by adding or deleting facts that
have a simple and clear set-theoretic semantics.  Such knowledge
management can be aided by GUIs, such as Protoge @cite{protege} or a
special-purpose editor based on InterProlog @cite{interprolog}.

The CDF package has several functions.  First, it compiles ontology
information into a form efficiently accessable and updatable by XSB,
and implements inheritance for various relations.  Many aspects of
semantic consistency of ontology information are checked automatically
by CDF.  Ontology information itself can be speficied either as Prolog
facts using the @em{ external format} or as Prolog rules that perhaps
access a database using the @em{ external intensional format}.
Persistency of ontology information is handled in various ways, either
using a backing file system via the @em{ components} mechanism, or
using the updatable database inteface.

As of 3/03, CDF is under rapid development, and many of its functions
are changing.  However, it has been used in several large commercial
projects so that its core functionality is stable and robust.
Accordingly, in the manual, modules subject to change are marked as
being under development.
