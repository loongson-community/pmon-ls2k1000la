
const char *conffile;		/* source file, e.g., "GENERIC.sparc" */
const char *machine;		/* machine type, e.g., "sparc" or "sun3" */
const char *machinearch;	/* machine arch, e.g., "sparc" or "m68k" */
const char *sysarch;		/* extra architecture specification */
const char *srcdir;		/* path to source directory (rel. to build) */
const char *builddir;		/* path to build directory */
const char *defbuilddir;	/* default build directory */
int	errors;			/* counts calls to error() */
struct	nvlist *options;	/* options */
struct	nvlist *defoptions;	/* "defopt"'d options */
struct	nvlist *mkoptions;	/* makeoptions */
struct	hashtab *devbasetab;	/* devbase lookup */
struct	hashtab *devatab;	/* devbase attachment lookup */
struct	hashtab *selecttab;	/* selects things that are "optional foo" */
struct	hashtab *needcnttab;	/* retains names marked "needs-count" */
struct	hashtab *opttab;	/* table of configured options */
struct	hashtab *defopttab;	/* options that have been "defopt"'d */
struct	devbase *allbases;	/* list of all devbase structures */
struct	deva *alldevas;		/* list of all devbase attachment structures */
struct	config *allcf;		/* list of configured kernels */
struct	devi *alldevi;		/* list of all instances */
struct	devi *allpseudo;	/* list of all pseudo-devices */
int	ndevi;			/* number of devi's (before packing) */
int	npseudo;		/* number of pseudo's */

struct	files *allfiles;	/* list of all kernel source files */
struct objects *allobjects;	/* list of all kernel object and library files */

struct	devi **packed;		/* arrayified table for packed devi's */
int	npacked;		/* size of packed table, <= ndevi */
struct  parents parents;
struct  locators locators;
